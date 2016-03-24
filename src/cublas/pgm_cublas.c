/*********************************
 *   Geraldo Zimbrão da Silva    *
 *   Fausto Ferreira Junqueira   *
 *   13/12/2012                  *
 *********************************/

#include "postgres.h"       //standard
#include "fmgr.h"           //standard
#include "utils/memutils.h"//standard
#include "utils/geo_decls.h"//standard
#include "utils/array.h"    //arrays
#include "utils/errcodes.h" //warning
#include "catalog/pg_type.h"// OID
#include "utils/lsyscache.h"// construi array type
#include "funcapi.h"        // trabalhar com row
#include "access/heapam.h"  //Construção de Tupla
#include "access/htup_details.h" //Add heap_form_tuple
#include "../Util/pgm_matriz.h"
#include "../Util/pgm_matriz_util.h"
#include "../Util/pgm_malloc.h"
#include "../pgm_math/pgm_math_util.h"

int cublas_multiply_matrix_double( PGM_Matriz_Double *A, PGM_Matriz_Double *B, PGM_Matriz_Double *result, PGM_Matriz_Double *work);
int cublas_multiply_matrix_float ( PGM_Matriz_Double  *A, PGM_Matriz_Double  *B, PGM_Matriz_Double  *result, PGM_Matriz_Float *work);
int cublas_multiply_matrix_float2( PGM_Matriz_Double  *A, PGM_Matriz_Double  *B, PGM_Matriz_Double  *result, PGM_Matriz_Float *work);
int cublas_multiply_matrix_double2( PGM_Matriz_Double  *A, PGM_Matriz_Double  *B, PGM_Matriz_Double  *result, PGM_Matriz_Double *work);
int cublas_multiply_matrix_double_v2(PGM_Matriz_Double *A, PGM_Matriz_Double* B, PGM_Matriz_Double *C);
int cublas_multiply_matrix_float_v2(PGM_Matriz_Double *A, PGM_Matriz_Double* B, PGM_Matriz_Double *C);
int initializeCUDA();

PG_FUNCTION_INFO_V1(pgm_cublas_dgemm);
Datum pgm_cublas_dgemm(PG_FUNCTION_ARGS);
/*Function: pgm_cublas_dgemm
        Multiplica matriz A por matriz B.
        Onde o numero de colunas de A deve ser igual ao numero de linhas de B
        Em caso de erro, retorno :<b>"Numero de colunas de A diferente de numero de linhas de B"
        Essa função so pode ser invocada se houve uma GPU NVidea e estiver instalado o driver do CUDA.
        Otimização:
            Essa funçao pode ser otimazada de duas formas:
                - Passando true no use_temp_matrix, porem será necessario mais memoria
                - Utilizando a <pgm_cublas_sgemm>, porem perderá precisão nos elementos da matriz. E tambem pode ser mais otimizado passando true e utilizará mais memoria.
	Parameter:
		PGM_Matriz_Double* matrix_a - Matriz A
		PGM_Matriz_Double* matrix_b - Matriz B

	Return:
		PGM_Matriz_Double* - Ponteiro para a matriz resultante
    See also:
        <blas_multiply>, <pgm_cublas_dgemm>, <pgm_cublas_sgemm>
 */
