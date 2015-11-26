#include<stdio.h>
#include <string.h>
#include "../Util/pgm_matriz.h"
#include <cuda_runtime.h>
#include <cublas_v2.h>
#include "pure_c_cublas.h"

extern "C" {

int cublas_multiply_matrix_double( PGM_Matriz_Double *A, PGM_Matriz_Double *B, PGM_Matriz_Double *result, PGM_Matriz_Double *work);
int cublas_multiply_matrix_float ( PGM_Matriz_Double  *A, PGM_Matriz_Double  *B, PGM_Matriz_Double  *result, PGM_Matriz_Float *work);
int cublas_multiply_matrix_float2( PGM_Matriz_Double  *A, PGM_Matriz_Double  *B, PGM_Matriz_Double  *result, PGM_Matriz_Float *work);
int cublas_multiply_matrix_double2( PGM_Matriz_Double  *A, PGM_Matriz_Double  *B, PGM_Matriz_Double  *result, PGM_Matriz_Double *work);

};

//Se as funções abaixo retornarem -1, assume-se que nao foi possivel completar a copia
int double_copyHost2GPU(PGM_Matriz_Double *host, double *device, int device_col, PGM_Matriz_Double *work ){
	int i, j;
	double *ptr;
	for(i = 0; i < host->n_linhas; i++){
		ptr = host->valor+(i*host->n_colunas);

		for( j = 0; j < host->n_colunas; j++)
			work->valor[j] = ptr[j];

        for(; j < work->n_colunas; j++)
            work->valor[j] = 0;

		if(cublasSetVector(work->n_colunas, sizeof(double), work->valor, 1, device+(device_col*i), 1) != CUBLAS_STATUS_SUCCESS){
			printf("Error: nao foi possivel executar a copia\n");
			return -1;
		}
	}

	for ( i = 0; i < work->n_colunas; i++) work->valor[i] = 0;

	for ( ; i < work->n_colunas; j++){
        if(cublasSetVector(work->n_colunas, sizeof(double), work->valor, 1, device+(device_col*i), 1) != CUBLAS_STATUS_SUCCESS){
			printf("Error: nao foi possivel executar a copia\n");
			return -1;
		}
	}
	return CUBLAS_STATUS_SUCCESS;
}

int double_copyGPU2Host(PGM_Matriz_Double *host, double *device, int device_col, PGM_Matriz_Double *work){
	int i,j;
	double *ptr;

	for( i = 0; i < host->n_linhas; i++ ){
		ptr = host->valor+(i*host->n_colunas);

		if(cublasGetVector(host->n_colunas,sizeof(double), device+(device_col*i), 1 ,work->valor, 1) != CUBLAS_STATUS_SUCCESS){
			printf("Error: nao foi possivel executar a copia!\n");
			return -1;
		}

		for( j = 0; j < host->n_colunas; j++)
			ptr[j] = work->valor[j];
	}
	return CUBLAS_STATUS_SUCCESS;
}

int double_copyGPU2Host_Transpose(PGM_Matriz_Double *host, double *device, int device_col, PGM_Matriz_Double *work){
	int i,j;
	double *ptr;

	for( i = 0; i < host->n_linhas; i++ ){

		ptr = host->valor+(i*host->n_colunas);

		if(cublasGetVector(host->n_colunas,sizeof(double), device+i, device_col ,work->valor, 1) != CUBLAS_STATUS_SUCCESS){
			printf("Error: nao foi possivel executar a copia!\n");
			return -1;
		}
		for( j = 0; j < host->n_colunas; j++)
			ptr[j] =  work->valor[j];
	}
	return CUBLAS_STATUS_SUCCESS;
}

