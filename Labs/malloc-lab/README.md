
# Malloc Lab實驗紀錄

## 相關檔案

### 主要檔案

**`mm.{c,h}`**
	主要邊寫程式的目標檔案

**`mdriver.c`**	

用來測試編寫程式正確性的test code

**`short{1,2}-bal.rep`**

測試用的trace files

**`Makefile`**

makefile文件

### 其他檔案

config.h	Configures the malloc lab driver
fsecs.{c,h}	Wrapper function for the different timer packages
clock.{c,h}	Routines for accessing the Pentium and Alpha cycle counters
fcyc.{c,h}	Timer functions based on cycle counters
ftimer.{c,h}	Timer functions based on interval timers and gettimeofday()
memlib.{c,h}	Models the heap and sbrk function

### 如何進行測試

To build the driver, type "make" to the shell.

To run the driver on a tiny test trace:

	unix> mdriver -V -f short1-bal.rep

The -V option prints out helpful tracing and summary information.

To get a list of the driver flags:

	unix> mdriver -h

## 實驗紀錄

- 編寫一個動態記憶體分配器

  - malloc, free, realloc, calloc

- `mm.c`文件是唯一需要編寫的程式

- 使用`make`指令進行編譯，然後使用`./mdriver`測試`mm.c`的正確性

- 主要編寫函式

  - 函式宣告在`mm.h`中

  - 額外新增void mm_checkheap(void)

  - 可以參考code/vm/malloc目錄下的mm.{c,h}

  - 書本有隱式列表的案例code

  - *運行環境為64bits機器，額外限制條件式堆空間不得大於等於4GB &larr;  優化對齊可會有bug但檢測不到

  - 可以先看mm.c的邏輯何mm-naive.c的實現或書本內容

  - 函式編寫規則

    - `mm_init`
      - 分配初始化heap空間
      - 錯誤返回-1，否則返回0
      - 不能調用mem_init
      - 每次使用trace測試時會調用該函式
    - `malloc`
      - 返回data block地址
      - 返回地址需要在堆空間範圍內，
      - 不得與其他塊重疊
      - 8 bytes對齊
    - `free`
      - 只有在傳入地址是之前被動態分配，且沒被釋放的塊
      - 傳入NULL沒有作用
    - `realloc`
      - 若傳入指標為NULL，則作用相當於malloc(size)
      - size若為0，則作用相當於free(ptr)
      - 傳入的地址必須是之前已經被動態分配過的地址
      - 新舊塊的地址可以相同 &rarr; 簡單的擴充舊塊長度
      - 若無法況充，則向後尋找新塊，並返回該塊的地址並將舊塊釋放掉
      - 新舊塊內容比較
        - 新大於舊 &rarr; 新舊塊前幾個bytes內容相同
        - 舊大於新 &rarr; 新快的所有內容與舊塊前幾個bytes相同
    - `calloc`
      - 動態分配一塊記憶體，並將其內容接初始化為0
      - 檢測code不會驗證calloc的吞吐率，只需要簡單編寫一個正確的應用就可以
    - `mm_checkheap`
      - debug用的檢測函式
      - 檢測heap的正確性

  - 模擬的記憶體系統

    - memlib.c模擬了一個虛擬的記憶體管理庫，裡頭包含
      - mem_sbrk
      - mem_heap_lo
      - mem_heap_hi
      - mem_heapsize
      - mem_pagesize

  - 編寫規則

    - 不要更改`mm.h`中的結構，實驗建議在`mm.c`中編寫static functions
    - 不要調用libc中任何記憶體有關的函式或系統調用
    - 不得在mm.c中使用任何數據結構(陣列、列表、樹、結構體等)，只能宣告全局的標量參數(int, float, char , pointer, etc)
    - 除了參考書或K &R之外不能複製任何代碼，但是可以參考並修改
    - 編寫時推薦查看trace files
    - 動態記憶體分配器返回的地址必須要是8 bytes對齊
    - 目標機器為64 bits
    - 編譯不得有warning產生

  - 如何進行測試

    - `mdriver.c`
      - 用於驗證`mm.c`的正確性、記憶體使用率、吞吐量

    
