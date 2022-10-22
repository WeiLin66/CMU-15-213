/* 
 * trans.c - Matrix transpose B = A^T
 *
 * Each transpose function must have a prototype of the form:
 * void trans(int M, int N, int A[N][M], int B[M][N]);
 *
 * A transpose function is evaluated by counting the number of misses
 * on a 1KB direct mapped cache with a block size of 32 bytes.
 */ 
#include <stdio.h>
#include "cachelab.h"

int is_transpose(int M, int N, int A[N][M], int B[M][N]);

/* 
 * transpose_submit - This is the solution transpose function that you
 *     will be graded on for Part B of the assignment. Do not change
 *     the description string "Transpose submission", as the driver
 *     searches for that string to identify the transpose function to
 *     be graded. 
 */
#define BLOCK 8
char transpose_submit_desc[] = "Transpose submission";
void transpose_submit(int M, int N, int A[N][M], int B[M][N]){

    int i, j, i1;

    for (i = 0; i < M; i += BLOCK) {

        for (j = 0; j < N; j += BLOCK) {
            
            /* B * B mini block */
            for(i1=i; i1<i+BLOCK; i1++) {

                int temp_0 = A[i1][j];
                int temp_1 = A[i1][j+1];
                int temp_2 = A[i1][j+2];
                int temp_3 = A[i1][j+3];
                int temp_4 = A[i1][j+4];
                int temp_5 = A[i1][j+5];
                int temp_6 = A[i1][j+6];
                int temp_7 = A[i1][j+7];

                B[j][i1] = temp_0;
                B[j+1][i1] = temp_1;
                B[j+2][i1] = temp_2;
                B[j+3][i1] = temp_3;
                B[j+4][i1] = temp_4;
                B[j+5][i1] = temp_5;
                B[j+6][i1] = temp_6;
                B[j+7][i1] = temp_7;
            }
        }
    }
}

/* 
 * You can define additional transpose functions below. We've defined
 * a simple one below to help you get started. 
 */ 

/* 
 * trans - A simple baseline transpose function, not optimized for the cache.
 */
char trans_desc[] = "Simple row-wise scan transpose";
void trans(int M, int N, int A[N][M], int B[M][N])
{
    int i, j, tmp;

    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            tmp = A[i][j];
            B[j][i] = tmp;
        }
    }    

}

/* 
 * trans32*32
 */
char trans_desc32[] = "Simple transpose func32";
void trans32(int M, int N, int A[N][M], int B[M][N]){ 

    int i, j, i1, temp[BLOCK];

    for (i = 0; i < M; i += BLOCK) {

        for (j = 0; j < N; j += BLOCK) {
            
            /* B * B mini block */
            for(i1=i; i1<i+BLOCK; i1++) {

                temp[0] = A[i1][j];
                temp[1] = A[i1][j+1];
                temp[2] = A[i1][j+2];
                temp[3] = A[i1][j+3];
                temp[4] = A[i1][j+4];
                temp[5] = A[i1][j+5];
                temp[6] = A[i1][j+6];
                temp[7] = A[i1][j+7];

                B[j][i1] = temp[0];
                B[j+1][i1] = temp[1];
                B[j+2][i1] = temp[2];
                B[j+3][i1] = temp[3];
                B[j+4][i1] = temp[4];
                B[j+5][i1] = temp[5];
                B[j+6][i1] = temp[6];
                B[j+7][i1] = temp[7];
            }
        }
    }

}

/*
 * registerFunctions - This function registers your transpose
 *     functions with the driver.  At runtime, the driver will
 *     evaluate each of the registered functions and summarize their
 *     performance. This is a handy way to experiment with different
 *     transpose strategies.
 */
void registerFunctions()
{
    /* Register your solution function */
    registerTransFunction(transpose_submit, transpose_submit_desc); 

    /* Register any additional transpose functions */
    registerTransFunction(trans, trans_desc); 

    registerTransFunction(trans32, trans_desc32);

}

/* 
 * is_transpose - This helper function checks if B is the transpose of
 *     A. You can check the correctness of your transpose by calling
 *     it before returning from the transpose function.
 */
int is_transpose(int M, int N, int A[N][M], int B[M][N])
{
    int i, j;

    for (i = 0; i < N; i++) {
        for (j = 0; j < M; ++j) {
            if (A[i][j] != B[j][i]) {
                return 0;
            }
        }
    }
    return 1;
}

