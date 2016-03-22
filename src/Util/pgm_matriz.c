/*
 * Autor: GZS, FFJ
 * GCCBD - COOPE - UFRJ
 * Criacao: 12/01/2012
 */
#include "postgres.h"       //standard
#include "fmgr.h"           //standard
#include "utils/memutils.h"//standard
#include "utils/geo_decls.h"//standard
#include "utils/array.h"    //arrays
#include "utils/errcodes.h" //warning
#include <cblas.h>
#include<time.h>

#include "pgm_matriz.h"
#include "pgm_matriz_util.h"

PG_FUNCTION_INFO_V1(pgm_matrix_int2arraytype);
Datum pgm_matrix_int2arraytype(PG_FUNCTION_ARGS);

Datum pgm_matrix_int2arraytype(PG_FUNCTION_ARGS){
	PGM_Matriz_Int* matrix = (PGM_Matriz_Int*) PG_GETARG_POINTER(0);
	PG_RETURN_ARRAYTYPE_P(pgm_PGM_Matriz_Int2ArrayType(matrix));
}

PG_FUNCTION_INFO_V1(pgm_arraytype2Matrix_Int);
Datum pgm_arraytype2Matrix_Int(PG_FUNCTION_ARGS);

Datum pgm_arraytype2Matrix_Int(PG_FUNCTION_ARGS){
	PGM_Matriz_Int* m =  pgm_ArrayType2PGM_Matriz_Int(PG_GETARG_ARRAYTYPE_P(0));
	PG_RETURN_POINTER(m);
}

PG_FUNCTION_INFO_V1(pgm_vector_matrix_multiply);
/*Function: pgm_vector_matrix_multiply
        Multiplica vetor A por matriz B.
        Onde o numero de elementos de A deve ser igual ao numero de linhas de B
        Em caso de erro, retorno zero!
	Parameter:
		PGM_Vetor_Double* vector_a - Vetor A n_elem >= 1
		PGM_Matriz_Double* matrix_b - Matriz B n_linhas == n_elem

	Return:
		PGM_Matriz_Double* - Ponteiro para a matriz resultante
 */
Datum pgm_vector_matrix_multiply(PG_FUNCTION_ARGS);

Datum pgm_vector_matrix_multiply(PG_FUNCTION_ARGS){
    PGM_Vetor_Double* vector_a = (PGM_Vetor_Double*) PG_GETARG_POINTER(0);
    PGM_Matriz_Double* matrix_b = (PGM_Matriz_Double*) PG_GETARG_POINTER(1);

    if( vector_a->n_elems == matrix_b->n_linhas ) {
        MemoryContext contextoAnterior = MemoryContextSwitchTo( CurTransactionContext );
        PGM_Vetor_Double* multiply = pgm_create_PGM_Vetor_Double( matrix_b->n_colunas);
        int j, k;

        MemoryContextSwitchTo( contextoAnterior );

        for( k = 0; k < vector_a->n_elems; k++ )
          for( j = 0; j < multiply->n_elems; j++ )
            PGM_ELEM_VETOR(multiply, j) += (PGM_ELEM_VETOR(vector_a, k)) * (PGM_ELEM_MATRIZ(matrix_b, k, j));

        PG_RETURN_POINTER(multiply);
    }else{
        elog(ERROR, "Numero de colunas de A diferente do numero de linhas de B: ( %d ) x ( %d, %d )", vector_a->n_elems, matrix_b->n_linhas, matrix_b->n_colunas );
        PG_RETURN_INT64(0);
    }
}


PG_FUNCTION_INFO_V1(pgm_matrix_apply_sigmoid);
/*Function: pgm_matrix_apply_sigmoid
        Aplica a função sigmoid na matriz.
        Função sigmoid -> 1/(1+exp(-2*x), onde x = elemento da matriz
	Parameter:
		PGM_Matriz_Double* matrix - Matriz onde sera aplicada a função sigmoid

	Return:
		PGM_Matriz_Double* - Ponteiro para a matriz resultante
 */
