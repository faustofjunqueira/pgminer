/*
 * Autor: GZS, FFJ
 * GCCBD - COOPE - UFRJ
 * Criacao: 05/11/2012
 */
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
#include "../Util/pgm_matriz.h"
#include "../Util/pgm_matriz_util.h"
#include "../pgm_SVD/pure_c_svd.h"
#include "../pgm_math/pgm_math_util.h"

#include "pgm_lapack_util.h"

PG_FUNCTION_INFO_V1(pgm_ordinary_least_squares);
Datum pgm_ordinary_least_squares(PG_FUNCTION_ARGS);

Datum pgm_ordinary_least_squares(PG_FUNCTION_ARGS){
	MemoryContext contextoAnterior;
	PGM_Matriz_Double *a = (PGM_Matriz_Double*) PG_GETARG_POINTER(0);
	PGM_Matriz_Double *b = (PGM_Matriz_Double*) PG_GETARG_POINTER(1);
	PGM_Matriz_Double *trans_a;
	PGM_Matriz_Double *trans_b;
	PGM_Matriz_Double *x;
	PGM_Vetor_Double *s;
	int rank;

	if(a->n_linhas != b->n_linhas){
		printf("ERROR: O numero de linhas de A deve ser igual ao numero de linhas de B");
		return -1;
	}

	trans_a = matrix_transpose(a);
	trans_b = matrix_transpose(b);

	if( trans_b->n_colunas < trans_a->n_linhas){
		trans_b = matrix_realloc(trans_b, trans_b->n_linhas, trans_a->n_linhas);
	}

	s = pgm_create_PGM_Vetor_Double(min(a->n_colunas, a->n_linhas));

	rank = ordinary_least_squares(trans_a, trans_b, s);

	contextoAnterior = MemoryContextSwitchTo( CurTransactionContext );
	x = matrix_transpose2(trans_b, trans_a->n_linhas, trans_b->n_linhas);
	MemoryContextSwitchTo( contextoAnterior );

	{

		Datum valor[3];
		bool *tup_isnull;
		TupleDesc tupdesc;
		HeapTuple tupla;

		if (get_call_result_type (fcinfo , NULL , &tupdesc) != TYPEFUNC_COMPOSITE)
				elog(ERROR , "Não foi possivel concluir a construção da tupla! reveja os data types");

		valor[0] = PointerGetDatum( x );
		valor[1] = PointerGetDatum( s );
		valor[2] = Int32GetDatum(rank);

		BlessTupleDesc (tupdesc);
		tup_isnull = palloc0 ( tupdesc->natts * sizeof(bool));

		tupla = heap_form_tuple ( tupdesc ,  valor , tup_isnull );

		PG_RETURN_DATUM (HeapTupleGetDatum ( tupla ));
    }
}

PG_FUNCTION_INFO_V1(pgm_lapack_svd);
Datum pgm_lapack_svd(PG_FUNCTION_ARGS);

Datum pgm_lapack_svd(PG_FUNCTION_ARGS){
	PGM_Matriz_Double *Matriz = (PGM_Matriz_Double*) PG_GETARG_POINTER(0);

	MemoryContext contextoAnterior = MemoryContextSwitchTo( CurTransactionContext );
	PGM_svdrec *pgm_rec =  lapack_svd( Matriz);
	MemoryContextSwitchTo( contextoAnterior );

	if(pgm_rec == NULL) elog(ERROR, "O retorno esta NULL.");
	{

		Datum valor[3];
		bool *tup_isnull;
		TupleDesc tupdesc;
		HeapTuple tupla;

		if (get_call_result_type (fcinfo , NULL , &tupdesc) != TYPEFUNC_COMPOSITE)
				elog(ERROR , "Não foi possivel concluir a construção da tupla! reveja os data types");

		valor[0] = (Datum) pgm_rec->Ut;
		valor[1] = (Datum) pgm_rec->S;;
		valor[2] = (Datum) pgm_rec->Vt;

		BlessTupleDesc (tupdesc);
		tup_isnull = palloc0 ( tupdesc->natts * sizeof(bool));

		tupla = heap_form_tuple ( tupdesc ,  valor , tup_isnull );

		PG_RETURN_DATUM (HeapTupleGetDatum ( tupla ));
	}

}

PG_FUNCTION_INFO_V1(pgm_matrix_inverse);
Datum pgm_matrix_inverse(PG_FUNCTION_ARGS);
Datum pgm_matrix_inverse(PG_FUNCTION_ARGS){
	PGM_Matriz_Double *m = (PGM_Matriz_Double*) PG_GETARG_POINTER(0);
	PGM_Matriz_Double* copy_matrix;
	MemoryContext contextoAnterior;
	int i, tam = m->n_linhas*m->n_colunas;
	if(m->n_linhas != m->n_colunas) elog(ERROR, "A matriz deve ser quadrada.");

	contextoAnterior = MemoryContextSwitchTo( CurTransactionContext );
	copy_matrix = pgm_create_matrix_double(m->n_linhas,m->n_colunas);
	MemoryContextSwitchTo( contextoAnterior );

	for(i=0; i < tam; i++) copy_matrix->valor[i] = m->valor[i];

	matrix_inverse(copy_matrix->valor, copy_matrix->n_linhas);

	PG_RETURN_POINTER(copy_matrix);
}

