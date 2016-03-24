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

#include <math.h>
#include "../Util/pgm_matriz.h"
#include "../Util/pgm_matriz_util.h"
#include "pgm_math_util.h"


PG_FUNCTION_INFO_V1(pgm_matrix_stats);
Datum pgm_matrix_stats(PG_FUNCTION_ARGS);
// MIN, MAX, SUM_X, SUM_X2, COUNT, AVG, STDDEV_SAMP, STDDEV_POP, COUNT_NZ, AVG_NZ, STDDEV_SAMP_NZ, STDDEV_POP_NZ
// '[ L C M ]' ==> Line stats, Col stats, Matrix stats

Datum pgm_matrix_stats(PG_FUNCTION_ARGS){
    PGM_Matriz_Double *A = (PGM_Matriz_Double*) PG_GETARG_POINTER(0);
    MemoryContext contextoAnterior = MemoryContextSwitchTo( CurTransactionContext );
    PGM_Matriz_Double *L = NULL, *C = NULL, *M = NULL;

    if( A->n_colunas < 1 || A->n_linhas < 1 )
      elog( ERROR, "A matriz deve ter mais de uma linha e de uma coluna");

    matrix_stats( A, &L, &C, &M );

    MemoryContextSwitchTo( contextoAnterior );

    {
        Datum valor[3];
        bool *tup_isnull;
        TupleDesc tupdesc;
        HeapTuple tupla;

        if (get_call_result_type (fcinfo , NULL , &tupdesc) != TYPEFUNC_COMPOSITE)
            elog(ERROR , "Não foi possivel concluir a construção da tupla! reveja os data types");

        valor[0] = (Datum) L;
        valor[1] = (Datum) C;
        valor[2] = (Datum) M;

        BlessTupleDesc (tupdesc);
        tup_isnull = palloc0 ( tupdesc->natts * sizeof(bool));

        tupla = heap_form_tuple ( tupdesc ,  valor , tup_isnull );

        PG_RETURN_DATUM (HeapTupleGetDatum ( tupla ));
    }
}

PG_FUNCTION_INFO_V1(pgm_center_matrix);
Datum pgm_center_matrix(PG_FUNCTION_ARGS);
// Centraliza a matrix M no vector V

Datum pgm_center_matrix(PG_FUNCTION_ARGS){
	PGM_Matriz_Double *M = (PGM_Matriz_Double*) PG_GETARG_POINTER(0);
	PGM_Vetor_Double  *V = (PGM_Vetor_Double*) PG_GETARG_POINTER(1);

    if (M->n_colunas != V->n_elems)
        elog(ERROR, "O numero de colunas de M e elementos de B devem ser iguais!");
    else
        center_matrix( M, V );

    PG_RETURN_POINTER(M);
}


PG_FUNCTION_INFO_V1(pgm_lsh_cos);
Datum pgm_lsh_cos(PG_FUNCTION_ARGS);

Datum pgm_lsh_cos(PG_FUNCTION_ARGS){
	PGM_Matriz_Double *A = (PGM_Matriz_Double*) PG_GETARG_POINTER(0);
	PGM_Matriz_Double *B = (PGM_Matriz_Double*) PG_GETARG_POINTER(1);
	PGM_Matriz_Double *C = (PGM_Matriz_Double*) PG_GETARG_POINTER(2);

	if (A->n_colunas != B->n_linhas)
	  elog(ERROR, "O numero de colunas de A e de linhas de B devem ser iguais!");
    else {
        MemoryContext contextoAnterior;
        contextoAnterior = MemoryContextSwitchTo( CurTransactionContext );

        lsh_cos( A, B, C );

        MemoryContextSwitchTo( contextoAnterior );
    }

    PG_RETURN_POINTER(C);
}



PG_FUNCTION_INFO_V1(pgm_cos);
Datum pgm_cos(PG_FUNCTION_ARGS);

Datum pgm_cos(PG_FUNCTION_ARGS){
	PGM_Matriz_Double *A = (PGM_Matriz_Double*) PG_GETARG_POINTER(0);
	PGM_Matriz_Double *B = (PGM_Matriz_Double*) PG_GETARG_POINTER(1);
	PGM_Vetor_Double *result;
	MemoryContext contextoAnterior;

	if (A->n_linhas != B->n_linhas && A->n_colunas != B->n_colunas)
	  elog(ERROR, "O numero de linhas e colunas de A e B devem ser iguais!");

	contextoAnterior = MemoryContextSwitchTo( CurTransactionContext );
	result = cos_ndimensional(A,B);
	MemoryContextSwitchTo( contextoAnterior );
	PG_RETURN_POINTER(result);

}

PG_FUNCTION_INFO_V1(_cos);
Datum _cos(PG_FUNCTION_ARGS);

Datum _cos(PG_FUNCTION_ARGS){

	PG_RETURN_FLOAT8(cos_vetorial(pgm_ArrayType2PGM_Vetor_Double(PG_GETARG_ARRAYTYPE_P(0)),pgm_ArrayType2PGM_Vetor_Double(PG_GETARG_ARRAYTYPE_P(1))));
}

PG_FUNCTION_INFO_V1(_sen);
Datum _sen(PG_FUNCTION_ARGS);

Datum _sen(PG_FUNCTION_ARGS){
    double Cos = cos_vetorial(pgm_ArrayType2PGM_Vetor_Double(PG_GETARG_ARRAYTYPE_P(0)),pgm_ArrayType2PGM_Vetor_Double(PG_GETARG_ARRAYTYPE_P(1)));
	PG_RETURN_FLOAT8(sqrt(1 - Cos*Cos));
}

PG_FUNCTION_INFO_V1(pgm_weighted_mean);
Datum pgm_weighted_mean(PG_FUNCTION_ARGS);

Datum pgm_weighted_mean(PG_FUNCTION_ARGS){
	PGM_Matriz_Double *A = (PGM_Matriz_Double*) PG_GETARG_POINTER(0);
	int p = PG_GETARG_FLOAT8(1);
	PGM_Matriz_Double *B = (PGM_Matriz_Double*) PG_GETARG_POINTER(2);
	int q = PG_GETARG_FLOAT8(3);
	PGM_Matriz_Double *result;
	MemoryContext contextoAnterior;

	if (A->n_linhas != B->n_linhas && A->n_colunas != B->n_colunas) elog(ERROR, "O numero de linhas e colunas de A e B devem ser iguais!");

	contextoAnterior = MemoryContextSwitchTo( CurTransactionContext );
	result = weighted_mean(A, p, B, q);
	MemoryContextSwitchTo( contextoAnterior );

	PG_RETURN_POINTER(result);

}