Datum pgm_matrix_apply_sigmoid(PG_FUNCTION_ARGS);

Datum pgm_matrix_apply_sigmoid(PG_FUNCTION_ARGS){
    PGM_Matriz_Double* matrix = (PGM_Matriz_Double*) PG_GETARG_POINTER(0);
    int i, j;

    for (i=0; i < matrix->n_linhas; i++)
        for(j=0; j < matrix->n_colunas; j++)
            PGM_ELEM_MATRIZ( matrix, i, j ) = 2.0/(1.0 + exp( -16.0*PGM_ELEM_MATRIZ( matrix, i, j ) ) ) - 1.0;

    PG_RETURN_POINTER(matrix);
}


PG_FUNCTION_INFO_V1(pgm_matrix_apply_inverse_sigmoid);
/*Function: pgm_matrix_apply_inverse_sigmoid
        VER COM O PROFESSOR!
	Parameter:
		PGM_Matriz_Double* matrix - Matriz onde sera aplicada a função sigmoid

	Return:
		PGM_Matriz_Double* - Ponteiro para a matriz resultante
 */
Datum pgm_matrix_apply_inverse_sigmoid(PG_FUNCTION_ARGS);

Datum pgm_matrix_apply_inverse_sigmoid(PG_FUNCTION_ARGS){
    PGM_Matriz_Double* matrix = (PGM_Matriz_Double*) PG_GETARG_POINTER(0);
    int i, j;

    for (i=0; i < matrix->n_linhas; i++)
        for(j=0; j < matrix->n_colunas; j++) {
            double x = PGM_ELEM_MATRIZ( matrix, i, j );

            if( x < 0.000000000001 ) x = 0.000000000001;
            else if( x > 0.999999999999 ) x = 0.999999999999;

            PGM_ELEM_MATRIZ( matrix, i, j ) = - log( 2.0/(x+1.0) -1.0)/16.0;
        }

    PG_RETURN_POINTER(matrix);
}

PG_FUNCTION_INFO_V1(pgm_matrix_transpose);
Datum pgm_matrix_transpose(PG_FUNCTION_ARGS);

Datum pgm_matrix_transpose(PG_FUNCTION_ARGS){
    PGM_Matriz_Double* matrix = (PGM_Matriz_Double*) PG_GETARG_POINTER(0);
    MemoryContext contextoAnterior = MemoryContextSwitchTo( CurTransactionContext );
    PGM_Matriz_Double* transpose = pgm_create_matrix_double(matrix->n_colunas, matrix->n_linhas);
    int i, j;

    MemoryContextSwitchTo( contextoAnterior );

    for (i=0; i < matrix->n_linhas; i++)
        for(j=0; j < matrix->n_colunas; j++)
            PGM_ELEM_MATRIZ( transpose, j, i ) = PGM_ELEM_MATRIZ( matrix, i, j );

    PG_RETURN_POINTER(transpose);
}

PG_FUNCTION_INFO_V1(pgm_matrix_multiply);

/*Function: pgm_matrix_multiply
        Multiplica matriz A por matriz B.
        Onde o numero de colunas de A deve ser igual ao numero de linhas de B
        Em caso de erro, retorno :<b>"Numero de colunas de A diferente de numero de linhas de B"
        Por motivos de ineficiencia, é sugerido usar a função <b>blas_multiply. Mas se tiver um GPU NVidea, então poderá usar <b>pgm_cublas_dgemm ou <b>pgm_cublas_sgemm para melhora significativamente o desempenho
	Parameter:
		PGM_Matriz_Double* matrix_a - Matriz A
		PGM_Matriz_Double* matrix_b - Matriz B

	Return:
		PGM_Matriz_Double* - Ponteiro para a matriz resultante
    See also:
        <blas_multiply>, <pgm_cublas_dgemm>, <pgm_cublas_sgemm>
 */
Datum pgm_matrix_multiply(PG_FUNCTION_ARGS);

void multiplicaPorLinha( double *ci, const double aik, const double* bk, int N );

