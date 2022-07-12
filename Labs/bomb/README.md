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

#### `func4`

```asm
   0x0000000000400fce <+0>:	sub    $0x8,%rsp
   0x0000000000400fd2 <+4>:	mov    %edx,%eax
   0x0000000000400fd4 <+6>:	sub    %esi,%eax
   0x0000000000400fd6 <+8>:	mov    %eax,%ecx
   0x0000000000400fd8 <+10>:	shr    $0x1f,%ecx
   0x0000000000400fdb <+13>:	add    %ecx,%eax
   0x0000000000400fdd <+15>:	sar    %eax
   0x0000000000400fdf <+17>:	lea    (%rax,%rsi,1),%ecx
   0x0000000000400fe2 <+20>:	cmp    %edi,%ecx
   0x0000000000400fe4 <+22>:	jle    0x400ff2 <func4+36>
   0x0000000000400fe6 <+24>:	lea    -0x1(%rcx),%edx
   0x0000000000400fe9 <+27>:	callq  0x400fce <func4>
   0x0000000000400fee <+32>:	add    %eax,%eax
   0x0000000000400ff0 <+34>:	jmp    0x401007 <func4+57>
   0x0000000000400ff2 <+36>:	mov    $0x0,%eax
   0x0000000000400ff7 <+41>:	cmp    %edi,%ecx
   0x0000000000400ff9 <+43>:	jge    0x401007 <func4+57>
   0x0000000000400ffb <+45>:	lea    0x1(%rcx),%esi
   0x0000000000400ffe <+48>:	callq  0x400fce <func4>
   0x0000000000401003 <+53>:	lea    0x1(%rax,%rax,1),%eax
   0x0000000000401007 <+57>:	add    $0x8,%rsp
   0x000000000040100b <+61>:	retq  
```



炸彈2需要使用者輸入2個參數，第一個參數的關鍵在於整個程式流程在於`func4`函式，這是一個遞迴函式。第二個參數相對簡單，只要輸入0就可以通過，所以我們著重分析參數1的判斷

1. 首先先界定範圍`cmpl   $0xe,0x8(%rsp)`這條語句表示輸入參數須小於等於`0xe`(14)否則引爆炸彈
2. 完成第一步後，將`%edx`設為`0xe`，將`%esi`設為`0x0`後進入`func4`
3. `func4`為一個遞迴函式，它會將使用者的輸入參數與`%rdx`與`%rsi`的計算結果進行比較，從而不斷自我調用，將其邏輯轉換成C：

```c
unsigned func4(unsigned input, unsigned* rdx, unsigned* rsi){
    unsigned rax = ((*rdx - *rsi) + ((*rdx - *rsi) >> 31)) / 2;
    unsigned rcx = rax + *rsi;
    
    if(rcx < input){
        *rsi = rcx + 1;
        func4(input, rdx, rsi);
        return 2*rax + 1;
    }else if(rcx > input){
        *rdx = rcx - 1;
        func4(input, rdx, rsi);
        return 2*rax;
    }else{
        return 0;
    }
}
```

4. 其實我們從`phase_4`函式中可以發現，當`%rax`等於0時炸彈就不會引爆，所以我們在解這題時要把注意力放在如何使`%rax`等於0。
5. 由於初始調用的`%rdx`與`%rsi`分別為14以及0，所以我們只需要使輸入`input`等於`rcx`就可以了，由這個計算結果可以得到第一個解`7 0`
6. 當`rcx`的值小於或大於`input`時就會進入遞迴環節，從上面的C代碼可以看出，無論如何我們都不希望`rcx`小於`input`，因為不管`rax`為何，返回值始終不為0。因此在撇除`rcx`與`input`相等的狀況下(7)，剩餘的選項就是0~6
7. 所以我們希望遞迴到底都是從`rcx > imput`這個代碼區塊出發，也就是說每次都調用`func4(input, rcx-1, rsi)`。而經過每次調用其`rcx`分別會是`3,1,0`
8. 經果整個邏輯推敲參數1的可能值分別為`0, 1, 3, 7`



### Phase 5