Datum pgm_cublas_dgemm(PG_FUNCTION_ARGS){
	PGM_Matriz_Double *A = (PGM_Matriz_Double*) PG_GETARG_POINTER(0);
	PGM_Matriz_Double *B = (PGM_Matriz_Double*) PG_GETARG_POINTER(1);
    bool use_temp_matrix = PG_GETARG_BOOL(2);
    PGM_Matriz_Double *result;
    PGM_Matriz_Double  *work;
    MemoryContext contextoAnterior;

    if(A->n_colunas == B->n_linhas){
        int info;
        contextoAnterior = MemoryContextSwitchTo( CurTransactionContext );
        result = pgm_create_matrix_double(A->n_linhas, B->n_colunas);
        MemoryContextSwitchTo( contextoAnterior );

        if(use_temp_matrix){
            work = pgm_create_matrix_double(max(A->n_linhas, max(A->n_colunas, B->n_colunas)), max(A->n_linhas, max(A->n_colunas, B->n_colunas)));

            info = cublas_multiply_matrix_double2(A, B, result, work);
            pgm_destroy_matrix_double(work);
        }
        else{
            work = pgm_create_matrix_double(1, max(A->n_linhas, max(A->n_colunas, B->n_colunas)));
            info = cublas_multiply_matrix_double(A, B, result, work);
            pgm_destroy_matrix_double(work);
        }

        if( info < 1 ){
            char error = 0;
            info *= -1;
            switch((info%9)){
                case 0: elog(WARNING,"Matriz result nao foi alocada corretamente! (%d)\n",info);break;
                case 1: elog(WARNING,"CUBLAS initialization error (%d)\n",info);break;
                case 2: elog(WARNING,"Ao alocar memoria no dispositivo, excedeu o limite de memoria da GPU (%d)\n",info);break;
                case 3: elog(WARNING,"No acesso ao dispositivo (%d)\n",info);break;
                case 4: elog(WARNING,"KERNEL DEVICE (%d)\n",info);break;
                case 5: elog(WARNING,"No acesso ao dispositivo (%d)\n",info);break;
                case 6: elog(WARNING,"Ao desalocar memoria do dispositivo (%d)\n",info);break;
                case 7: elog(WARNING,"Ao sair do dispositivo (%d)\n",info);break;
                case 8: elog(WARNING,"Dispositivo nao Encontrado (%d)\n",info);break;
            }
            error = (int) info/9;
            switch(error){
                case 0: elog(ERROR,"Após o erro, a memória foi totalmente liberada\n");break;
                case 1: elog(ERROR,"HINT: O sistema nao conseguiu liberar a memoria, favor reiniciar a aplicação\n");break;
            }
        }
        else{
            PG_RETURN_POINTER(result);
        }
    }
    else{
		elog(ERROR, "Numero de colunas de A(%d) é diferente no numero de linhas de B(%d)\n", A->n_colunas, B->n_linhas);
	}
	PG_RETURN_VOID();
}

PG_FUNCTION_INFO_V1(pgm_cublas_sgemm);
Datum pgm_cublas_sgemm(PG_FUNCTION_ARGS);
/*Function: pgm_cublas_sgemm
        Multiplica matriz A por matriz B.
        Onde o numero de colunas de A deve ser igual ao numero de linhas de B
        Em caso de erro, retorno :<b>"Numero de colunas de A diferente de numero de linhas de B"
        Essa função so pode ser invocada se houve uma GPU NVidea e estiver instalado o driver do CUDA.
        Com o uso dessa função se perde um pouco da precisão.
        Otimização:
            Essa funçao pode ser otimazada de duas formas:
                - Passando true no use_temp_matrix, porem será necessario mais memoria
	Parameter:
		PGM_Matriz_Float* matrix_a - Matriz A
		PGM_Matriz_Float* matrix_b - Matriz B

	Return:
		PGM_Matriz_Double* - Ponteiro para a matriz resultante
    See also:
        <blas_multiply>, <pgm_cublas_dgemm>, <pgm_cublas_sgemm>
 */