inline void multiplicaPorLinha( double *ci, const double aik, const double* bk, int N ) {
  int j;

  for( j = 0; j < N; j++ )
    ci[j] += aik * bk[j];
}


void multiplicaPorLinha4( double *ci, const double aik0, const double* bk0,
                                      const double aik1, const double* bk1,
                                      const double aik2, const double* bk2,
                                      const double aik3, const double* bk3, int N );

inline void multiplicaPorLinha4( double *ci, const double aik0, const double* bk0,
                                             const double aik1, const double* bk1,
                                             const double aik2, const double* bk2,
                                             const double aik3, const double* bk3, int N ) {
  int j;

  for( j = 0; j < N; j++ )
    ci[j] +=  aik0 * bk0[j] + aik1 * bk1[j] + aik2 * bk2[j] + aik3 * bk3[j];
}

Datum pgm_matrix_multiply(PG_FUNCTION_ARGS){
    PGM_Matriz_Double* matrix_a = (PGM_Matriz_Double*) PG_GETARG_POINTER(0);
    PGM_Matriz_Double* matrix_b = (PGM_Matriz_Double*) PG_GETARG_POINTER(1);
    if (matrix_a->n_colunas == matrix_b->n_linhas){
        MemoryContext contextoAnterior = MemoryContextSwitchTo( CurTransactionContext );
        PGM_Matriz_Double* multiply = pgm_create_matrix_double(matrix_a->n_linhas, matrix_b->n_colunas);
        int i, k;

        MemoryContextSwitchTo( contextoAnterior );

        for( i = 0; i < multiply->n_linhas; i++ ) {
          for( k = 0; k < matrix_a->n_colunas - matrix_a->n_colunas % 4; k += 4 )
            multiplicaPorLinha4( PGM_LINHA_MATRIZ( multiply, i ), PGM_ELEM_MATRIZ(matrix_a, i, k), PGM_LINHA_MATRIZ(matrix_b, k),
                                                                  PGM_ELEM_MATRIZ(matrix_a, i, k+1), PGM_LINHA_MATRIZ(matrix_b, k+1),
                                                                  PGM_ELEM_MATRIZ(matrix_a, i, k+2), PGM_LINHA_MATRIZ(matrix_b, k+2),
                                                                  PGM_ELEM_MATRIZ(matrix_a, i, k+3), PGM_LINHA_MATRIZ(matrix_b, k+3), multiply->n_colunas );
          for( ; k < matrix_a->n_colunas; k++ )
            multiplicaPorLinha( PGM_LINHA_MATRIZ( multiply, i ), PGM_ELEM_MATRIZ(matrix_a, i, k), PGM_LINHA_MATRIZ(matrix_b, k), multiply->n_colunas );
        }

        PG_RETURN_POINTER(multiply);
    }else{
        elog(ERROR, "Numero de colunas de A diferente de numero de linhas de B: ( %d, %d ) x ( %d, %d )", matrix_a->n_linhas, matrix_a->n_colunas, matrix_b->n_linhas, matrix_b->n_colunas );
        PG_RETURN_INT64(0);
    }
}

PG_FUNCTION_INFO_V1(pgm_matrix_add);
/*Function: pgm_matrix_add
        Soma a matriz a com a matriz b.
        As dimensoes da matriz deve ser iguais aos da matriz b.
	Parameter:
		PGM_Matriz_Double* matrix_a - Matriz A
		PGM_Matriz_Double* matrix_b - Matriz B

	Return:
		PGM_Matriz_Double* - Ponteiro para a matriz resultante
 */
Datum pgm_matrix_add(PG_FUNCTION_ARGS);