```asm
   0x0000000000401062 <+0>:	push   %rbx
   0x0000000000401063 <+1>:	sub    $0x20,%rsp
   0x0000000000401067 <+5>:	mov    %rdi,%rbx
   0x000000000040106a <+8>:	mov    %fs:0x28,%rax
   0x0000000000401073 <+17>:	mov    %rax,0x18(%rsp)
   0x0000000000401078 <+22>:	xor    %eax,%eax
   0x000000000040107a <+24>:	callq  0x40131b <string_length>
   0x000000000040107f <+29>:	cmp    $0x6,%eax
   0x0000000000401082 <+32>:	je     0x4010d2 <phase_5+112>
   0x0000000000401084 <+34>:	callq  0x40143a <explode_bomb>
   0x0000000000401089 <+39>:	jmp    0x4010d2 <phase_5+112>
   0x000000000040108b <+41>:	movzbl (%rbx,%rax,1),%ecx
   0x000000000040108f <+45>:	mov    %cl,(%rsp)
   0x0000000000401092 <+48>:	mov    (%rsp),%rdx
   0x0000000000401096 <+52>:	and    $0xf,%edx
   0x0000000000401099 <+55>:	movzbl 0x4024b0(%rdx),%edx
   0x00000000004010a0 <+62>:	mov    %dl,0x10(%rsp,%rax,1)
   0x00000000004010a4 <+66>:	add    $0x1,%rax
   0x00000000004010a8 <+70>:	cmp    $0x6,%rax
   0x00000000004010ac <+74>:	jne    0x40108b <phase_5+41>
   0x00000000004010ae <+76>:	movb   $0x0,0x16(%rsp)
   0x00000000004010b3 <+81>:	mov    $0x40245e,%esi
   0x00000000004010b8 <+86>:	lea    0x10(%rsp),%rdi
   0x00000000004010bd <+91>:	callq  0x401338 <strings_not_equal>
   0x00000000004010c2 <+96>:	test   %eax,%eax
   0x00000000004010c4 <+98>:	je     0x4010d9 <phase_5+119>
   0x00000000004010c6 <+100>:	callq  0x40143a <explode_bomb>
   0x00000000004010cb <+105>:	nopl   0x0(%rax,%rax,1)
   0x00000000004010d0 <+110>:	jmp    0x4010d9 <phase_5+119>
   0x00000000004010d2 <+112>:	mov    $0x0,%eax
   0x00000000004010d7 <+117>:	jmp    0x40108b <phase_5+41>
   0x00000000004010d9 <+119>:	mov    0x18(%rsp),%rax
   0x00000000004010de <+124>:	xor    %fs:0x28,%rax
   0x00000000004010e7 <+133>:	je     0x4010ee <phase_5+140>
   0x00000000004010e9 <+135>:	callq  0x400b30 <__stack_chk_fail@plt>
   0x00000000004010ee <+140>:	add    $0x20,%rsp
   0x00000000004010f2 <+144>:	pop    %rbx
   0x00000000004010f3 <+145>:	retq 
```

1. 從調用`string_length()`後`cmp    $0x6,%eax`的比較指令可以判定輸入的為字串類型，且長度應當為6
2. 接著就是這段程式碼的主要邏輯，透過`movzbl (%rbx,%rax,1),%ecx`這條語句，每次將字串上的字元存放到`%ecx`暫存器上，然後`%rax`加1
3. 然後將當前取出的字元進行AND運算，也就是`and    $0xf,%edx`語句，然後將`%edx`加上`0x4024b0`，並將這個地址存放的字元儲存到棧上
4. 由此可知這一連串的地址將會存放16個字元信息，我們透過查看記憶體`x/16c 0x4024b0`可以打印出

| 與0xf AND運算結果 | 對應字元 |
| :---------------: | :------: |
|         0         |    m     |
|         1         |    a     |
|         2         |    d     |
|         3         |    u     |
|         4         |    i     |
|         5         |    e     |
|         6         |    r     |
|         7         |    s     |
|         8         |    n     |
|         9         |    f     |
|         A         |    o     |
|         B         |    t     |
|         C         |    v     |
|         D         |    b     |
|         E         |    y     |
|         F         |    l     |

5. 然後在調用`string_not_equal()`之前可以發現`mov    $0x40245e,%esi`更新`%rsi`語句，推測就是目標字串，
   透過`x/s 0x40245e`打印出字串`flyers`，因此我們需要對應上表中的映射關係，組合出`flyers`這個單字
6. 在終端機中輸入`man ascii`查看字元對應的hex form，然後組合出解答，例如`ionefg`或`IONEFG`



### Phase 6