int float_copyHost2GPU(PGM_Matriz_Double *host,  float *device, int device_col, PGM_Matriz_Float *work){

	int i, j;
	double *ptr;
	for(i = 0; i < host->n_linhas; i++){
		ptr = host->valor+(i*host->n_colunas);

		for( j = 0; j < host->n_colunas; j++)
			work->valor[j] = (float) ptr[j];

        for(; j < work->n_colunas; j++)
            work->valor[j] = 0;

		if(cublasSetVector(work->n_colunas, sizeof(float), work->valor, 1, device+(device_col*i), 1) != CUBLAS_STATUS_SUCCESS){
			printf("Error: nao foi possivel executar a copia\n");
			return -1;
		}
	}

	for ( i = 0; i < work->n_colunas; i++) work->valor[i] = 0;

	for ( ; i < work->n_colunas; j++){
        if(cublasSetVector(work->n_colunas, sizeof(float), work->valor, 1, device+(device_col*i), 1) != CUBLAS_STATUS_SUCCESS){
			printf("Error: nao foi possivel executar a copia\n");
			return -1;
		}
	}
	return CUBLAS_STATUS_SUCCESS;
}

int float_copyGPU2Host(PGM_Matriz_Double *host, float *device, int device_col, PGM_Matriz_Float *work){
	int i,j;
	double *ptr;

	for( i = 0; i < host->n_linhas; i++ ){
		ptr = host->valor+(i*host->n_colunas);

		if(cublasGetVector(host->n_colunas,sizeof(float), device+(device_col*i), 1 ,work->valor, 1) != CUBLAS_STATUS_SUCCESS){
			printf("Error: nao foi possivel executar a copia!\n");
			return -1;
		}

		for( j = 0; j < host->n_colunas; j++)
			ptr[j] = (double) work->valor[j];
	}
	return CUBLAS_STATUS_SUCCESS;
}

int float_copyGPU2Host_Transpose(PGM_Matriz_Double *host, float *device, int device_col, PGM_Matriz_Float *work){
	int i,j;
	double *ptr;

	for( i = 0; i < host->n_linhas; i++ ){

		ptr = host->valor+(i*host->n_colunas);

		if(cublasGetVector(host->n_colunas,sizeof(float), device+i, device_col ,work->valor, 1) != CUBLAS_STATUS_SUCCESS){
			printf("Error: nao foi possivel executar a copia!\n");
			return -1;
		}
		for( j = 0; j < host->n_colunas; j++)
			ptr[j] = (double) work->valor[j];
	}
	return CUBLAS_STATUS_SUCCESS;
}

int float_copyMatrixGPU2Host_Transpose(PGM_Matriz_Double *host, PGM_Matriz_GPU* device, PGM_Matriz_Float *work){
    if(work->n_colunas == device->max_dim && work->n_linhas == device->max_dim){
        if(cublasGetMatrix(device->max_dim,device->max_dim,sizeof(float),device->valor, device->max_dim,work->valor,work->n_colunas) != CUBLAS_STATUS_SUCCESS){
            return CUBLAS_STATUS_INVALID_VALUE;
        }else{
            int i,j;
            for( i = 0; i < host->n_linhas; i++)
                for(j = 0; j < host->n_colunas; j++)
                    PGM_ELEM_MATRIZ(host, i,j) = (double) PGM_ELEM_MATRIZ(work,j,i);
            return CUBLAS_STATUS_SUCCESS;
        }
    }else{
        return CUBLAS_STATUS_INVALID_VALUE;
    }
}

int float_copyMatrixHost2GPU(PGM_Matriz_Double *host, PGM_Matriz_Float *work, PGM_Matriz_GPU* device){
    if(work->n_colunas == device->max_dim && work->n_linhas == device->max_dim){
        int i,j = 0;
        for(i = 0; i < host->n_colunas; i++){
            for( j = 0; j < host->n_linhas; j++){
                PGM_ELEM_MATRIZ(work,j,i) =(float) PGM_ELEM_MATRIZ(host,j,i);
            }
        }
        for ( ; i < work->n_colunas; i++)
            for ( ;j < work->n_linhas; j++)
                PGM_ELEM_MATRIZ(work,j, i) = 0;

        return cublasSetMatrix(device->max_dim,device->max_dim,sizeof(float),work->valor,work->n_colunas, device->valor, device->max_dim);

    }else{
        return CUBLAS_STATUS_INVALID_VALUE;
    }
}

