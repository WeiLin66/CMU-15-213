# bomb lab實驗紀錄

---

作者 : 林浩為

版本 : v1.0

## GDB常用指令

- `gcc -g test.c`生成可除錯文件

  - `gdb a.out`進行除錯

- r

  - 程式開始執行

- q

  - 離開gdb

- list 行數

  - 顯示以該行數為中心的前後總共10行程式碼
  - list 函數名
  - list 10,20
    - 打印10~20行
  - list +
    - 打印前進10行
  - list -
    - 打印後退10行

- p 變數名

  - 打印變數內容

  - 可以直接p 陣列或結構體變數名

  - p 函數名::某變數

    - 打印該函數下某變數值

    - `p main::i`

    - `p test::res`

    - `p struct/union`

    - `p struct->elements`

    - ```
      p foo(*newdata)
      ```

      - 可以直接打印函數調用結果

    - ```
      p test.c::res
      ```

      - 透過檔名打印全域變數

    - ```
      p /x i
      ```

      - 已16進制打印i

  - `p i + 3`

- `x addr`

    - 記憶體檢查
    - `x/nfu addr`
        - n, f, u為可選參數
        - n表示欲顯示記憶體數量，顯示的記憶體單元長度由u決定，若n為負數，則從addr向後計數
        - f為顯示格式
            - o - octal
            - x - hexadecimal
            - d - decimal
            - u - unsigned decimal
            - t - binary
            - f - floating point
            - a - address
            - c - char
            - s - string
            - i - instruction
        - u為記憶體單元長度
            - b - Bytes
            - h - Halfwords (2 Bytes)
            - w - Words (4 Bytes)
            - g - Giant words (8 Bytes)

- set var 變數＝特定值

  - 可以在運行中設置變數值(利用斷點)

  - 支持多種語法

    - `set var i=10`
    - `set var i = i * 10`
    - `set var i = $3 + i`
    - `set var array = {100,200,300,400,500}`
    - set
    - `set var struct->element = 10`
    - 同樣規則也是用全域變數

    - ```
      set var main::i=0
      ```

      - 指定檔案中的變數

- info

  - 用來顯示各種信息
  - info source
    - 顯示檔案資訊
  - info b
    - 檢查斷點
  - info args
    - 查看調用函數參數
  - info r
    - 查看暫存器狀態

- b

  - 設定斷點
  - b 函數名
    - `b main`
    - `b test`
  - b 行數
    - `b 10`
  - 刪除節點
    - clear 20
    - clear func
    - d Num

- watch

  - 建立監視點，一旦該變數發生變化，gdb就會停下來並顯示數值

- c

  - 繼續執行到下一個斷點

- s

  - 單步調試
  - s 步數
    - s 10

- n

  - 多步調試

- finish

  - 跳出函數

- bt

  - back trace
  - 查看stack調用消息

- frame

  - 當前所處函數

- help

  - 查看幫助手冊
  - 可以指定查找指令
    - help break

- layout

  - layout src
    - source file界面
  - layout asm
    - 組合語言界面
    - 組合語言界面單步為`si`
  - ctrl + x再按a可開關界面

- disas 函數名

    - 將該函數轉換為組合語言

- shell clear

  - 清除畫面

  

## 實驗過程

### 前言

該實驗需要我們找出每個炸彈的解碼密碼，由於其他source, header files缺失，我們僅能透過可執行檔`bomb`進行GDB調試，並在組合語言層面找出相對應的字串密碼。因此在開始前請先安裝GDB以及熟悉基本的GDB指令。



### Phase 1

```assembly
   0x0000000000400ee0 <+0>:	sub    $0x8,%rsp
   0x0000000000400ee4 <+4>:	mov    $0x402400,%esi
   0x0000000000400ee9 <+9>:	callq  0x401338 <strings_not_equal>
   0x0000000000400eee <+14>:	test   %eax,%eax
   0x0000000000400ef0 <+16>:	je     0x400ef7 <phase_1+23>
   0x0000000000400ef2 <+18>:	callq  0x40143a <explode_bomb>
   0x0000000000400ef7 <+23>:	add    $0x8,%rsp
   0x0000000000400efb <+27>:	retq 
```



主要是`strings_not_equal`這個函式在判斷輸入字串與密碼字串是否相等，是則返回`true`，否則返回`false`

`strings_not_equal`函式主要包含

1. `string_length`函式，用來逐一檢查字元，判斷字串長度
2. `strings_not_equal`後續就是逐一對字元進行比對，若不同則返回`false`，直到`\0`為止
3. 可以發現`strings_not_equal`具有兩個參數，一個是輸入參數，一個是解答字串，分別為`%rdi %rsi`，這個解碼字串儲存位置位於`0x402400`
4. 因此在GDB中直接打印該地址就可以查看該字串`x/s 0x402400`，輸出結果為`Border relations with Canada have never been better.`



### Phase 2

