# Cache Lab實驗紀錄

## 實驗簡介

*This is the handout directory for the CS:APP Cache Lab.* 

************************
Running the autograders:
************************

Before running the autograders, compile your code:
    `linux> make`

Check the correctness of your simulator:
    `linux> ./test-csim`

Check the correctness and performance of your transpose functions:
 `   linux> ./test-trans -M 32 -N 32`
    `linux> ./test-trans -M 64 -N 64`
   ` linux> ./test-trans -M 61 -N 67`

Check everything at once (this is the program that your instructor runs):
   ` linux> ./driver.py    `

******
Files:
******

### 欲修改檔案
`csim.c`       Your cache simulator
`trans.c`      Your transpose function

### 測試工具
`Makefile`     Builds the simulator and tools
`README.md`       This file
`driver.py*`   The driver program, runs test-csim and test-trans
`cachelab.c`   Required helper functions
`cachelab.h`   Required header file
`csim-ref*`    The executable reference cache simulator
`test-csim*`   Tests your cache simulator
`test-trans.c` Tests your transpose function
`tracegen.c`   Helper program used by test-trans
`traces/`      Trace files used by test-csim.c

### 環境配置

我們需要讀取trace文件中的命令來評估緩存的性能，而這些trace文件是由`Valgrind`這告軟體生成的，因此建議先進行安裝

> 輸入 `sudo apt install valgrind`

完成安裝後可以透過指令`valgrind --log-fd=1 --tool=lackey -v --trace-mem=yes ls -l`，追蹤目錄下所有可執行文件的緩存使用狀況

輸入完指令後可以在終端中看到以下圖的格式輸出，它代表程式執行過程中緩存的運作紀錄

- I &rarr; 讀取指令(*實驗不需要實現該指令*)
- M &rarr; 修改數據並存回緩存
- L &rarr; 讀取數據
- S &rarr; 儲存數據

> 格式： [操作指令], [64 bits地址], [數據大小]

