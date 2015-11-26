#include<stdio.h>
#include <string.h>
#include "../Util/pgm_matriz.h"
#include <cuda_runtime.h>
#include <cublas_v2.h>

#include "pure_c_cublas_v2.h"


extern "C"{
    int cublas_multiply_matrix_float_v2(PGM_Matriz_Double *A, PGM_Matriz_Double* B, PGM_Matriz_Double *C);
	int cublas_multiply_matrix_double_v2(PGM_Matriz_Double *A, PGM_Matriz_Double* B, PGM_Matriz_Double *C);
    int initializeCUDA();
    void* pgm_malloc( size_t size );
    void  pgm_free( void * ptr );
};
//Inicializa o CUDA
int initializeCUDA(){
	int devID = 0;
	cudaError_t error;

	// get number of SMs on this GPU
    error = cudaGetDevice(&devID);

    if (error != cudaSuccess){
        return -1;
    }
    return devID;

}

//As 3 Matrizes jah devem Vir alocadas
int cublas_multiply_matrix_double_v2(PGM_Matriz_Double *A, PGM_Matriz_Double* B, PGM_Matriz_Double *C){

    unsigned int size_A = A->n_linhas * A->n_colunas,
    			 mem_size_A = sizeof(double) * size_A,
    			 size_B = B->n_linhas * B->n_colunas,
    			 mem_size_B = sizeof(double) * size_B,
    			 size_C = C->n_linhas * C->n_colunas,
    			 mem_size_C = sizeof(double) * size_C;

    // allocate device memory
    double *d_A, *d_B, *d_C;


    if (cudaMalloc((void **) &d_A, mem_size_A) != cudaSuccess)
        return -1;

    if (cudaMalloc((void **) &d_B, mem_size_B) != cudaSuccess){
        cudaFree(d_A);
        return -1;
    }

    if (cudaMalloc((void **) &d_C, mem_size_C) != cudaSuccess){
        cudaFree(d_A);
        cudaFree(d_B);
        return -1;
    }

    // copy host memory to device

    if (cudaMemcpy(d_A, A->valor, mem_size_A, cudaMemcpyHostToDevice) != cudaSuccess){
        cudaFree(d_A);
        cudaFree(d_B);
        cudaFree(d_C);
        return -2;
    }

    if (cudaMemcpy(d_B, B->valor, mem_size_B, cudaMemcpyHostToDevice) != cudaSuccess){
        cudaFree(d_A);
        cudaFree(d_B);
        cudaFree(d_C);
        return -2;
    }

    {
        cublasHandle_t handle;
        const double alpha = 1.0;
        const double beta  = 0.0;


        if (cublasCreate(&handle) != CUBLAS_STATUS_SUCCESS){
            cudaFree(d_A);
            cudaFree(d_B);
            cudaFree(d_C);
            return -3;
        }
        if (cublasDgemm(handle, CUBLAS_OP_N, CUBLAS_OP_N, B->n_colunas, A->n_linhas,B->n_linhas, &alpha, d_B, B->n_colunas, d_A, A->n_colunas, &beta, d_C, C->n_colunas) != CUBLAS_STATUS_SUCCESS){
            cudaFree(d_A);
            cudaFree(d_B);
            cudaFree(d_C);
            return -4;
        }

        if (cudaMemcpy(C->valor,d_C, mem_size_C, cudaMemcpyDeviceToHost) != cudaSuccess){
            cudaFree(d_A);
            cudaFree(d_B);
            cudaFree(d_C);
            return -5;
        }

    }

    cudaFree(d_A);
    cudaFree(d_B);
    cudaFree(d_C);

    return 0;
}

int cublas_multiply_matrix_float_v2(PGM_Matriz_Double *A, PGM_Matriz_Double* B, PGM_Matriz_Double *C){

    unsigned int i;
    unsigned int size_A = A->n_linhas * A->n_colunas,
    			 mem_size_A = sizeof(float) * size_A,
    			 size_B = B->n_linhas * B->n_colunas,
    			 mem_size_B = sizeof(float) * size_B,
    			 size_C = C->n_linhas * C->n_colunas,
    			 mem_size_C = sizeof(float) * size_C;

    // allocate device memory
    float *d_A, *d_B, *d_C;
    float *host_temp;

    if (cudaMalloc((void **) &d_A, mem_size_A) != cudaSuccess)
        return -1;

    if (cudaMalloc((void **) &d_B, mem_size_B) != cudaSuccess){
        cudaFree(d_A);
        return -1;
    }

    if (cudaMalloc((void **) &d_C, mem_size_C) != cudaSuccess){
        cudaFree(d_A);
        cudaFree(d_B);
        return -1;
    }

    // copy host memory to device
    if( (host_temp = (float*) pgm_malloc (mem_size_A)) == NULL)
        return -6;

    for(i = 0; i < size_A; i++)
        host_temp[i] = (float) A->valor[i];



    if (cudaMemcpy(d_A, host_temp, mem_size_A, cudaMemcpyHostToDevice) != cudaSuccess){
        cudaFree(d_A);
        cudaFree(d_B);
        cudaFree(d_C);
        return -2;
    }
    pgm_free(host_temp);
    if( (host_temp = (float*) pgm_malloc (mem_size_B)) == NULL)
        return -6;
    for(i = 0; i < size_B; i++)
        host_temp[i] = (float) B->valor[i];

    if (cudaMemcpy(d_B, host_temp, mem_size_B, cudaMemcpyHostToDevice) != cudaSuccess){
        cudaFree(d_A);
        cudaFree(d_B);
        cudaFree(d_C);
        return -2;
    }

    pgm_free(host_temp);
    if( (host_temp = (float*) pgm_malloc (mem_size_C)) == NULL)
        return -6;

    {
        cublasHandle_t handle;
        const float alpha = 1.0;
        const float beta  = 0.0;


        if (cublasCreate(&handle) != CUBLAS_STATUS_SUCCESS){
            cudaFree(d_A);
            cudaFree(d_B);
            cudaFree(d_C);
            return -3;
        }
        if (cublasSgemm(handle, CUBLAS_OP_N, CUBLAS_OP_N, B->n_colunas, A->n_linhas,B->n_linhas, &alpha, d_B, B->n_colunas, d_A, A->n_colunas, &beta, d_C, C->n_colunas) != CUBLAS_STATUS_SUCCESS){
            cudaFree(d_A);
            cudaFree(d_B);
            cudaFree(d_C);
            return -4;
        }

        if (cudaMemcpy(host_temp,d_C, mem_size_C, cudaMemcpyDeviceToHost) != cudaSuccess){
            cudaFree(d_A);
            cudaFree(d_B);
            cudaFree(d_C);
            return -5;
        }
    }
    for(i = 0; i < size_C; i++)
        C->valor[i] = (double) host_temp[i];
    cudaFree(d_A);
    cudaFree(d_B);
    cudaFree(d_C);

    return 0;
}