```asm
   0x00000000004010f4 <+0>:	push   %r14
   0x00000000004010f6 <+2>:	push   %r13
   0x00000000004010f8 <+4>:	push   %r12
   0x00000000004010fa <+6>:	push   %rbp
   0x00000000004010fb <+7>:	push   %rbx
   0x00000000004010fc <+8>:	sub    $0x50,%rsp
   0x0000000000401100 <+12>:	mov    %rsp,%r13
   0x0000000000401103 <+15>:	mov    %rsp,%rsi
   0x0000000000401106 <+18>:	callq  0x40145c <read_six_numbers>
   0x000000000040110b <+23>:	mov    %rsp,%r14
   0x000000000040110e <+26>:	mov    $0x0,%r12d
   0x0000000000401114 <+32>:	mov    %r13,%rbp
   0x0000000000401117 <+35>:	mov    0x0(%r13),%eax
   0x000000000040111b <+39>:	sub    $0x1,%eax
   0x000000000040111e <+42>:	cmp    $0x5,%eax
   0x0000000000401121 <+45>:	jbe    0x401128 <phase_6+52>
   0x0000000000401123 <+47>:	callq  0x40143a <explode_bomb>
   0x0000000000401128 <+52>:	add    $0x1,%r12d
   0x000000000040112c <+56>:	cmp    $0x6,%r12d
   0x0000000000401130 <+60>:	je     0x401153 <phase_6+95>
   0x0000000000401132 <+62>:	mov    %r12d,%ebx
   0x0000000000401135 <+65>:	movslq %ebx,%rax
   0x0000000000401138 <+68>:	mov    (%rsp,%rax,4),%eax
   0x000000000040113b <+71>:	cmp    %eax,0x0(%rbp)
   0x000000000040113e <+74>:	jne    0x401145 <phase_6+81>
   0x0000000000401140 <+76>:	callq  0x40143a <explode_bomb>
   0x0000000000401145 <+81>:	add    $0x1,%ebx
   0x0000000000401148 <+84>:	cmp    $0x5,%ebx
   0x000000000040114b <+87>:	jle    0x401135 <phase_6+65>
   0x000000000040114d <+89>:	add    $0x4,%r13
   0x0000000000401151 <+93>:	jmp    0x401114 <phase_6+32>
   0x0000000000401153 <+95>:	lea    0x18(%rsp),%rsi
   0x0000000000401158 <+100>:	mov    %r14,%rax
   0x000000000040115b <+103>:	mov    $0x7,%ecx
   0x0000000000401160 <+108>:	mov    %ecx,%edx
   0x0000000000401162 <+110>:	sub    (%rax),%edx
   0x0000000000401164 <+112>:	mov    %edx,(%rax)
   0x0000000000401166 <+114>:	add    $0x4,%rax
   0x000000000040116a <+118>:	cmp    %rsi,%rax
   0x000000000040116d <+121>:	jne    0x401160 <phase_6+108>
   0x000000000040116f <+123>:	mov    $0x0,%esi
   0x0000000000401174 <+128>:	jmp    0x401197 <phase_6+163>
   0x0000000000401176 <+130>:	mov    0x8(%rdx),%rdx
   0x000000000040117a <+134>:	add    $0x1,%eax
   0x000000000040117d <+137>:	cmp    %ecx,%eax
   0x000000000040117f <+139>:	jne    0x401176 <phase_6+130>
   0x0000000000401181 <+141>:	jmp    0x401188 <phase_6+148>
   0x0000000000401183 <+143>:	mov    $0x6032d0,%edx
   0x0000000000401188 <+148>:	mov    %rdx,0x20(%rsp,%rsi,2)
   0x000000000040118d <+153>:	add    $0x4,%rsi

```

`phase_6`其實是一個不斷將節點連接成**鏈狀表**的程式

1. 首先從輸入可以得知參數數量為6，由`sub    $0x1,%eax`與`cmp    $0x5,%eax`得知參數範圍需要小於等於6
2. 逐一檢查輸入參數，每個參數必須是獨一無二的
3. 接著用7去循環減去輸入的參數，若當初輸入為`1 2 3 4 5 6`則結果會是`6 5 4 3 2 1`
4. 上述的運算結果將會被當成偏移量不斷的動記憶體位置，其實這步就是在遍歷整個鏈狀表。每次取從當前node算起的第n個node，n取決於上面計算的數值

```c
typedef struct{
    int val;
    struct node* next;
}node;
```