int create_PGM_Matriz_GPU_float(PGM_Matriz_GPU *device, int n_linhas, int n_colunas, int max_dim){
    device->n_colunas = n_colunas;
    device->n_linhas = n_linhas;
    device->max_dim = max_dim;
    return cudaMalloc((void **) &device->valor, sizeof(float) *device->max_dim*device->max_dim);
}

int double_copyMatrixGPU2Host_Transpose(PGM_Matriz_Double *host, PGM_Matriz_GPU* device, PGM_Matriz_Double *work){
    if(work->n_colunas == device->max_dim && work->n_linhas == device->max_dim){
        if(cublasGetMatrix(device->max_dim,device->max_dim,sizeof(double),device->valor, device->max_dim,work->valor,work->n_colunas) != CUBLAS_STATUS_SUCCESS){
            return CUBLAS_STATUS_INVALID_VALUE;
        }else{
            int i,j;
            for( i = 0; i < host->n_linhas; i++)
                for(j = 0; j < host->n_colunas; j++)
                    PGM_ELEM_MATRIZ(host, i,j) = PGM_ELEM_MATRIZ(work,j,i);
            return CUBLAS_STATUS_SUCCESS;
        }
    }else{
        return CUBLAS_STATUS_INVALID_VALUE;
    }
}

int double_copyMatrixHost2GPU(PGM_Matriz_Double *host, PGM_Matriz_Double *work, PGM_Matriz_GPU* device){
    if(work->n_colunas == device->max_dim && work->n_linhas == device->max_dim){
        int i,j = 0;
        for(i = 0; i < host->n_colunas; i++){
            for( j = 0; j < host->n_linhas; j++){
                PGM_ELEM_MATRIZ(work,j,i) = PGM_ELEM_MATRIZ(host,j,i);
            }
        }
        for ( ; i < work->n_colunas; i++)
            for ( ;j < work->n_linhas; j++)
                PGM_ELEM_MATRIZ(work,j, i) = 0;

        return cublasSetMatrix(device->max_dim,device->max_dim,sizeof(double),work->valor,work->n_colunas, device->valor, device->max_dim);

    }else{
        return CUBLAS_STATUS_INVALID_VALUE;
    }
}

int create_PGM_Matriz_GPU_double(PGM_Matriz_GPU *device, int n_linhas, int n_colunas, int max_dim){
    device->n_colunas = n_colunas;
    device->n_linhas = n_linhas;
    device->max_dim = max_dim;
    return cudaMalloc((void **) &device->valor, sizeof(double) *device->max_dim*device->max_dim);
}