```assembly
   0x0000000000400efc <+0>:	push   %rbp
   0x0000000000400efd <+1>:	push   %rbx
   0x0000000000400efe <+2>:	sub    $0x28,%rsp
   0x0000000000400f02 <+6>:	mov    %rsp,%rsi
   0x0000000000400f05 <+9>:	callq  0x40145c <read_six_numbers>
   0x0000000000400f0a <+14>:	cmpl   $0x1,(%rsp)
   0x0000000000400f0e <+18>:	je     0x400f30 <phase_2+52>
   0x0000000000400f10 <+20>:	callq  0x40143a <explode_bomb>
   0x0000000000400f15 <+25>:	jmp    0x400f30 <phase_2+52>
   0x0000000000400f17 <+27>:	mov    -0x4(%rbx),%eax
   0x0000000000400f1a <+30>:	add    %eax,%eax
   0x0000000000400f1c <+32>:	cmp    %eax,(%rbx)
   0x0000000000400f1e <+34>:	je     0x400f25 <phase_2+41>
   0x0000000000400f20 <+36>:	callq  0x40143a <explode_bomb>
   0x0000000000400f25 <+41>:	add    $0x4,%rbx
   0x0000000000400f29 <+45>:	cmp    %rbp,%rbx
   0x0000000000400f2c <+48>:	jne    0x400f17 <phase_2+27>
   0x0000000000400f2e <+50>:	jmp    0x400f3c <phase_2+64>
   0x0000000000400f30 <+52>:	lea    0x4(%rsp),%rbx
   0x0000000000400f35 <+57>:	lea    0x18(%rsp),%rbp
   0x0000000000400f3a <+62>:	jmp    0x400f17 <phase_2+27>
   0x0000000000400f3c <+64>:	add    $0x28,%rsp
   0x0000000000400f40 <+68>:	pop    %rbx
   0x0000000000400f41 <+69>:	pop    %rbp
   0x0000000000400f42 <+70>:	retq 
```

phase 2主要觀察％rsp的變化(stack指標)，並觀察存放在中的參數變化

1. 首先判斷輸入參數個數，從程式碼執中可以判斷`phase_2`函數調用了名為`read_six_number`的函數，因此可以判斷出參數數量為6。實際進入函數體可以發現該函數又調用了`sscanf()`，並且返回值小於等於5就會引爆炸彈，由此可知我們的假設正確
1. 緊接著我們查看位於棧頂的局部變數(由於輸入參數不大於6因此棧頂沒有儲存多餘參數)。從`cmpl   $0x1,(%rsp)`這條語句推測出輸入值有可能為1開頭或1結尾(可能棧頂存放順序不同)，這個問題可以直接打印記憶體存放數據來解決。假如我輸入的數據為`1 2 3 4 5 6` 經過指令`x/6uw 0x7fffffffdf80`輸出後結果為`1 2 3 4 5 6`，這就可以判斷出棧頂存放的是輸入的元素1，即順序是隨棧地址增長
1. 從後續代碼其實就是一個類似`for loop`的操作，每次判斷當前元素是否為前一個元素的兩倍，直到指標指向`%rsp+0x18`為止(相當於遍歷完6個整數後的下一個地址)，將其轉換成C大致就是：

```c
/* 假設6個變數存放在arr中 */
if(arr[0] != 1){
    // bomb!
}

for(int i=1; i<6; i++){
    if(arr[i] != arr[i-1]*2){
        // bomb!
    }
}

// 解碼成功！
```

4. 總結以上邏輯，輸入參數其實就是`1 2 4 8 16 32`



### Phase 3

```assembly
   0x0000000000400f43 <+0>:	sub    $0x18,%rsp
   0x0000000000400f47 <+4>:	lea    0xc(%rsp),%rcx
   0x0000000000400f4c <+9>:	lea    0x8(%rsp),%rdx
   0x0000000000400f51 <+14>:	mov    $0x4025cf,%esi
   0x0000000000400f56 <+19>:	mov    $0x0,%eax
   0x0000000000400f5b <+24>:	callq  0x400bf0 <__isoc99_sscanf@plt>
   0x0000000000400f60 <+29>:	cmp    $0x1,%eax
   0x0000000000400f63 <+32>:	jg     0x400f6a <phase_3+39>
   0x0000000000400f65 <+34>:	callq  0x40143a <explode_bomb>
   0x0000000000400f6a <+39>:	cmpl   $0x7,0x8(%rsp)
   0x0000000000400f6f <+44>:	ja     0x400fad <phase_3+106>
   0x0000000000400f71 <+46>:	mov    0x8(%rsp),%eax
   0x0000000000400f75 <+50>:	jmpq   *0x402470(,%rax,8)
   0x0000000000400f7c <+57>:	mov    $0xcf,%eax
   0x0000000000400f81 <+62>:	jmp    0x400fbe <phase_3+123>
   0x0000000000400f83 <+64>:	mov    $0x2c3,%eax
   0x0000000000400f88 <+69>:	jmp    0x400fbe <phase_3+123>
   0x0000000000400f8a <+71>:	mov    $0x100,%eax
   0x0000000000400f8f <+76>:	jmp    0x400fbe <phase_3+123>
   0x0000000000400f91 <+78>:	mov    $0x185,%eax
   0x0000000000400f96 <+83>:	jmp    0x400fbe <phase_3+123>
   0x0000000000400f98 <+85>:	mov    $0xce,%eax
   0x0000000000400f9d <+90>:	jmp    0x400fbe <phase_3+123>
   0x0000000000400f9f <+92>:	mov    $0x2aa,%eax
   0x0000000000400fa4 <+97>:	jmp    0x400fbe <phase_3+123>
   0x0000000000400fa6 <+99>:	mov    $0x147,%eax
   0x0000000000400fab <+104>:	jmp    0x400fbe <phase_3+123>
   0x0000000000400fad <+106>:	callq  0x40143a <explode_bomb>
   0x0000000000400fb2 <+111>:	mov    $0x0,%eax
   0x0000000000400fb7 <+116>:	jmp    0x400fbe <phase_3+123>
   0x0000000000400fb9 <+118>:	mov    $0x137,%eax
   0x0000000000400fbe <+123>:	cmp    0xc(%rsp),%eax
   0x0000000000400fc2 <+127>:	je     0x400fc9 <phase_3+134>
   0x0000000000400fc4 <+129>:	callq  0x40143a <explode_bomb>
   0x0000000000400fc9 <+134>:	add    $0x18,%rsp
   0x0000000000400fcd <+138>:	retq   
```