5. 從上面的結構體可以推測出節點變數的記憶體位置，因此`mov    0x8(%rdx),%rdx`這部其實就是取next並指向存放在棧上的數據(各節點地址)，所以`%rbx`就是存放節點的地址，透過`x/uw可以查看val大小`

   - 每個節點對應的`val`

     - `node6.val = 433`

     - `node5.val = 477`

     - `node4.val = 691`

     - `node3.val = 924`

     - `node2.val = 168`

     - `node1.val = 332`

6. 組合完成的鏈狀表的`val`值都要大於等於next的`val`值

7. 依照順序將每個節點串起來，因此排列後結果為`node3 --> node4 --> node5 --> node6 --> node1 --> node2 --> null`，所以反推輸入應當為`4 3 2 1 6 5`

```c
typedef int[6] six_arr;

Node node[6] = ({332,NULL},{168,NULL},
                {924,NULL},{691,NULL},
				{477,NULL},{433,NULL}};

void phase_6(six_arr arr){
    for(int i=0; i<6; i++){
        for(int j=0; j<6; j++){
            if(arr[i] == arr[j]){
                // bomb!
            }
        }
    }
    
    for(int i=0; i<6; i++){
        arr[i] = 7-arr[i];
    }
        
    for(int i=0; i<5; i++){
        node[arr[i]].next = node[arr[i+1]];
    }
    node[arr[5]] = NULL;
    
    for(int i=0; i<5; i++){
        if(node[arr[i]].val < node[arr[i+1]].val){
            // bomb!
        }
    }
    
}
```



### Secret Phase

當我們解完第6個炸彈後會看到下面這段註解，貌似還有什麼為完成

```c
    /* Wow, they got it!  But isn't something... missing?  Perhaps
     * something they overlooked?  Mua ha ha ha ha! */
```

其實隱層彩蛋就躲在`phase_defused()`裡頭，當輸入數據等於6行時就會觸發。但是要進入`secret_phase`還有令一個條件，我們需要將`phase_3`的輸出改為3個，最後一個為字串類型

```asm
   0x00000000004015c4 <+0>:	sub    $0x78,%rsp
   0x00000000004015c8 <+4>:	mov    %fs:0x28,%rax
   0x00000000004015d1 <+13>:	mov    %rax,0x68(%rsp)
   0x00000000004015d6 <+18>:	xor    %eax,%eax
   0x00000000004015d8 <+20>:	cmpl   $0x6,0x202181(%rip)        # 0x603760 <num_input_strings>
   0x00000000004015df <+27>:	jne    0x40163f <phase_defused+123>
   0x00000000004015e1 <+29>:	lea    0x10(%rsp),%r8
   0x00000000004015e6 <+34>:	lea    0xc(%rsp),%rcx
   0x00000000004015eb <+39>:	lea    0x8(%rsp),%rdx
   0x00000000004015f0 <+44>:	mov    $0x402619,%esi
   0x00000000004015f5 <+49>:	mov    $0x603870,%edi
   0x00000000004015fa <+54>:	callq  0x400bf0 <__isoc99_sscanf@plt>
   0x00000000004015ff <+59>:	cmp    $0x3,%eax
   0x0000000000401602 <+62>:	jne    0x401635 <phase_defused+113>
   0x0000000000401604 <+64>:	mov    $0x402622,%esi
   0x0000000000401609 <+69>:	lea    0x10(%rsp),%rdi
   0x000000000040160e <+74>:	callq  0x401338 <strings_not_equal>
   0x0000000000401613 <+79>:	test   %eax,%eax
   0x0000000000401615 <+81>:	jne    0x401635 <phase_defused+113>
   0x0000000000401617 <+83>:	mov    $0x4024f8,%edi
   0x000000000040161c <+88>:	callq  0x400b10 <puts@plt>
   0x0000000000401621 <+93>:	mov    $0x402520,%edi
   0x0000000000401626 <+98>:	callq  0x400b10 <puts@plt>
   0x000000000040162b <+103>:	mov    $0x0,%eax
   0x0000000000401630 <+108>:	callq  0x401242 <secret_phase>
   0x0000000000401635 <+113>:	mov    $0x402558,%edi
   0x000000000040163a <+118>:	callq  0x400b10 <puts@plt>
   0x000000000040163f <+123>:	mov    0x68(%rsp),%rax
   0x0000000000401644 <+128>:	xor    %fs:0x28,%rax
   0x000000000040164d <+137>:	je     0x401654 <phase_defused+144>
   0x000000000040164f <+139>:	callq  0x400b30 <__stack_chk_fail@plt>
   0x0000000000401654 <+144>:	add    $0x78,%rsp
   0x0000000000401658 <+148>:	retq
```

