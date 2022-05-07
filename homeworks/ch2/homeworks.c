#include "homeworks.h"

/**
 * @brief 2.55, 2.56
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
 * 2.57 show_short, show_long, show_double
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
 * @brief 2.58
 * 
 * @param pointer 
 * @param size 
 * @return true 
 * @return false 
 */
bool is_little_endian(u8_pointer pointer, int size){
    
    for(int i=0; i<size; i++){
        
    }

    return 1;
}

int main(){
    long l = 10;
    show_long(&l);

    return 0;
}