Datum pgm_cublas_sgemm(PG_FUNCTION_ARGS){
	PGM_Matriz_Double *A = (PGM_Matriz_Double*) PG_GETARG_POINTER(0);
	PGM_Matriz_Double *B = (PGM_Matriz_Double*) PG_GETARG_POINTER(1);
	bool use_temp_matrix = PG_GETARG_BOOL(2);
	PGM_Matriz_Double *result;
	PGM_Matriz_Float  *work;
    MemoryContext contextoAnterior;
    if(A->n_colunas == B->n_linhas){
        int info;
        contextoAnterior = MemoryContextSwitchTo( CurTransactionContext );
        result = pgm_create_matrix_double(A->n_linhas, B->n_colunas);
        MemoryContextSwitchTo( contextoAnterior );
        if(use_temp_matrix){
            work = pgm_create_matrix_float(max(A->n_linhas, max(A->n_colunas, B->n_colunas)), max(A->n_linhas, max(A->n_colunas, B->n_colunas)));

            info = cublas_multiply_matrix_float2(A, B, result, work);
            pgm_destroy_matrix_float(work);
        }else{
            work = pgm_create_matrix_float(1, max(A->n_linhas, max(A->n_colunas, B->n_colunas)));

            info = cublas_multiply_matrix_float(A, B, result, work);
            pgm_destroy_matrix_float(work);
        }

        if( info < 1 ){
            char error = 0;
            info *= -1;
            switch((info%9)){
                case 0: elog(WARNING,"Matriz result nao foi alocada corretamente! (%d)\n",info);break;
                case 1: elog(WARNING,"CUBLAS initialization error (%d)\n",info);break;
                case 2: elog(WARNING,"Ao alocar memoria no dispositivo, excedeu o limite de memoria da GPU (%d)\n",info);break;
                case 3: elog(WARNING,"No acesso ao dispositivo (%d)\n",info);break;
                case 4: elog(WARNING,"KERNEL DEVICE (%d)\n",info);break;
                case 5: elog(WARNING,"No acesso ao dispositivo (%d)\n",info);break;
                case 6: elog(WARNING,"Ao desalocar memoria do dispositivo (%d)\n",info);break;
                case 7: elog(WARNING,"Ao sair do dispositivo (%d)\n",info);break;
                case 8: elog(WARNING,"Dispositivo nao Encontrado (%d)\n",info);break;
            }
            error = (int) info/9;
            switch(error){
                case 0: elog(ERROR,"Após o erro, a memória foi totalmente liberada\n");break;
                case 1: elog(ERROR,"HINT: O sistema nao conseguiu liberar a memoria, favor reiniciar a aplicação\n");break;
            }
        }else{
            PG_RETURN_POINTER(result);
        }
    }else{
		elog(ERROR, "Numero de colunas de A(%d) é diferente no numero de linhas de B(%d)\n", A->n_colunas, B->n_linhas);
	}
	PG_RETURN_VOID();
}

PG_FUNCTION_INFO_V1(pgm_cublas_dgemm_v2);
Datum pgm_cublas_dgemm_v2(PG_FUNCTION_ARGS);
/*Function: pgm_cublas_dgemm
        Multiplica matriz A por matriz B.
        Onde o numero de colunas de A deve ser igual ao numero de linhas de B
        Em caso de erro, retorno :<b>"Numero de colunas de A diferente de numero de linhas de B"
        Essa função so pode ser invocada se houve uma GPU NVidea e estiver instalado o driver do CUDA.
        Otimização:
            Essa funçao pode ser otimazada de duas formas:
                - Passando true no use_temp_matrix, porem será necessario mais memoria
                - Utilizando a <pgm_cublas_sgemm>, porem perderá precisão nos elementos da matriz. E tambem pode ser mais otimizado passando true e utilizará mais memoria.
	Parameter:
		PGM_Matriz_Double* matrix_a - Matriz A
		PGM_Matriz_Double* matrix_b - Matriz B

	Return:
		PGM_Matriz_Double* - Ponteiro para a matriz resultante
    See also:
        <blas_multiply>, <pgm_cublas_dgemm>, <pgm_cublas_sgemm>
 */