Datum pgm_matrix_add(PG_FUNCTION_ARGS){
    PGM_Matriz_Double* matrix_a = (PGM_Matriz_Double*) PG_GETARG_POINTER(0);
    PGM_Matriz_Double* matrix_b = (PGM_Matriz_Double*) PG_GETARG_POINTER(1);
    if (matrix_a->n_linhas == matrix_b->n_linhas && matrix_a->n_colunas == matrix_b->n_colunas){
        MemoryContext contextoAnterior = MemoryContextSwitchTo( CurTransactionContext );
        PGM_Matriz_Double* add = pgm_create_matrix_double(matrix_a->n_linhas, matrix_a->n_colunas);
        int i, j;

        MemoryContextSwitchTo( contextoAnterior );

        for( i = 0; i < add->n_linhas; i++ )
          for( j = 0; j < add->n_colunas; j++ )
            PGM_ELEM_MATRIZ(add, i, j) = (PGM_ELEM_MATRIZ(matrix_a, i, j)) + (PGM_ELEM_MATRIZ(matrix_b, i, j));

        PG_RETURN_POINTER(add);
    }else{
        elog(ERROR, "Tamanho da matriz A diferente do tamanho de B: ( %d, %d ) x ( %d, %d )", matrix_a->n_linhas, matrix_a->n_colunas, matrix_b->n_linhas, matrix_b->n_colunas );
        PG_RETURN_INT64(0);
    }
}

PG_FUNCTION_INFO_V1(pgm_matrix_subtract);
/*Function: pgm_matrix_subtract
        Subtrai a matriz a com a matriz b.
        As dimensoes da matriz deve ser iguais aos da matriz b.
	Parameter:
		PGM_Matriz_Double* matrix_a - Matriz A
		PGM_Matriz_Double* matrix_b - Matriz B

	Return:
		PGM_Matriz_Double* - Ponteiro para a matriz resultante
 */
Datum pgm_matrix_subtract(PG_FUNCTION_ARGS);

Datum pgm_matrix_subtract(PG_FUNCTION_ARGS){
    PGM_Matriz_Double* matrix_a = (PGM_Matriz_Double*) PG_GETARG_POINTER(0);
    PGM_Matriz_Double* matrix_b = (PGM_Matriz_Double*) PG_GETARG_POINTER(1);
    if (matrix_a->n_linhas == matrix_b->n_linhas && matrix_a->n_colunas == matrix_b->n_colunas){
        MemoryContext contextoAnterior = MemoryContextSwitchTo( CurTransactionContext );
        PGM_Matriz_Double* subtract = pgm_create_matrix_double(matrix_a->n_linhas, matrix_a->n_colunas);
        int i, j;

        MemoryContextSwitchTo( contextoAnterior );

        for( i = 0; i < subtract->n_linhas; i++ )
          for( j = 0; j < subtract->n_colunas; j++ )
            PGM_ELEM_MATRIZ(subtract, i, j) = (PGM_ELEM_MATRIZ(matrix_a, i, j)) - (PGM_ELEM_MATRIZ(matrix_b, i, j));

        PG_RETURN_POINTER(subtract);
    }else{
        elog(ERROR, "Tamanho da matriz A diferente do tamanho de B: ( %d, %d ) x ( %d, %d )", matrix_a->n_linhas, matrix_a->n_colunas, matrix_b->n_linhas, matrix_b->n_colunas );
        PG_RETURN_INT64(0);
    }
}

PG_FUNCTION_INFO_V1(pgm_matrix_norma);
/*Function: pgm_matrix_norma
        Calcula a norma da matriz
	Parameter:
		PGM_Matriz_Double* matriz

	Return:
		PGM_Matriz_Double* - Ponteiro para a matriz resultante
 */
Datum pgm_matrix_norma(PG_FUNCTION_ARGS);

Datum pgm_matrix_norma(PG_FUNCTION_ARGS){
	PGM_Matriz_Double* matrix = (PGM_Matriz_Double*) PG_GETARG_POINTER(0);
	double result = 0.0;
	int i, j;

	for( i = 0; i < matrix->n_linhas; i++ )
		for( j = 0; j < matrix->n_colunas; j++ )
			result += PGM_ELEM_MATRIZ(matrix, i, j) * PGM_ELEM_MATRIZ(matrix, i, j);

	PG_RETURN_FLOAT8(sqrt(result));
}