1. 我們粗看一遍`phase 3`的程式碼，可以看到`lea` ，`sscanf()`這兩個關鍵指令，可以推測出它要求我們輸入2個參數(`%rcx`與`%rdx`這兩個暫存器存放輸入的局部變數)。為了驗證假設，我們打印輸出`x/s $0x4025cf`可以得到`"%d %d"`以此證實假設正確

2. 得知輸入參數個數後，透過`info r`獲取當前`%rsp`位置`0x7fffffffdf10`，並打印出`%rsp`加上`0x8`以及`0xc`的值 

   - `x/u 0x7fffffffdf18`&#8594; 輸入參數1

   - `x/u 0x7fffffffdf1c`&#8594; 輸入參數2

3. 從`cmpl   $0x7,0x8(%rsp)` 、`jg     0x400f6a <phase_3+39> `這兩條語句可以發現，若參數1大於7則會直接引爆炸彈，所以參輸1必須小於等於7(輸入負數會轉換成`unsigned`)

4. `jmpq   *0x402470(,%rax,8)`這條語句非常關鍵，它的作用類似C的switch邏輯語句，它會依照輸入參數1的值去作條跳轉(搜尋jump table)，跳轉地址為記憶體位置`8 * %rax + 0x402470`儲存的值，我們可以用`x/x`來查看。可以依照switch中p1,p2來組合解答，例如`1 331`就是一組正確的解。

```c
/* 假設p1為參數1;p2為參數2 */
unsigned res; // %rax

switch(p1){
    case 0:
        res = 0xcf; // 207
        break;
    case 1:
        res = 0x137; // 331
        break;
    case 2:
        res = 0x2c3; // 707
        break;
    case 3:
        res = 0x100; // 256
        break;
    case 4:
        res = 0x185; // 389
        break;
    case 5:
        res = 0xce; // 206
        break;
    case 6:
        res = 0x2aa; // 682
        break;
    case 7:
        res = 0x147; // 327
        break;
}

return res == p2; // 參數2的值必須要符合res的對應值
```



### Phase 4

```asm
   0x000000000040100c <+0>:	sub    $0x18,%rsp
   0x0000000000401010 <+4>:	lea    0xc(%rsp),%rcx
   0x0000000000401015 <+9>:	lea    0x8(%rsp),%rdx
   0x000000000040101a <+14>:	mov    $0x4025cf,%esi
   0x000000000040101f <+19>:	mov    $0x0,%eax
   0x0000000000401024 <+24>:	callq  0x400bf0 <__isoc99_sscanf@plt>
   0x0000000000401029 <+29>:	cmp    $0x2,%eax
   0x000000000040102c <+32>:	jne    0x401035 <phase_4+41>
   0x000000000040102e <+34>:	cmpl   $0xe,0x8(%rsp)
   0x0000000000401033 <+39>:	jbe    0x40103a <phase_4+46>
   0x0000000000401035 <+41>:	callq  0x40143a <explode_bomb>
   0x000000000040103a <+46>:	mov    $0xe,%edx
   0x000000000040103f <+51>:	mov    $0x0,%esi
   0x0000000000401044 <+56>:	mov    0x8(%rsp),%edi
   0x0000000000401048 <+60>:	callq  0x400fce <func4>
   0x000000000040104d <+65>:	test   %eax,%eax
   0x000000000040104f <+67>:	jne    0x401058 <phase_4+76>
   0x0000000000401051 <+69>:	cmpl   $0x0,0xc(%rsp)
   0x0000000000401056 <+74>:	je     0x40105d <phase_4+81>
   0x0000000000401058 <+76>:	callq  0x40143a <explode_bomb>
   0x000000000040105d <+81>:	add    $0x18,%rsp
   0x0000000000401061 <+85>:	retq 
```