#### `secret_phase`

```asm
   0x0000000000401242 <+0>:	push   %rbx
   0x0000000000401243 <+1>:	callq  0x40149e <read_line>
   0x0000000000401248 <+6>:	mov    $0xa,%edx
   0x000000000040124d <+11>:	mov    $0x0,%esi
   0x0000000000401252 <+16>:	mov    %rax,%rdi
   0x0000000000401255 <+19>:	callq  0x400bd0 <strtol@plt>
   0x000000000040125a <+24>:	mov    %rax,%rbx
   0x000000000040125d <+27>:	lea    -0x1(%rax),%eax
   0x0000000000401260 <+30>:	cmp    $0x3e8,%eax
   0x0000000000401265 <+35>:	jbe    0x40126c <secret_phase+42>
   0x0000000000401267 <+37>:	callq  0x40143a <explode_bomb>
   0x000000000040126c <+42>:	mov    %ebx,%esi
   0x000000000040126e <+44>:	mov    $0x6030f0,%edi
   0x0000000000401273 <+49>:	callq  0x401204 <fun7>
   0x0000000000401278 <+54>:	cmp    $0x2,%eax
   0x000000000040127b <+57>:	je     0x401282 <secret_phase+64>
   0x000000000040127d <+59>:	callq  0x40143a <explode_bomb>
   0x0000000000401282 <+64>:	mov    $0x402438,%edi
   0x0000000000401287 <+69>:	callq  0x400b10 <puts@plt>
   0x000000000040128c <+74>:	callq  0x4015c4 <phase_defused>
   0x0000000000401291 <+79>:	pop    %rbx
   0x0000000000401292 <+80>:	retq  
```

#### func7

```asm
   0x0000000000401204 <+0>:	sub    $0x8,%rsp
   0x0000000000401208 <+4>:	test   %rdi,%rdi
   0x000000000040120b <+7>:	je     0x401238 <fun7+52>
   0x000000000040120d <+9>:	mov    (%rdi),%edx
   0x000000000040120f <+11>:	cmp    %esi,%edx
   0x0000000000401211 <+13>:	jle    0x401220 <fun7+28>
   0x0000000000401213 <+15>:	mov    0x8(%rdi),%rdi
   0x0000000000401217 <+19>:	callq  0x401204 <fun7>
   0x000000000040121c <+24>:	add    %eax,%eax
   0x000000000040121e <+26>:	jmp    0x40123d <fun7+57>
   0x0000000000401220 <+28>:	mov    $0x0,%eax
   0x0000000000401225 <+33>:	cmp    %esi,%edx
   0x0000000000401227 <+35>:	je     0x40123d <fun7+57>
   0x0000000000401229 <+37>:	mov    0x10(%rdi),%rdi
   0x000000000040122d <+41>:	callq  0x401204 <fun7>
   0x0000000000401232 <+46>:	lea    0x1(%rax,%rax,1),%eax
   0x0000000000401236 <+50>:	jmp    0x40123d <fun7+57>
   0x0000000000401238 <+52>:	mov    $0xffffffff,%eax
   0x000000000040123d <+57>:	add    $0x8,%rsp
   0x0000000000401241 <+61>:	retq 
```

- 首先對字串`bt`作以下處理

```c
char str[]="bt";
char* ptr=NULL;
long res;

res = strol(str, &ptr, 10); // res = 0; ptr = "bt"
```

- 需要輸入一個參數(小於等於1001)
- 調用`fun7()` &rarr; `%rdi為"$(36)"`
- `fun7()`為一個遞迴函式，循環遍歷整個數結構
- `%rax`返回值需要剛好等於2
  - 往左後再往右找到
  - 深度應該是4
  - 絕對不能遍歷到null
  - 用1001去看右節點
- `%rsi輸入參數 %rdx節點val`



```c
typedef struct{
    unsigned val;
    Node* left;
    Node* right;
}Node;

unsigned fun7(Node* n, int val){
    if(n == NULL){
   		reutrn 0xffffffff;     
    }
    
    if(n->val == val){
        return 0;
    }else if(n->val < val){
        return 2*(fun7(n->right, val))+1;
    }else{
        return 2*fun7(n->right, val);
    }
    
}
```