int cublas_multiply_matrix_float ( PGM_Matriz_Double  *A, PGM_Matriz_Double  *B, PGM_Matriz_Double  *result, PGM_Matriz_Float *work){
    cublasStatus_t status;
    cublasHandle_t handle;
    int max_dim = A->n_linhas >= (A->n_colunas >= B->n_colunas ? A->n_colunas : B->n_colunas) ? A ->n_linhas : (A->n_colunas >= B->n_colunas ? A->n_colunas : B->n_colunas);
    float *device_A,
           *device_B,
           *device_C,
            alpha = 1.0,
            beta = 0.0;

    if (A->n_linhas != result->n_linhas || B->n_colunas != result->n_colunas){
        return 0;
    }

    status = cublasCreate(&handle);

    if (status != CUBLAS_STATUS_SUCCESS){
        return -1;
    }

    if(cudaMalloc((void **) &device_A, sizeof(float) *max_dim*max_dim) != cudaSuccess){
        return -2;
    }
    if(cudaMalloc((void**) &device_B, sizeof(float) *max_dim*max_dim) != cudaSuccess){
        if(cudaFree(device_A) == cudaSuccess) return -2;
        else return -11;
    }
    if(cudaMalloc((void**) &device_C, sizeof(float) *max_dim*max_dim) != cudaSuccess){
        if(cudaFree(device_A) == cudaSuccess && cudaFree(device_B) == cudaSuccess) return -2;
        else return -11;
    }

    if(float_copyHost2GPU(A,device_A,max_dim,work) != CUBLAS_STATUS_SUCCESS){
        if(cudaFree(device_A) == cudaSuccess && cudaFree(device_B) == cudaSuccess && cudaFree(device_C) == cudaSuccess) return -3;
        else return -12;
    }

    if(float_copyHost2GPU(B,device_B,max_dim,work) != CUBLAS_STATUS_SUCCESS){
        if(cudaFree(device_A) == cudaSuccess && cudaFree(device_B) == cudaSuccess && cudaFree(device_C) == cudaSuccess) return -3;
        else return -12;
    }
    if(cublasSgemm(handle, CUBLAS_OP_T, CUBLAS_OP_T, max_dim,max_dim,max_dim, &alpha, device_A,max_dim,device_B, max_dim, &beta, device_C, max_dim) != CUBLAS_STATUS_SUCCESS){
        if(cudaFree(device_A) == cudaSuccess && cudaFree(device_B) == cudaSuccess && cudaFree(device_C) == cudaSuccess) return -4;
        else return -13;
    }
    if(float_copyGPU2Host_Transpose(result, device_C, max_dim, work) != CUBLAS_STATUS_SUCCESS){
        if(cudaFree(device_A) == cudaSuccess && cudaFree(device_B) == cudaSuccess && cudaFree(device_C) == cudaSuccess) return -5;
        else return -14;
    }
    if(cudaFree(device_A) != cudaSuccess){
        return -15;
    }
    if(cudaFree(device_B) != cudaSuccess){
        return -15;
    }
    if(cudaFree(device_C) != cudaSuccess){
        return -15;
    }
    if(cublasDestroy(handle) != CUBLAS_STATUS_SUCCESS){
        return -7;
    }

    return 1;
}

int cublas_multiply_matrix_double( PGM_Matriz_Double *A, PGM_Matriz_Double *B, PGM_Matriz_Double *result, PGM_Matriz_Double *work){
    cublasStatus_t status;
    cublasHandle_t handle;
    int max_dim = A->n_linhas >= (A->n_colunas >= B->n_colunas ? A->n_colunas : B->n_colunas) ? A ->n_linhas : (A->n_colunas >= B->n_colunas ? A->n_colunas : B->n_colunas);
    double *device_A,
           *device_B,
           *device_C,
            alpha = 1.0,
            beta = 0.0;

    if (A->n_linhas != result->n_linhas || B->n_colunas != result->n_colunas){
        return 0;
    }

    status = cublasCreate(&handle);


    if (status != CUBLAS_STATUS_SUCCESS){
        return -1;
    }

    if(cudaMalloc((void**) &device_A, sizeof(double) *max_dim*max_dim) != cudaSuccess){
        return -2;
    }
    if(cudaMalloc((void**) &device_B, sizeof(double) *max_dim*max_dim) != cudaSuccess){
        if(cudaFree(device_A) == cudaSuccess) return -2;
        else return -11;
    }
    if(cudaMalloc((void**) &device_C, sizeof(double) *max_dim*max_dim) != cudaSuccess){
        if(cudaFree(device_A) == cudaSuccess && cudaFree(device_B) == cudaSuccess) return -2;
        else return -11;
    }

    if(double_copyHost2GPU(A,device_A,max_dim,work) != CUBLAS_STATUS_SUCCESS){
        if(cudaFree(device_A) == cudaSuccess && cudaFree(device_B) == cudaSuccess && cudaFree(device_C) == cudaSuccess) return -3;
        else return -12;
    }

    if(double_copyHost2GPU(B,device_B,max_dim,work) != CUBLAS_STATUS_SUCCESS){
        if(cudaFree(device_A) == cudaSuccess && cudaFree(device_B) == cudaSuccess && cudaFree(device_C) == cudaSuccess) return -3;
        else return -12;
    }
    if(cublasDgemm(handle, CUBLAS_OP_T, CUBLAS_OP_T, max_dim,max_dim,max_dim, &alpha, device_A,max_dim,device_B, max_dim, &beta, device_C, max_dim) != CUBLAS_STATUS_SUCCESS){
        if(cudaFree(device_A) == cudaSuccess && cudaFree(device_B) == cudaSuccess && cudaFree(device_C) == cudaSuccess) return -4;
        else return -13;
    }
    if(double_copyGPU2Host_Transpose(result, device_C, max_dim, work) != CUBLAS_STATUS_SUCCESS){
        if(cudaFree(device_A) == cudaSuccess && cudaFree(device_B) == cudaSuccess && cudaFree(device_C) == cudaSuccess) return -5;
        else return -14;
    }

    if(cudaFree(device_A) != cudaSuccess)
        return -15;
    if(cudaFree(device_B) != cudaSuccess)
        return -15;
    if(cudaFree(device_C) != cudaSuccess)
        return -15;
    if(cublasDestroy(handle) != CUBLAS_STATUS_SUCCESS)
        return -7;

    return 1;
}

