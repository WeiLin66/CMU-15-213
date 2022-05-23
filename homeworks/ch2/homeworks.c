/*****************************
 * @brief CSAPP章節二回家作業解答
 * @author 林浩為
 * @version v0.1 
 * @date 2022-05
 ****************************/

#include "homeworks.h"

/**
 * @brief 2.55, 2.56 檢視機器記憶體除存格式
 * 
 * @param pointer uint8* type pointer 
 * @param size data length
 */
void show_bytes(u8_pointer pointer, int size){

    for(int i=0; i<size; i++){
        printf("0x%X ", *pointer);
        pointer++;
    }
    printf("\n");
}

/**
 * 
 * 2.57 show_short, show_long, show_double 檢視short, long, double類型的記憶體存儲格式
 * 
 */
void show_short(u16_pointer pointer){
    show_bytes((u8_pointer)pointer, sizeof(unsigned short));
}

void show_long(u64_pointer pointer){
    show_bytes((u8_pointer)pointer, sizeof(unsigned long));
}

void show_double(d_pointer pointer){
    show_bytes((u8_pointer)pointer, sizeof(double));
}

/**
 * @brief 2.58 判斷是否為小端模式
 * 
 * @param pointer 
 * @param size 
 * @return true 
 * @return false 
 */
bool is_little_endian(u8_pointer pointer, int size){
    uint8_t* copy_mem = (uint8_t*)malloc(size);
    memcpy(copy_mem, pointer, size);

    for(int i=0; i<size; i++){
        if(*(pointer++) != copy_mem[i]){
            return false;
        }
    }

    free(copy_mem);
    copy_mem = NULL;

    return true;
}

/**
 * @brief hw 2.59 將y的LSB替換成x的LSB
 * 
 * @param x 
 * @param y 
 * @return unsigned 
 */
unsigned lsbx_and_y(unsigned x, unsigned y){
    return (y & ~0xff) | (x & 0xff);
}

/**
 * @brief hw 2.60 將x的第i byte替換成b
 * 
 * @param x 
 * @param i 
 * @param b 
 * @return unsigned 
 */
unsigned replace_byte(unsigned x, int i, unsigned char b){
    return (x & ~(0xff << i*8)) | (b << i*8);
}

/**
 * 以下作業禁止使用: 一般的循環語句、條件語句、函式調用、加減乘除模、對比運算
 * 僅支持: 所有的位級以及邏輯運算、== !=、加減法、強制/隱式類型轉換、整形常數INT_MIN, INT_MAX
 */

/**
 * @brief hw 2.61
 * 返回1，當x任何位都是1或都是0時，最高有效byte為0；最低有效byte為1
 * @param x 
 * @return int 
 */
int conditional_one_zero(unsigned x){
    if((x ^ (~0x0)) == 0x0 || (x & (~0x0)) == 0x0){
        return 1;
    }else if(((x & 0xff) == 0xff) || (((x >> 24) & 0xff) == 0x0)){
        return 1;
    }else{
        return 0;
    }
}

/**
 * @brief hw 2.62
 * 判斷該機器對int類型整數式使用邏輯右移還是算術左移
 * @return int 
 */
int int_shifts_are_arithmetic(){
    int t = -1;
    int s = (t >> 31) & 0x01;
    if((((t >> 1) >> 31) & 0x01) == s){
        return 1;
    }else{
        return 0;
    }
}

/**
 * @brief hw 2.63
 * 補足函式程式碼完成邏輯與算術右移需求
 */
unsigned srl(unsigned x, int k){
    /* Perform shift arithmetically */
    unsigned xsra = (int) x >> k;

    for(int i=0; i<k; i++){
        xsra &= ~(0x01 << (31-i));
    }

    return xsra;
}

int sra(int x, int k){
    /* Perform shift logically */
    int xsrl = (unsigned)x >> k;
    int shift = (xsrl >> (31-k)) & 0x01;

    for(int i=0; i<k; i++){
        xsrl |= (shift << (31-i));
    }

    return xsrl;
}

/**
 * @brief hw 2.64
 * 判斷x的任何基數位為1則返回1，反之則返回0
 * @param x 
 * @return int 
 */
int any_odd_one(unsigned x){
    if(x == 0){
        return 0;
    }

    for(int i=0; i<3; i++){
        if(((x & 0xff) & 0x55) != 0){
            return 1;
        }
        x >>= 8;
    }
    return 0;
}

/**
 * @brief hw 2.65
 * 判斷x是否含有基數個1
 * 只能使用算術運算、邏輯運算、位運算
 * 限定只能使用12個運算符
 * @param x 
 * @return int 
 */
int odd_ones(unsigned x){
    unsigned b16 = (x >> 16) ^ x;
    unsigned b8 = (b16 >> 8) ^ b16;
    unsigned b4 = (b8 >> 4) ^ b8;
    unsigned b2 = (b4 >> 2) ^ b4;

    return ((b2 >> 1) ^ b2) & 0x01; 
}

/**
 * @brief hw 2.66
 * 求表示x的最大比特位mask
 * @param x 
 * @return int 
 */
int leftmost_one(unsigned x) {
    x |= x >> 1;
    x |= x >> 2;
    x |= x >> 4;
    x |= x >> 8;
    x |= x >> 16;

    return (x >> 1) + 0x1;
}

int main(){
    unsigned test = 0x7f000001;
    printf("0x%X\n", leftmost_one(test));

    return 0;
}