PG_FUNCTION_INFO_V1(pgm_vector_norma);
/*Function: pgm_vector_norma
        Calcula a norma do vetor
	Parameter:
		PGM_Matriz_Double* matriz

	Return:
		PGM_Matriz_Double* - Ponteiro para a matriz resultante
 */
Datum pgm_vector_norma(PG_FUNCTION_ARGS);

Datum pgm_vector_norma(PG_FUNCTION_ARGS){
	PGM_Vetor_Double* vector = (PGM_Vetor_Double*) PG_GETARG_POINTER(0);
	double result = 0.0;
	int i;
	for( i = 0; i < vector->n_elems; i++ )
			result += PGM_ELEM_VETOR(vector, i) * PGM_ELEM_VETOR(vector, i);

	PG_RETURN_FLOAT8(sqrt(result));
}

PG_FUNCTION_INFO_V1(pgm_matrix2arraytype);
/*Function: pgm_matrix2arraytype
        Converte de PGM_Matriz_Double para ArrayType
	Parameter:
		PGM_Matriz_Double* matrix

	Return:
		ArrayType* - Ponteiro para o array resultante
 */
Datum pgm_matrix2arraytype(PG_FUNCTION_ARGS);

Datum pgm_matrix2arraytype(PG_FUNCTION_ARGS){
	PGM_Matriz_Double* matrix = (PGM_Matriz_Double*) PG_GETARG_POINTER(0);
	PG_RETURN_ARRAYTYPE_P(pgm_PGM_Matriz_Double2ArrayType(matrix));
}


PG_FUNCTION_INFO_V1(pgm_matrix_float2arraytype);
/*Function: pgm_matrix_float2arraytype
        Converte de PGM_Matriz_Float para ArrayType
	Parameter:
		PGM_Matriz_Float* matrix

	Return:
		ArrayType* - Ponteiro para o array resultante
 */
Datum pgm_matrix_float2arraytype(PG_FUNCTION_ARGS);

Datum pgm_matrix_float2arraytype(PG_FUNCTION_ARGS){
	PGM_Matriz_Float* matrix = (PGM_Matriz_Float*) PG_GETARG_POINTER(0);
	PG_RETURN_ARRAYTYPE_P(pgm_PGM_Matriz_Float2ArrayType(matrix));
}

PG_FUNCTION_INFO_V1(pgm_vectorDouble2arraytype);
/*Function: pgm_vectorDouble2arraytype
        Converte PGM_Vetor_Double para ArrayType
	Parameter:
		PGM_Vetor_Double* vetor - Vetor

	Return:
		ArrayType* - Ponteiro para o array resultante
 */
Datum pgm_vectorDouble2arraytype(PG_FUNCTION_ARGS);

Datum pgm_vectorDouble2arraytype(PG_FUNCTION_ARGS){
	PGM_Vetor_Double* vector = (PGM_Vetor_Double*) PG_GETARG_POINTER(0);
	PG_RETURN_ARRAYTYPE_P(pgm_PGM_Vetor_Double2ArrayType(vector));
}

PG_FUNCTION_INFO_V1(pgm_arraytype2Matrix_Float);
/*Function: pgm_arraytype2Matrix_Float
            Converte ArrayType para PGM_Matrix_Float
	Parameter:
		ArraType - array de entrada

	Return:
		PGM_Matriz_Float* - Ponteiro para a matriz resultante
 */
Datum pgm_arraytype2Matrix_Float(PG_FUNCTION_ARGS);

Datum pgm_arraytype2Matrix_Float(PG_FUNCTION_ARGS){
	PGM_Matriz_Float* m =  pgm_ArrayType2PGM_Matriz_Float(PG_GETARG_ARRAYTYPE_P(0));
	PG_RETURN_POINTER(m);
}