int cublas_multiply_matrix_float2( PGM_Matriz_Double  *A, PGM_Matriz_Double  *B, PGM_Matriz_Double  *result, PGM_Matriz_Float *work){
    cublasStatus_t status;
    cublasHandle_t handle;
    int max_dim = work->n_linhas;
    PGM_Matriz_GPU device_A,
                   device_B,
                   device_C;
    float alpha = 1.0,
          beta = 0.0;

    if (A->n_linhas != result->n_linhas || B->n_colunas != result->n_colunas){
        return 0;
    }

    status = cublasCreate(&handle);

    if (status != CUBLAS_STATUS_SUCCESS){
        return -1;
    }

    if(create_PGM_Matriz_GPU_float(&device_A,A->n_linhas, A->n_colunas,max_dim) != cudaSuccess){
        return -2;
    }
    if(create_PGM_Matriz_GPU_float(&device_B,B->n_linhas, B->n_colunas,max_dim) != cudaSuccess){
        if(cudaFree(device_A.valor) == cudaSuccess) return -2;
        else return -11;
    }
    if(create_PGM_Matriz_GPU_float(&device_C,result->n_linhas, result->n_colunas,max_dim) != cudaSuccess){
        if(cudaFree(device_A.valor) == cudaSuccess && cudaFree(device_B.valor) == cudaSuccess) return -2;
        else return -11;
    }

    if(float_copyMatrixHost2GPU(A,work,&device_A) != CUBLAS_STATUS_SUCCESS){
        if(cudaFree(device_A.valor) == cudaSuccess && cudaFree(device_B.valor) == cudaSuccess && cudaFree(device_C.valor) == cudaSuccess) return -3;
        else return -12;
    }

    if(float_copyMatrixHost2GPU(B,work,&device_B) != CUBLAS_STATUS_SUCCESS){
        if(cudaFree(device_A.valor) == cudaSuccess && cudaFree(device_B.valor) == cudaSuccess && cudaFree(device_C.valor) == cudaSuccess) return -3;
        else return -12;
    }
    if(cublasSgemm(handle, CUBLAS_OP_T, CUBLAS_OP_T, max_dim,max_dim,max_dim, &alpha,(float*) device_A.valor,max_dim,(float*)device_B.valor, max_dim, &beta, (float*)device_C.valor, max_dim) != CUBLAS_STATUS_SUCCESS){
        if(cudaFree(device_A.valor) == cudaSuccess && cudaFree(device_B.valor) == cudaSuccess && cudaFree(device_C.valor) == cudaSuccess) return -4;
        else return -13;
    }
    if(float_copyMatrixGPU2Host_Transpose(result, &device_C, work) != CUBLAS_STATUS_SUCCESS){
        if(cudaFree(device_A.valor) == cudaSuccess && cudaFree(device_B.valor) == cudaSuccess && cudaFree(device_C.valor) == cudaSuccess) return -5;
        else return -14;
    }

    if(cudaFree(device_A.valor) != cudaSuccess){
        return -15;
    }
    if(cudaFree(device_B.valor) != cudaSuccess){
        return -15;
    }
    if(cudaFree(device_C.valor) != cudaSuccess){
        return -15;
    }
    if(cublasDestroy(handle) != CUBLAS_STATUS_SUCCESS){
        return -7;
    }
    return 1;
}