Datum pgm_cublas_dgemm_v2(PG_FUNCTION_ARGS){
	PGM_Matriz_Double *A = (PGM_Matriz_Double*) PG_GETARG_POINTER(0);
	PGM_Matriz_Double *B = (PGM_Matriz_Double*) PG_GETARG_POINTER(1);
    PGM_Matriz_Double *result;
    MemoryContext contextoAnterior;

    if(A->n_colunas == B->n_linhas){
        int info;
        contextoAnterior = MemoryContextSwitchTo( CurTransactionContext );
        result = pgm_create_matrix_double(A->n_linhas, B->n_colunas);
        MemoryContextSwitchTo( contextoAnterior );

        if( initializeCUDA() == -1)
            elog(ERROR,"Ao inicializar o CUDA\n");

        info = cublas_multiply_matrix_double_v2(A, B, result);

        if( info ){
            info *= -1;
            switch((info)){
                case 1: elog(ERROR,"Ao alocar memoria no dispositivo, excedeu o limite de memoria da GPU (%d)\n",info);break;
                case 2: elog(ERROR,"Ao transferir dados do Host para a GPU (%d)\n",info);break;
                case 3: elog(ERROR,"Ao inicializar o ambiente (%d) \n",info);break;
                case 4: elog(ERROR,"Ao executar a multiplicação (%d)\n",info); break;
                case 5: elog(ERROR,"Ao transferir dados da GPU para o Host (%d)\n",info);break;
            }
        }
        else{
            PG_RETURN_POINTER(result);
        }
    }
    else{
		elog(ERROR, "Numero de colunas de A(%d) é diferente no numero de linhas de B(%d)\n", A->n_colunas, B->n_linhas);
	}
	PG_RETURN_VOID();
}

PG_FUNCTION_INFO_V1(pgm_cublas_sgemm_v2);
Datum pgm_cublas_sgemm_v2(PG_FUNCTION_ARGS);
/*Function: pgm_cublas_sgemm
        Multiplica matriz A por matriz B.
        Onde o numero de colunas de A deve ser igual ao numero de linhas de B
        Em caso de erro, retorno :<b>"Numero de colunas de A diferente de numero de linhas de B"
        Essa função so pode ser invocada se houve uma GPU NVidea e estiver instalado o driver do CUDA.
        Com o uso dessa função se perde um pouco da precisão.
        Otimização:
            Essa funçao pode ser otimazada de duas formas:
                - Passando true no use_temp_matrix, porem será necessario mais memoria
	Parameter:
		PGM_Matriz_Double* matrix_a - Matriz A
		PGM_Matriz_Double* matrix_b - Matriz B

	Return:
		PGM_Matriz_Double* - Ponteiro para a matriz resultante
    See also:
        <blas_multiply>, <pgm_cublas_dgemm>, <pgm_cublas_sgemm>
 */
Datum pgm_cublas_sgemm_v2(PG_FUNCTION_ARGS){
	PGM_Matriz_Double *A = (PGM_Matriz_Double*) PG_GETARG_POINTER(0);
	PGM_Matriz_Double *B = (PGM_Matriz_Double*) PG_GETARG_POINTER(1);
    PGM_Matriz_Double *result;
    MemoryContext contextoAnterior;

    if(A->n_colunas == B->n_linhas){
        int info;
        contextoAnterior = MemoryContextSwitchTo( CurTransactionContext );
        result = pgm_create_matrix_double(A->n_linhas, B->n_colunas);
        MemoryContextSwitchTo( contextoAnterior );

        if( initializeCUDA() == -1)
            elog(ERROR,"Ao inicializar o CUDA\n");

        info = cublas_multiply_matrix_float_v2(A, B, result);

        if( info ){
            info *= -1;
            switch((info)){
                case 1: elog(ERROR,"Ao alocar memoria no dispositivo, excedeu o limite de memoria da GPU (%d)\n",info);break;
                case 2: elog(ERROR,"Ao transferir dados do Host para a GPU (%d)\n",info);break;
                case 3: elog(ERROR,"Ao inicializar o ambiente (%d) \n",info);break;
                case 4: elog(ERROR,"Ao executar a multiplicação (%d)\n",info); break;
                case 5: elog(ERROR,"Ao transferir dados da GPU para o Host (%d)\n",info);break;
                case 6: elog(ERROR,"Ao alocar memoria para vetor temporario (%d)\n",info);break;
            }
        }
        else{
            PG_RETURN_POINTER(result);
        }
    }
    else{
		elog(ERROR, "Numero de colunas de A(%d) é diferente no numero de linhas de B(%d)\n", A->n_colunas, B->n_linhas);
	}
	PG_RETURN_VOID();
}
