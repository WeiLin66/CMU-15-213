# Data Lab實驗記錄

## 實驗環境搭建

> *建議使用Ubuntu作業系統或其他Linux distros*

1. 首先從Github拉取該repo
2. 複製Lab_Backup中的Lab檔
3. 軟體配置
   1. 安裝gcc: `sudo apt-get install gcc`(已安裝請跳過)
   2. 安裝make: `sudo apt -y install make`(已安裝請跳過)
4. 編寫腳本(可選)

⚠️使用腳本的目的是為了省略`make`, `make` `clean`, `./btest`

開啟lab並在資料夾中建立`run.sh，`，並將下列程式碼複製到文件中

```bash
#/bin/bash
make clean
make
./btest
```

5. 完成後每次編譯只需要在終端中輸入`./run.sh即可`
6. `./dlc -e bits.c`查看執行operator的次數

## 解答

```c
//1
/* 
 * bitXor - x^y using only ~ and & 
 *   Example: bitXor(4, 5) = 1
 *   Legal ops: ~ &
 *   Max ops: 14
 *   Rating: 1
 */
int bitXor(int x, int y) {
  return (~(x&y))&(~(~x&~y));
}
/* 
 * tmin - return minimum two's complement integer 
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 4
 *   Rating: 1
 */
int tmin(void) {
  return 0x01 << 31;
}
//2
/*
 * isTmax - returns 1 if x is the maximum, two's complement number,
 *     and 0 otherwise 
 *   Legal ops: ! ~ & ^ | +
 *   Max ops: 10
 *   Rating: 1
 */
int isTmax(int x) {
    return (!(~(x^(x+1)))) & (!(!(x^(~0x00000000))));
}
/* 
 * allOddBits - return 1 if all odd-numbered bits in word set to 1
 *   where bits are numbered from 0 (least significant) to 31 (most significant)
 *   Examples allOddBits(0xFFFFFFFD) = 0, allOddBits(0xAAAAAAAA) = 1
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 12
 *   Rating: 2
 */
int allOddBits(int x) {
  return !(~(x | 0x55 | (0x55 << 8) | (0x55 << 16) | (0x55 << 24)));
}
/* 
 * negate - return -x 
 *   Example: negate(1) = -1.
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 5
 *   Rating: 2
 */
int negate(int x) {
  return (~x) + 0x01;;
}
//3
/* 
 * isAsciiDigit - return 1 if 0x30 <= x <= 0x39 (ASCII codes for characters '0' to '9')
 *   Example: isAsciiDigit(0x35) = 1.
 *            isAsciiDigit(0x3a) = 0.
 *            isAsciiDigit(0x05) = 0.
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 15
 *   Rating: 3
 */
int isAsciiDigit(int x) {
  return !(x + (~(0x30) + 0x01) & (0x01 << 31)) & ((x + (~(0x3a) + 0x01) & (0x01 << 31)) >> 31);
}
/* 
 * conditional - same as x ? y : z 
 *   Example: conditional(2,4,5) = 4
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 16
 *   Rating: 3
 */
int conditional(int x, int y, int z) {
  return (((!x) + (~0x00)) & y) | (((~(!x) + 0x01) & z));
}
/* 
 * isLessOrEqual - if x <= y  then return 1, else return 0 
 *   Example: isLessOrEqual(4,5) = 1.
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 24
 *   Rating: 3
 */
int isLessOrEqual(int x, int y) {
  int a = (x >> 31) & 0x1; // check +/-
  int b = (y >> 31) & 0x1; // check +/-

  int c1 = (a & (!b)); // positive overflow
  int c2 = ((!a) & b); // negative overflow

  int e = y + ((~x) + 1);
  int flag = e >> 31; // normal minus operation

return c1 | ((!c2) & (!flag));
}
//4
/* 
 * logicalNeg - implement the ! operator, using all of 
 *              the legal operators except !
 *   Examples: logicalNeg(3) = 0, logicalNeg(0) = 1
 *   Legal ops: ~ & ^ | + << >>
 *   Max ops: 12
 *   Rating: 4 
 */
int logicalNeg(int x) {
  return ((((x ^ (~(0x01 << 31))) + 0x01) >> 31) & 0x01) & (((x ^ (0x01 << 31)) >> 31) & 0x01);
}
/* howManyBits - return the minimum number of bits required to represent x in
 *             two's complement
 *  Examples: howManyBits(12) = 5
 *            howManyBits(298) = 10
 *            howManyBits(-5) = 4
 *            howManyBits(0)  = 1
 *            howManyBits(-1) = 1
 *            howManyBits(0x80000000) = 32
 *  Legal ops: ! ~ & ^ | + << >>
 *  Max ops: 90
 *  Rating: 4
 */
int howManyBits(int x) {
    int b16, b8, b4, b2, b1, b0;
    int flag = x >> 31;
    x = (~flag & x) | (flag & ~x);
    b16 = !!(x >> 16) << 4;
    x >>= b16;
    b8 = !!(x >> 8) << 3;
    x >>= b8;
    b4 = !!(x >> 4) << 2;
    x >>= b4;
    b2 = !!(x >> 2) << 1;
    x >>= b2;
    b1 = !!(x >> 1);
    x >>= b1;
    b0 = x;
  return b0 + b1 + b2 + b4 + b8 + b16 +1;
}
//float
/* 
 * floatScale2 - Return bit-level equivalent of expression 2*f for
 *   floating point argument f.
 *   Both the argument and result are passed as unsigned int's, but
 *   they are to be interpreted as the bit-level representation of
 *   single-precision floating point values.
 *   When argument is NaN, return argument
 *   Legal ops: Any integer/unsigned operations incl. ||, &&. also if, while
 *   Max ops: 30
 *   Rating: 4
 */
unsigned floatScale2(unsigned uf) {
  int flag = (uf >> 31) & 0x01;
  int exp = ((uf >> 23) & 0xff);
  int frac = uf & 0x7fffff;

  // 0
  if(exp == 0 && frac == 0){
    return uf;
  }

  // infinite or NaN
  if(exp == 0xff){
    return uf;
  }

  // denormalize
  if(exp == 0){
    frac <<= 1;
    return (flag << 31) | (exp << 23) | frac;
  }

  // normalize
  uf += (0x01 << 23);
  return uf;
}

/* 
 * floatFloat2Int - Return bit-level equivalent of expression (int) f
 *   for floating point argument f.
 *   Argument is passed as unsigned int, but
 *   it is to be interpreted as the bit-level representation of a
 *   single-precision floating point value.
 *   Anything out of range (including NaN and infinity) should return
 *   0x80000000u.
 *   Legal ops: Any integer/unsigned operations incl. ||, &&. also if, while
 *   Max ops: 30
 *   Rating: 4
 */
int floatFloat2Int(unsigned uf) {
  int flag = (uf >> 31) & 0x01;
  int exp = ((uf >> 23) & 0xff);
  int frac = uf & 0x7fffff;
  int n = exp - 127;

  // 0
  if(exp == 0 && frac == 0){
    return 0;
  }

  // infinite or NaN
  if(n >= 32){
    return 0x80000000u;
  }

  // denormalize
  if(exp == 0){
    return 0;
  }

  // normalize
  float base = 1.0;
  
  flag = (flag == 0x01) ? -1 : 1;

  if((frac >> 22) & 0x01 == 0x01){
    base += 0.5;
  }

  if(n > 0){
    return (int)(flag * base * (1 << n));
  }else if(n == 0){
      return (int) flag * base;
  }else{
    return (int)(flag * base * (1 >> (-n)));
  }
}
/* 
 * floatPower2 - Return bit-level equivalent of the expression 2.0^x
 *   (2.0 raised to the power x) for any 32-bit integer x.
 *
 *   The unsigned value that is returned should have the identical bit
 *   representation as the single-precision floating-point number 2.0^x.
 *   If the result is too small to be represented as a denorm, return
 *   0. If too large, return +INF.
 * 
 *   Legal ops: Any integer/unsigned operations incl. ||, &&. Also if, while 
 *   Max ops: 30 
 *   Rating: 4
 */
unsigned floatPower2(int x) {
  if(x > 127){
    return (0xff << 23);
  }
  
  if(x < -127){
      return 0;
  }

  return (127 + x) << 23;
}
```

## 資源

https://github.com/WeiLin66/CMU-15-213/tree/main/Labs/data-lab