int cublas_multiply_matrix_double2( PGM_Matriz_Double  *A, PGM_Matriz_Double  *B, PGM_Matriz_Double  *result, PGM_Matriz_Double *work){
    cublasStatus_t status;
    cublasHandle_t handle;
    int max_dim = work->n_linhas;
    PGM_Matriz_GPU device_A,
                   device_B,
                   device_C;
    double alpha = 1.0,
           beta = 0.0;

    if (A->n_linhas != result->n_linhas || B->n_colunas != result->n_colunas){
        return 0;
    }

    status = cublasCreate(&handle);

    if (status != CUBLAS_STATUS_SUCCESS){
        return -1;
    }

    if(create_PGM_Matriz_GPU_double(&device_A,A->n_linhas, A->n_colunas,max_dim) != cudaSuccess){
        return -2;
    }
    if(create_PGM_Matriz_GPU_double(&device_B,B->n_linhas, B->n_colunas,max_dim) != cudaSuccess){
        if(cudaFree(device_A.valor) == cudaSuccess) return -2;
        else return -11;
    }
    if(create_PGM_Matriz_GPU_double(&device_C,result->n_linhas, result->n_colunas,max_dim) != cudaSuccess){
        if(cudaFree(device_A.valor) == cudaSuccess && cudaFree(device_B.valor) == cudaSuccess) return -2;
        else return -11;
    }

    if(double_copyMatrixHost2GPU(A,work,&device_A) != CUBLAS_STATUS_SUCCESS){
        if(cudaFree(device_A.valor) == cudaSuccess && cudaFree(device_B.valor) == cudaSuccess && cudaFree(device_C.valor) == cudaSuccess) return -3;
        else return -12;
    }

    if(double_copyMatrixHost2GPU(B,work,&device_B) != CUBLAS_STATUS_SUCCESS){
        if(cudaFree(device_A.valor) == cudaSuccess && cudaFree(device_B.valor) == cudaSuccess && cudaFree(device_C.valor) == cudaSuccess) return -3;
        else return -12;
    }
    if(cublasDgemm(handle, CUBLAS_OP_T, CUBLAS_OP_T, max_dim,max_dim,max_dim, &alpha,(double*) device_A.valor,max_dim,(double*)device_B.valor, max_dim, &beta, (double*)device_C.valor, max_dim) != CUBLAS_STATUS_SUCCESS){
        if(cudaFree(device_A.valor) == cudaSuccess && cudaFree(device_B.valor) == cudaSuccess && cudaFree(device_C.valor) == cudaSuccess) return -4;
        else return -13;
    }
    if(double_copyMatrixGPU2Host_Transpose(result, &device_C, work) != CUBLAS_STATUS_SUCCESS){
        if(cudaFree(device_A.valor) == cudaSuccess && cudaFree(device_B.valor) == cudaSuccess && cudaFree(device_C.valor) == cudaSuccess) return -5;
        else return -14;
    }

    if(cudaFree(device_A.valor) != cudaSuccess){
        return -15;
    }
    if(cudaFree(device_B.valor) != cudaSuccess){
        return -15;
    }
    if(cudaFree(device_C.valor) != cudaSuccess){
        return -15;
    }
    if(cublasDestroy(handle) != CUBLAS_STATUS_SUCCESS){
        return -7;
    }
    return 1;
}

