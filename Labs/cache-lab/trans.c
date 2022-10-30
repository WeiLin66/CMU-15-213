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
char transpose_submit_desc[] = "Transpose submission";
void transpose_submit(int M, int N, int A[N][M], int B[M][N]){

    int i, j, k;
    int temp_0, temp_1, temp_2, temp_3, temp_4, temp_5, temp_6, temp_7;

    /* 64 * 56 */
    for (i = 0; i < 64; i += 8) {

        for (j = 0; j < 56; j += 8) {

            /* A1, A2 --> B1, B2 */
            for (k = 0; k < 4; k++) {

                temp_0 = A[i + k][j];
                temp_1 = A[i + k][j + 1];
                temp_2 = A[i + k][j + 2];
                temp_3 = A[i + k][j + 3];
                temp_4 = A[i + k][j + 4];
                temp_5 = A[i + k][j + 5];
                temp_6 = A[i + k][j + 6];
                temp_7 = A[i + k][j + 7];

                B[j][i + k] = temp_0;
                B[j + 1][i + k] = temp_1;
                B[j + 2][i + k] = temp_2;
                B[j + 3][i + k] = temp_3;

                B[j][i + k + 4] = temp_4;
                B[j + 1][i + k + 4] = temp_5;
                B[j + 2][i + k + 4] = temp_6;
                B[j + 3][i + k + 4] = temp_7;
            }

            /* B2 --> B3, A3 --> B2 */
            for (k = 0; k < 4; k++) {

                temp_0 = A[i + 4][j + k];
                temp_1 = A[i + 5][j + k];
                temp_2 = A[i + 6][j + k];
                temp_3 = A[i + 7][j + k];

                temp_4 = B[j + k][i + 4];
                temp_5 = B[j + k][i + 5];
                temp_6 = B[j + k][i + 6];
                temp_7 = B[j + k][i + 7];

                B[j + k][i + 4] = temp_0;
                B[j + k][i + 5] = temp_1;
                B[j + k][i + 6] = temp_2;
                B[j + k][i + 7] = temp_3;

                B[j + k + 4][i] = temp_4;
                B[j + k + 4][i + 1] = temp_5;
                B[j + k + 4][i + 2] = temp_6;
                B[j + k + 4][i + 3] = temp_7;

            }

            /* A4 --> B4 */
            for (k = 4; k < 8; k++) {

                temp_0 = A[i + k][j + 4];
                temp_1 = A[i + k][j + 5];
                temp_2 = A[i + k][j + 6];
                temp_3 = A[i + k][j + 7];

                B[j + 4][i + k] = temp_0;
                B[j + 5][i + k] = temp_1;
                B[j + 6][i + k] = temp_2;
                B[j + 7][i + k] = temp_3;
            }
        }

        /* top right */
        for(j = 56; j < M; j++){

            temp_0 = A[i][j];
            temp_1 = A[i+1][j];
            temp_2 = A[i+2][j];
            temp_3 = A[i+3][j];
            temp_4 = A[i+4][j];
            temp_5 = A[i+5][j];
            temp_6 = A[i+6][j];
            temp_7 = A[i+7][j];

            B[j][i] = temp_0;
            B[j][i+1] = temp_1;
            B[j][i+2] = temp_2;
            B[j][i+3] = temp_3;
            B[j][i+4] = temp_4;
            B[j][i+5] = temp_5;
            B[j][i+6] = temp_6;
            B[j][i+7] = temp_7;
        }

    }  

    /* buttom left */
    for (i = 64; i < 67; i++) {

        for (j = 0; j < 56; j += 8) {

            temp_0 = A[i][j];
            temp_1 = A[i][j + 1];
            temp_2 = A[i][j + 2];
            temp_3 = A[i][j + 3];
            temp_4 = A[i][j + 4];
            temp_5 = A[i][j + 5];
            temp_6 = A[i][j + 6];
            temp_7 = A[i][j + 7];

            B[j][i] = temp_0;
            B[j + 1][i] = temp_1;
            B[j + 2][i] = temp_2;
            B[j + 3][i] = temp_3;
            B[j + 4][i] = temp_4;
            B[j + 5][i] = temp_5;
            B[j + 6][i] = temp_6;
            B[j + 7][i] = temp_7;
        }
    }

    /* buttom right */
     for (i = 56; i < 61; i++) {

        temp_0 = A[64][i];
        temp_1 = A[65][i];
        temp_2 = A[66][i];

        B[i][64] = temp_0;
        B[i][65] = temp_1;
        B[i][66] = temp_2;
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

    int i, j, i1;

    for (i = 0; i < M; i += 8) {

        for (j = 0; j < N; j += 8) {
            
            /* B * B mini block */
            for(i1=i; i1<i+8; i1++) {

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
 * trans64*64
 */
char trans_desc64[] = "Simple transpose func64";
void trans64(int M, int N, int A[N][M], int B[M][N]){ 

    int i, j, k;
    int temp_0, temp_1, temp_2, temp_3, temp_4, temp_5, temp_6, temp_7;

    for (i = 0; i < N; i += 8) {

        for (j = 0; j < M; j += 8) {


            /* A1, A2 --> B1, B2 */
            for (k = 0; k < 4; k++) {

                temp_0 = A[i + k][j];
                temp_1 = A[i + k][j + 1];
                temp_2 = A[i + k][j + 2];
                temp_3 = A[i + k][j + 3];
                temp_4 = A[i + k][j + 4];
                temp_5 = A[i + k][j + 5];
                temp_6 = A[i + k][j + 6];
                temp_7 = A[i + k][j + 7];

                B[j][i + k] = temp_0;
                B[j + 1][i + k] = temp_1;
                B[j + 2][i + k] = temp_2;
                B[j + 3][i + k] = temp_3;

                B[j][i + k + 4] = temp_4;
                B[j + 1][i + k + 4] = temp_5;
                B[j + 2][i + k + 4] = temp_6;
                B[j + 3][i + k + 4] = temp_7;
            }

            /* B2 --> B3, A3 --> B2 */
            for (k = 0; k < 4; k++) {

                temp_0 = A[i + 4][j + k];
                temp_1 = A[i + 5][j + k];
                temp_2 = A[i + 6][j + k];
                temp_3 = A[i + 7][j + k];

                temp_4 = B[j + k][i + 4];
                temp_5 = B[j + k][i + 5];
                temp_6 = B[j + k][i + 6];
                temp_7 = B[j + k][i + 7];

                B[j + k][i + 4] = temp_0;
                B[j + k][i + 5] = temp_1;
                B[j + k][i + 6] = temp_2;
                B[j + k][i + 7] = temp_3;

                B[j + k + 4][i] = temp_4;
                B[j + k + 4][i + 1] = temp_5;
                B[j + k + 4][i + 2] = temp_6;
                B[j + k + 4][i + 3] = temp_7;

            }

            /* A4 --> B4 */
            for (k = 4; k < 8; k++) {

                temp_0 = A[i + k][j + 4];
                temp_1 = A[i + k][j + 5];
                temp_2 = A[i + k][j + 6];
                temp_3 = A[i + k][j + 7];

                B[j + 4][i + k] = temp_0;
                B[j + 5][i + k] = temp_1;
                B[j + 6][i + k] = temp_2;
                B[j + 7][i + k] = temp_3;
            }
        }
    }

}

/* 
 * trans61*67
 */
char trans_desc6167[] = "Simple transpose func6167";
void trans6167(int M, int N, int A[N][M], int B[M][N]){ 

    int i, j;
    int tmp1, tmp2, tmp3, tmp4, tmp5, tmp6, tmp7, tmp8;

    /* 56 * 67 */
    for (i = 0; i < (M/8)*8; i += 8) {

        for (j = 0; j < N; i++) {

            tmp1 = A[j][i];
            tmp2 = A[j][i + 1];
            tmp3 = A[j][i + 2];
            tmp4 = A[j][i + 3];
            tmp5 = A[j][i + 4];
            tmp6 = A[j][i + 5];
            tmp7 = A[j][i + 6];
            tmp8 = A[j][i + 7];

            B[i][j] = tmp1;
            B[i + 1][j] = tmp2;
            B[i + 2][j] = tmp3;
            B[i + 3][j] = tmp4;
            B[i + 4][j] = tmp5;
            B[i + 5][j] = tmp6;
            B[i + 6][j] = tmp7;
            B[i + 7][j] = tmp8;
        }
    }

    /* transpose rest elements */
    for (i = 0; i < N; i++) {

        for (j = (M/8)*8; j < M; j++) {

            tmp1 = A[i][j];
            B[j][i] = tmp1;
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

    // registerTransFunction(trans32, trans_desc32);

    // registerTransFunction(trans64, trans_desc64);

    // registerTransFunction(trans6167, trans_desc6167);

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