PG_FUNCTION_INFO_V1(pgm_vectorInt2arraytype);
/*Function: pgm_matrix_add
        Converte um PGM_Vetor_Int para ArrayType
	Parameter:
		PGM_Vetor_Int *vetor - ponteiro para o vetor

	Return:
		ArrayType* - Ponteiro para a array resultante
 */
Datum pgm_vectorInt2arraytype(PG_FUNCTION_ARGS);

Datum pgm_vectorInt2arraytype(PG_FUNCTION_ARGS){
	PGM_Vetor_Int* vector = (PGM_Vetor_Int*) PG_GETARG_POINTER(0);
	PG_RETURN_ARRAYTYPE_P(pgm_PGM_Vetor_Int2ArrayType(vector));

}

PG_FUNCTION_INFO_V1(pgm_matrix_apply_scale);
/*Function: pgm_matrix_apply_scale
        Aplica a escala f na matriz A
        Onde o numero de colunas de A deve ser igual ao numero de linhas de B
        Em caso de erro, retorno :<b>"Numero de colunas de A diferente de numero de linhas de B"
	Parameter:
		PGM_Matriz_Double* matrix - Matriz A
		double f - Escala

	Return:
		PGM_Matriz_Double* - Ponteiro para a matriz A
 */
Datum pgm_matrix_apply_scale(PG_FUNCTION_ARGS);

Datum pgm_matrix_apply_scale(PG_FUNCTION_ARGS){
	PGM_Matriz_Double* matrix = (PGM_Matriz_Double*) PG_GETARG_POINTER(0);
	double f = PG_GETARG_FLOAT8(1);
	matrix_apply_scale(matrix, f);
	PG_RETURN_POINTER(matrix);
}

PG_FUNCTION_INFO_V1(pgm_matrix_set_col_value);
/*Function: pgm_matrix_set_col_value
        Troca o valor de uma coluna da matriz A para um determinado valor
	Parameter:
		PGM_Matriz_Double* matrix - Matriz A
		double valor - Valor a ser setado

	Return:
		PGM_Matriz_Double* - onteiro para a matriz A
 */
Datum pgm_matrix_set_col_value(PG_FUNCTION_ARGS);

Datum pgm_matrix_set_col_value(PG_FUNCTION_ARGS){
	PGM_Matriz_Double *matrix = (PGM_Matriz_Double*) PG_GETARG_POINTER(0);
	int column = PG_GETARG_INT32(1);
	double value = PG_GETARG_FLOAT8(2);

	matrix_set_col_value(matrix, column, value);

	PG_RETURN_POINTER(matrix);
}

PG_FUNCTION_INFO_V1(pgm_matrix_set_line_value);
/*Function: pgm_matrix_set_line_value
        Troca o valor de uma linha da matriz A para um determinado valor
	Parameter:
		PGM_Matriz_Double* matrix - Matriz A
		double valor - Valor a ser setado

	Return:
		PGM_Matriz_Double* - onteiro para a matriz A
 */
Datum pgm_matrix_set_line_value(PG_FUNCTION_ARGS);

Datum pgm_matrix_set_line_value(PG_FUNCTION_ARGS){
	PGM_Matriz_Double *matrix = (PGM_Matriz_Double*) PG_GETARG_POINTER(0);
	int line = PG_GETARG_INT32(1);
	double value = PG_GETARG_FLOAT8(2);

	matrix_set_line_value(matrix, line, value);

	PG_RETURN_POINTER(matrix);
}

PG_FUNCTION_INFO_V1(blas_multiply);
/*Function: blas_multiply
        Multiplica matriz A por matriz B.
        Onde o numero de colunas de A deve ser igual ao numero de linhas de B
        Em caso de erro, retorno :<b>"Numero de colunas de A diferente de numero de linhas de B"
        Utiliza a biblioteca Blas para otimizar o processo.
	Parameter:
		PGM_Matriz_Double* matrix_a - Matriz A
		PGM_Matriz_Double* matrix_b - Matriz B

	Return:
		PGM_Matriz_Double* - Ponteiro para a matriz resultante
    See also:
        <blas_multiply>, <pgm_cublas_dgemm>, <pgm_cublas_sgemm>
 */