![image-20221029145935169](https://raw.githubusercontent.com/WeiLin66/pictures/main/image-20221029145935169.png)

## Part. A

該實驗值主要編寫一個LRU策略的虛擬緩存，透過讀取trace文件指令模擬緩存的操作，並對`hits`, `miss`, `eviction`數量進行分析

### 實驗須知

1. 我們需要在`csim.c`中實現虛擬緩存
2. 由於測試時會使用不同的`sets`, `line`, `block`緩存使用的記憶體空間要使用`malloc`動態分配
3. trace資料夾包括要所有要讀取的`.trace`文件
4. 可以調用`cachelab.c/.h`中的helper function，例如`printfSummary()`



### 如何進行驗證

Lab文件說得很清楚:

![image-20221029201125306](https://raw.githubusercontent.com/WeiLin66/pictures/main/image-20221029201125306.png)

![image-20221029201214547](https://raw.githubusercontent.com/WeiLin66/pictures/main/image-20221029201214547.png)

> 測試自行編寫的緩存，執行結果必須要跟`csim-ref`相同

或是直接執行`./test-csim`查看輸出結果與reference simulator是否相同，若是輸出結果為27分那就代表通過測試用例

![image-20221029203833556](https://raw.githubusercontent.com/WeiLin66/pictures/main/image-20221029203833556.png)

### 程式撰寫

我們把整個程式粗略分成5個區塊:

![image-20221029161404919](https://raw.githubusercontent.com/WeiLin66/pictures/main/image-20221029161404919.png)

附上Github上完整的code: [csim.c](https://github.com/WeiLin66/CMU-15-213/blob/main/Labs/cache-lab/csim.c)

#### 讀取輸入參數

該模塊的目的是為了讀取可執行檔的後續參數，例如`./csim -s 4 -E 1 -b 4 -t traces/yi.trace`中的組數、行、偏移量、trace文件，先把這些數據暫存起來，待初始化緩存與讀取trace文件時使用，其程式碼區塊如下

```c
/**
 * func for getting operator
 */ 
static char get_operation(char* str){

    if(strlen(str) <= 1){
        
        return '\0';
    }

    return str[1];
}

int main(int argc, char* argv[]){
    
    // ... 省略
    
	for(int i=1; i<argc; i++){

        char op = get_operation(argv[i]);

        switch(op){

            case 'h':
                print_help_message();
            break;

            case 'v':
                print_msg = true;
            break;

            case 's':
                s = atoi(argv[++i]);
                input_check++;
            break;

            case 'E':
                E = atoi(argv[++i]);
                input_check++;
            break;

            case 'b':
                b = atoi(argv[++i]);
                input_check++;
            break;

            case 't':
                strcpy(filename, argv[++i]);
                input_check++;
            break;

            default:
            break;
        }
    }
    
    // ... 省略
}


```



#### 初始化cache

利用得到的參數，為cache分配動態記憶體，其function為`cache_init(s, E)`，緩存中的行定義為結構體`cacheLine`

```c
typedef struct{

    uint32_t vaild;
    uint32_t tag;
    uint64_t timeStamp; // 供LRU使用的時間戳
}cacheLine;

/**
 * func for initializing cache memory
 */ 
static void cache_init(uint8_t s, uint8_t E){

    uint32_t sets = 1 << s;

    virtual_cache = (cacheLine**)malloc(sizeof(cacheLine*) * sets);

    for(int i=0; i<sets; i++){

        virtual_cache[i] = (cacheLine*)malloc(sizeof(cacheLine) * E);
    }
}
```

#### 操作指令解讀

該部份用來解讀trace文件中的操作指令、地址、大小信息

函式`cmd_parsing(filename)`會逐條取trace文件的行直到EOF

函式`load_operation(line)`會解析該條指令並操作緩存

```C
/**
 * func for loading data from cache
 */ 
static void load_operation(char* line){

    uint64_t addr=0;
    uint32_t dataBytes=0;
    char op;

    /* 取trace文件的一行命令 */
    sscanf(line, " %c %lx,%u", &op, &addr, &dataBytes);

    /* 操作指令只能有L, M, S */
    if(op != 'L' && op != 'M' && op != 'S'){

        return;
    }

    if(print_msg){

        printf("%c %lx,%u ", op, addr, dataBytes);
    }    

    /* 對地址進行解析 */
    uint32_t set = (addr >> b) & (~(U64MAX << s));
    uint32_t tag = addr >> (s+b);

    bool find = false;
    int empty_line = -1;

    /* 遍歷cache查看是否存在該數據 */
    for(int i=E-1; i>=0; i--){

        /* 緩存命中，同時更新時間 */
        if(virtual_cache[set][i].vaild && virtual_cache[set][i].tag == tag){

            find = true;
            virtual_cache[set][i].timeStamp = ticks;
            break;
        }else if(virtual_cache[set][i].vaild == 0){

            /* 找到空的行 */
            empty_line = i;
        }
    }

    /* 若為修改指令，因為會寫回所以hit數一定要加一 */
    hits = op == 'M' ? hits+1 : hits; 

    if(find){

        hits++;

        if(print_msg){

            printf("hit ");
        }        
    }else{

        miss++;

        if(print_msg){

            printf("miss ");
        }
          
        /* 若不命中且沒有空行，就會發生eviction */
        if(empty_line == -1){

            evictions++;
            /* 使用LRU找需要替換的行 */
            empty_line = LRU(set);
            
            if(print_msg){

                printf("eviction ");
            }             
        }
		
        /* 不命中後重新加載，更新緩存 */
        virtual_cache[set][empty_line].vaild = 1;
        virtual_cache[set][empty_line].tag = tag;
        virtual_cache[set][empty_line].timeStamp = ticks;
    }

    if(op == 'M' && print_msg){

        printf("hit ");
    } 

    if(print_msg){

        printf("\n");
    }  

}

/**
 * func for parsing commands from trace file
 */ 
static void cmd_parsing(char* filename){

    char line[MAX_LEN];
    FILE* fp = fopen(filename, "r");

    while(!feof(fp) && !ferror(fp)){

        strcpy(line, "\n");
        fgets(line, MAX_LEN, fp);
        load_operation(line);

        ticks++;        
    }

    fclose(fp);    
}
```

##### LRU

使用cacheLine的變數`timeStamp`判斷哪個行距離現在使用最久遠，因為每次操作時都會更新時間週期`ticks`(加一)，這代表`timeStamp`越小，距離現在越久

所以我們只需要掃描一遍某組中的所有行就可找出要替換的行

```C
/**
 * func for applying LRU algo.
 */ 
static uint8_t LRU(uint8_t set){

    int repalce = 0;

    for(int i=1; i<E; i++){

        repalce = virtual_cache[set][i].timeStamp < virtual_cache[set][repalce].timeStamp ? i : repalce;
    }

    return repalce;
}
```

#### 打印輸出結果

可以調用`cachelab.c/.h`提供的`printSummary(hits, miss, evictions)`函式，簡單打印緩存執行後的結果

#### 釋放cache記憶體

所有trace文件的操作指令執行完後，釋放當初為緩存動態分配的記憶體

```c
/**
 * func for releasing cache memory
 */ 
static void free_cache(){

    uint32_t sets = 1 << s;

    for(int i=0; i<sets; i++){

        free(virtual_cache[i]);
        virtual_cache[i] = NULL;
    }

    free(virtual_cache);
    virtual_cache = NULL;
}
```

## Part. B

實驗B要求實現一個矩陣的轉移函式，並盡可能的降低緩存不命中的次數

實驗總共要編寫三種不同大小的轉移函式

1. 32 * 32
2. 64 * 64
3. 61 * 67

Lab文件有提到miss的範圍與得分佔比，例如64 * 64的矩陣測試結果miss要小於1300才能得到滿分

![image-20221029220634538](https://raw.githubusercontent.com/WeiLin66/pictures/main/image-20221029220634538.png)

### 實驗須知

1. 我們需要在`trans.c`中編寫並測試轉移函式
2. 轉移函式中**最多只能使用12個**`int`類型的局部變數，若轉移函式中調用其他函式，也要將該函式中的局部變數考慮進去
3. 不能使用位運算在一個整型變數存放多個值
4. 不允許使用``long`類型局部變數
5. `transpose_submit(int M, int N, int A[N][M], int B[M][N])`中陣列參數只允許修改B
6. 轉移函式中不允許宣告陣列變數或使用`malloc`

### 如何進行驗證

- `./test-trans -M 32 -N 32` for 32 * 32轉移矩陣
- `./test-trans -M 64 -N 64` for 64 * 64轉移矩陣
- `./test-trans -M 61 -N 67` for 61 * 67轉移矩陣

#### 註冊測試函式

註冊多個不同的函式，可以在進行驗證時同時執行，最多可以註冊100個不同的轉移函式

只需要在`registerFunctions()`中調用`registerTransFunction()`並填入該函式的註解字串以及函式名就可以了

> 註冊函式細節請見`cachelab.c/.h`

```c
/* 
 * trans - A simple baseline transpose function, not optimized for the cache.
 */
char trans_desc[] = "Simple row-wise scan transpose";
void trans(int M, int N, int A[N][M], int B[M][N]){
    int i, j, tmp;

    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            tmp = A[i][j];
            B[j][i] = tmp;
        }
    }    
}

void registerFunctions(){
    
    registerTransFunction(transpose_submit, transpose_submit_desc); 

    registerTransFunction(trans, trans_desc); 

    registerTransFunction(trans32, trans_desc32);

    registerTransFunction(trans64, trans_desc64);

    registerTransFunction(trans6167, trans_desc6167);
}
```



### Blocking

Blocking是一種記憶體區塊化處理的技術，透過提高循環內的局部性降低不命中率

為了演示Blocking為何有效，我們編寫一個簡單的小函式，計算矩陣A的每列與矩陣B的每行相乘的加總

#### Non-Blocking

- 先遍歷矩陣A的一列(row)，然後複製到矩陣B的一行上(column)
- 假設N, M均為8
- 假設緩存有4組，每組一行，一行存放8個整數

```c
int multi(int A[8][8], int B[8][8]){
    int i, j, temp=0, sum=0;

    for (i = 0; i < 8; i++) {
        for (j = 0; j < 8; j++) {
            temp = A[i][j] * B[j][i];
            sum += temp;
        }
    }
    
    return sum;
}
```

![non-blocking](https://raw.githubusercontent.com/WeiLin66/pictures/main/202210301622085.png)

##### 矩陣miss數

由於緩存的一行剛好可以容納矩陣A的一列，所以總共會發生8次miss，由於緩存只有4組，所以當遍歷到第4列時就會覆蓋掉第0列中緩存內容(發生`eviction`)，但由於是逐列掃描，所以差別不大

B矩陣是逐行(column)讀取的，所以每次都會發生miss

- b0, b8, b16, b24均發生miss，並分別將其加載進block0~block3
- b32, b40, b48, b56發生miss + eviction，**將加載的數據覆蓋block0~block3**
- 在每次循環中緩存內要不沒有數據，不然就是存放其他列數據，產生miss和eviction
- 因此每行均會發生8次miss，總長8行，miss數來到64次

> 總miss數為8 + 64 = 72次miss

#### Blocking

- 將8 * 8矩陣分成4個4*4小塊
- 每次處理其中一個塊
- 將A矩陣塊中的列與B矩陣塊中的行相乘
- 假設A矩陣塊處理順率為**先左到右，再上到下**
- 假設B矩陣塊處理順率為**先上到下，再左到右**

```C
int multi(int A[8][8], int B[8][8]){
    int i, j, temp=0, sum=0;
    
	/* 處理4個塊 */
    for (i = 0; i < 8; i+=4) {
        for (j = 0; j < 8; j+=4) {
            
            /* 處理每一個小塊 */
            for(int i1=i; i1<i+4; i1++){
                for(int j1=j; j1<j+4; j1++){
					temp = A[i1][j1] * B[j1][i1];
                    sum += temp;
                }
            }
        }
    }
    
    return sum;
}
```

![blocking](https://raw.githubusercontent.com/WeiLin66/pictures/main/202210301716397.png)

##### 矩陣miss數

由於緩存的一行剛好可以容納矩陣A的一列，所以前4列的數據會全部加載進緩存中；同理左下的塊加載時，右下的塊需要的數據也已經在緩存中了。Blocking方法對矩陣A來說沒變，依然是8次miss

對於B矩陣來說，每個塊產生的miss:

- 讀取矩陣B的b0, b8, b16, b24，並將前4列數據加載進緩存中
- 下一次循環對b1, b9, b17, b25操作均不會發生miss，因為數據已經在緩存了
- 因此每個塊會發生4次miss，總共4塊，所以miss數為4*4 = 16

> 總miss數為8 + 16 = 24次miss

### 為何Blocking有效

從上述的小實驗中可以看出，Blocking方法盡量避免緩存沒有被充分利用(不管是緩存為空，或是加載後只讀取一點數據)，讓數據的讀取有更好的空間局部性

使用Blocking方法需要特別注意緩存的大小以及讀取數據的規模，因為這會涉及到緩存如何看待數據的加載目標

### 程式撰寫

開始前先看看實驗前提:

- 函式執行完後B矩陣必須是A矩陣的轉移矩陣
- 使用Blocking方法降低miss
- 假設緩存大小為(s=5, E=1, b=5)，32組、1行、每行32bytes
- 緩存大小為1024bytes，可以容納256個`int`類型整數

#### 32*32

- 由於一列有32個整數，所以8列可以剛好填滿緩存

- 使用大小為8*8的塊(8代表8個整數大小)

```C
/* 
 * trans32*32
 */
char trans_desc32[] = "Simple transpose func32";
void trans32(int M, int N, int A[N][M], int B[M][N]){ 

    int i, j, i1;

    for (i = 0; i < M; i += 8) {

        for (j = 0; j < N; j += 8) {
            
            /* B * B mini block */
            for(i1=i; i1<i+8; i1++) {

                int temp_0 = A[i1][j];
                int temp_1 = A[i1][j+1];
                int temp_2 = A[i1][j+2];
                int temp_3 = A[i1][j+3];
                int temp_4 = A[i1][j+4];
                int temp_5 = A[i1][j+5];
                int temp_6 = A[i1][j+6];
                int temp_7 = A[i1][j+7];

                B[j][i1] = temp_0;
                B[j+1][i1] = temp_1;
                B[j+2][i1] = temp_2;
                B[j+3][i1] = temp_3;
                B[j+4][i1] = temp_4;
                B[j+5][i1] = temp_5;
                B[j+6][i1] = temp_6;
                B[j+7][i1] = temp_7;
            }
        }
    }
}
```



#### 64*64

```C
/* 
 * trans64*64
 */
char trans_desc64[] = "Simple transpose func64";
void trans64(int M, int N, int A[N][M], int B[M][N]){ 

    int i, j, k;
    int temp_0, temp_1, temp_2, temp_3, temp_4, temp_5, temp_6, temp_7;

    for (i = 0; i < N; i += 8) {

        for (j = 0; j < M; j += 8) {


            /* A1, A2 --> B1, B2 */
            for (k = 0; k < 4; k++) {

                temp_0 = A[i + k][j];
                temp_1 = A[i + k][j + 1];
                temp_2 = A[i + k][j + 2];
                temp_3 = A[i + k][j + 3];
                temp_4 = A[i + k][j + 4];
                temp_5 = A[i + k][j + 5];
                temp_6 = A[i + k][j + 6];
                temp_7 = A[i + k][j + 7];

                B[j][i + k] = temp_0;
                B[j + 1][i + k] = temp_1;
                B[j + 2][i + k] = temp_2;
                B[j + 3][i + k] = temp_3;

                B[j][i + k + 4] = temp_4;
                B[j + 1][i + k + 4] = temp_5;
                B[j + 2][i + k + 4] = temp_6;
                B[j + 3][i + k + 4] = temp_7;
            }

            /* B2 --> B3, A3 --> B2 */
            for (k = 0; k < 4; k++) {

                temp_0 = A[i + 4][j + k];
                temp_1 = A[i + 5][j + k];
                temp_2 = A[i + 6][j + k];
                temp_3 = A[i + 7][j + k];

                temp_4 = B[j + k][i + 4];
                temp_5 = B[j + k][i + 5];
                temp_6 = B[j + k][i + 6];
                temp_7 = B[j + k][i + 7];

                B[j + k][i + 4] = temp_0;
                B[j + k][i + 5] = temp_1;
                B[j + k][i + 6] = temp_2;
                B[j + k][i + 7] = temp_3;

                B[j + k + 4][i] = temp_4;
                B[j + k + 4][i + 1] = temp_5;
                B[j + k + 4][i + 2] = temp_6;
                B[j + k + 4][i + 3] = temp_7;

            }

            /* A4 --> B4 */
            for (k = 4; k < 8; k++) {

                temp_0 = A[i + k][j + 4];
                temp_1 = A[i + k][j + 5];
                temp_2 = A[i + k][j + 6];
                temp_3 = A[i + k][j + 7];

                B[j + 4][i + k] = temp_0;
                B[j + 5][i + k] = temp_1;
                B[j + 6][i + k] = temp_2;
                B[j + 7][i + k] = temp_3;
            }
        }
    }
}

```



#### 61*67

```C
/* 
 * trans61*67
 */
char trans_desc6167[] = "Simple transpose func6167";
void trans6167(int M, int N, int A[N][M], int B[M][N]){ 

    int i, j;
    int tmp1, tmp2, tmp3, tmp4, tmp5, tmp6, tmp7, tmp8;

    /* 56 * 67 */
    for (i = 0; i < (M/8)*8; i += 8) {

        for (j = 0; j < N; i++) {

            tmp1 = A[j][i];
            tmp2 = A[j][i + 1];
            tmp3 = A[j][i + 2];
            tmp4 = A[j][i + 3];
            tmp5 = A[j][i + 4];
            tmp6 = A[j][i + 5];
            tmp7 = A[j][i + 6];
            tmp8 = A[j][i + 7];

            B[i][j] = tmp1;
            B[i + 1][j] = tmp2;
            B[i + 2][j] = tmp3;
            B[i + 3][j] = tmp4;
            B[i + 4][j] = tmp5;
            B[i + 5][j] = tmp6;
            B[i + 6][j] = tmp7;
            B[i + 7][j] = tmp8;
        }
    }

    /* transpose rest elements */
    for (i = 0; i < N; i++) {

        for (j = (M/8)*8; j < M; j++) {

            tmp1 = A[i][j];
            B[j][i] = tmp1;
        }
    }    
}
```



