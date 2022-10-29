# Cache Labb實驗紀錄

## Intro

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

### You will modifying and handing in these two files
`csim.c`       Your cache simulator
`trans.c`      Your transpose function

### Tools for evaluating your simulator and transpose function
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

## Part. A

該實驗值主要編寫一個LRU策略的`cache simulator`，透過讀取``.trace`文件指令模擬`cache`的操作，並對`hits, miss, eviction`的數量進行分析

### 環境配置

在Part. A中我們需要讀取`trace`文件中的命令來評估`cache`性能，而這些`trace`文件是由`Linux`中的`Valgrind`軟體生成的

> 輸入 `sudo apt install valgrind` 進行安裝

完成安裝後可以透過指令`valgrind --log-fd=1 --tool=lackey -v --trace-mem=yes ls -l`，追蹤目錄下所有可執行文件的`cache`使用狀況

輸入完指令後可以在終端中看到以下格式的輸出，它代表程式執行過程中`cache`內對應的操作

- I &rarr; 讀取指令(該實驗不需要實現該指令)
- M &rarr; 修改數據並存回緩存
- L &rarr; 讀取數據
- S &rarr; 儲存數據

> 格式： [操作指令], [64 bits地址], [數據大小]

![image-20221029145935169](https://raw.githubusercontent.com/WeiLin66/pictures/main/image-20221029145935169.png)

### 實驗須知

1. 我們需要在`csim.c`中實現虛擬緩存
2. 由於測試時會使用不同的``sets`, `line`, `block`緩存使用的記憶體空間要使用`malloc`動態分配
3. `trace`資料夾包括要所有要讀取的`.trace`文件
4. 可以調用`cachelab.c/.h`中的helper function，例如`printfSummary()`



### 如何進行驗證



### 程式撰寫

![image-20221029161404919](https://raw.githubusercontent.com/WeiLin66/pictures/main/image-20221029161404919.png)