Datum blas_multiply(PG_FUNCTION_ARGS);

Datum blas_multiply(PG_FUNCTION_ARGS){
	PGM_Matriz_Double *A = (PGM_Matriz_Double*) PG_GETARG_POINTER(0);
	PGM_Matriz_Double *B = (PGM_Matriz_Double*) PG_GETARG_POINTER(1);

	if(A->n_colunas == B->n_linhas){
		int lda = A->n_colunas, ldb = B->n_colunas;
		MemoryContext contextoAnterior = MemoryContextSwitchTo( CurTransactionContext );
		PGM_Matriz_Double *C = pgm_create_matrix_double(A->n_linhas, B->n_colunas);
		MemoryContextSwitchTo( contextoAnterior );
		//Multiplica as matrizes AxB = C, cblas_dgemm libblas
		cblas_dgemm(CblasRowMajor,CblasNoTrans, CblasNoTrans,A->n_linhas,B->n_colunas,A->n_colunas, 1.0,  A->valor,lda, B->valor,ldb,0.0,C->valor,C->n_colunas);

		PG_RETURN_POINTER(C);

	}else{
		elog(ERROR, "Numero de linhas de A(%d) é diferente no numero de colunas de B(%d)\n", A->n_colunas, B->n_linhas);
		PG_RETURN_VOID();
	}

}

PG_FUNCTION_INFO_V1(make_random_matrix);
/*Function: make_random_matrix
            Cria uma matriz randomica
	Parameter:
		int n_lines - numero de linhas da matriz desejada
        int n_col - numero de linhas da matriz desejada
        double a - fator A
        double b - fator B

	Return:
		PGM_Matriz_Double* - Ponteiro para a matriz resultante

 */
Datum make_random_matrix(PG_FUNCTION_ARGS);

Datum make_random_matrix(PG_FUNCTION_ARGS){
    int n_lines = PG_GETARG_INT32(0);
    int n_col = PG_GETARG_INT32(1);
    double a = PG_GETARG_FLOAT8(2);
    double b = PG_GETARG_FLOAT8(3);
    MemoryContext contextoAnterior = MemoryContextSwitchTo( CurTransactionContext );
    PGM_Matriz_Double* result = pgm_create_matrix_double(n_lines,n_col);

    MemoryContextSwitchTo( contextoAnterior );
    {
        int n = n_lines*n_col, i;

        for(i = 0; i < n; i++)
            result->valor[i] = a + (b-a)*((double)rand()/2147483647 );
    }

    PG_RETURN_POINTER(result);
}


PG_FUNCTION_INFO_V1(pgm_matrix_remove_line_col);
/*Function: matrix_remove_line_col
		Cria uma nova matriz removendo 'linhas' e 'colunas'.
	Parameter:
		PGM_Matriz_Double* matrix
		PGM_Vetor_Int *linhas - indices das linhas que serão removidas
		PGM_Vetor_Int *colunas - indices das colunas que serao removidas
	Return:
        	PGM_Matriz_Double* - matriz nova
*/
Datum pgm_matrix_remove_line_col(PG_FUNCTION_ARGS);

Datum pgm_matrix_remove_line_col(PG_FUNCTION_ARGS){
    PGM_Matriz_Double *matrix = (PGM_Matriz_Double*) PG_GETARG_POINTER(0);
    PGM_Vetor_Int *linhas = pgm_ArrayType2PGM_Vetor_Int(PG_GETARG_ARRAYTYPE_P(1));
    PGM_Vetor_Int *colunas = pgm_ArrayType2PGM_Vetor_Int(PG_GETARG_ARRAYTYPE_P(2));
    MemoryContext contextoAnterior = MemoryContextSwitchTo( CurTransactionContext );
    PGM_Matriz_Double* result = matrix_remove_line_col( matrix, linhas, colunas );

    MemoryContextSwitchTo( contextoAnterior );
    PG_RETURN_POINTER(result);
}
