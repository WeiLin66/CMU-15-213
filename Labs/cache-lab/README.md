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

完成安裝後可以透過指令`valgrind --log-fd=1 --tool=lackey -v --trace-mem=yes ls -l`，追蹤目錄下所有可執行文件的``cache`使用狀況

![image-20221029145935169](/home/zach/snap/typora/76/.config/Typora/typora-user-images/image-20221029145935169.png)

### 檔案說明





### 程式撰寫





### 如何進行驗證
