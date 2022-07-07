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
  - info g
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

- shell clear

  - 清除畫面

  

## 實驗過程

### 前言

該實驗需要我們找出每個炸彈的解碼密碼，由於其他source, header files缺失，我們僅能透過可執行檔``bomb`進行GDB調適，並在組合語言層面找出相對應的字串密碼。因此在開始前請先安裝GDB以及熟悉基本的GDB指令。



### Phase 1

主要是`strings_not_equal`這個函式在判斷輸入字串與密碼字串是否相等，是則返回`true`，否則返回`false`

`strings_not_equal`函式主要包含

1. `string_length`函式，用來逐一檢查字元，判斷字串長度
2. `strings_not_equal`後續就是逐一對字元進行比對，若不同則返回`false`，直到`\0`為止
3. 可以發現`strings_not_equal`具有兩個參數，一個是輸入參數，一個是解答字串，分別為`%rdi %rsi`，這個解碼字串儲存位置位於`0x402400`
4. 因此在GDB中直接打印該地址就可以查看該字串`x/s 0x402400`，輸出結果為`Border relations with Canada have never been better.`



### Phase 2

