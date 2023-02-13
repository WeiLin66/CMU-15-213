
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

## 實驗須知

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
      - 若無法擴充，則向後尋找新塊，並返回該塊的地址並將舊塊釋放掉
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
      - `mem_sbrk()`
      - `mem_heap_lo()`
      - `mem_heap_hi()`
      - `mem_heapsize()`
      - `mem_pagesize()`
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
      - 執行驗證時會測試程式的正確性、記憶體使用效率、效能等
      - 輸入指令參數
        - -a 不要檢查team structure (交作業的個人信息)
        - -g 生成總結信息
        - -t <tracedir>使用指定地址內的trace files
        - -f <tracefile>使用指定的trace file
        - -c <tracefile>只執行trace file一次，目的在於驗證正確性。便於debug
        - -h 打印所有指令參數
        - -l 執行libc版本的m-alloc，比對跟我們編寫的分配器之間效能差距
        - -V 打印詳細執行信息
        - -v <verbose level>手動設定打印消息等級
        - -d \<i\>設定debug等級
          - 預設1，可調範圍: 0~2，對塊中的內容作合法性檢測
        - -D 相當於-d2
        - -s 經過\<s\>秒後超時
          - 預設沒有超時限制
    - 驗證項目
      - 空間使用率 &rarr; 如何減少fragmentation
      - 吞吐量 &rarr; 單位時間(秒)內執行次數
      - Heap Consistency Checker驗證項目
        - 堆檢查
          - 序言塊與結尾塊
          - 塊地址是否對齊
          - 堆邊界檢查
          - 檢查塊的header與footer是否存放相同內容
  
        - 空閒列表檢查
          - 塊的next, pre指標是否正確
          - 空閒塊指向地址在堆區間之中
          - 若使用分離式空閒列表，確保塊大小符合class範圍
  
      - 程式碼風格
        - 函式性編程
        - global全局變數使用的越少越好
        - 利用宏定義和內嵌函數代替繁雜的指標運算(可讀性、可複用性問題)
        - 為函式添加header註解
        - 程式開始前需要註解該動態分配器的功能、使用結構、具體邏輯、空閒列表等等資訊
  
  - 一些有用提示
    - 在編寫初期運用mdriver -c option或mdriver -f option來debug會非常有幫助。起初可以先選擇包含操作指令較少的trace file
    - 使用mdriver -v options來查看詳細資訊
    - 使用mdriver -d option來快速找出錯誤
    - 在性能測試前巧秒的使用gdb來調試
      - 可以透過修改Makefile來完成 &rarr; 為gcc添加-g選項並移除優化選項-O2
    - 使用gdb中的watch監看那些值被非預期改動
    - 將指標的運算操作(例如加減乘除移位)封裝在宏定義以及內嵌函數中
    - 書本案例運行在32 bits機器上，porting時要注意，我們使用的機器為**64 bits**
    - 任何時刻修正完代碼後，記得調用`mm_checkheap`來檢測堆消息，這對debug、追蹤問題特別有效。實驗為其配分其中很大的原因就是希望學生養成撰寫這類函式的習慣
    - 使用性能工具優化性能
      - valgrind
      - gprof
    - 版本控管
      - 每當對已成功運行的分配器進行修改時，記得將原先的程式備份起來，以免發生不預期的問題導致程式碼無法復原(好習慣)
      - 管理使用不同版本的空閒列表機制
    - 可以採納的資料結構與演算法
      - 管理空閒塊的結構
        - 顯式空閒列表
        - 隱式空閒列表
        - 分離式空閒列表
      - 尋找空閒塊的演算法
        - First fit
        - Next fit
        - 使用first fit搜索案地址分配的空閒塊
        - Best fit
      - 官方推薦順序: 隱式 &rarr; 顯式 &rarr; 分離式

## 實驗紀錄
### 隱式空閒列表
- first fit
- next fit

### 顯式空閒列表
- LIFO + first fit

### 分離式空閒列表
- LIFO + first fit