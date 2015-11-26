/*
 * Autor: GZS, FFJ
 * GCCBD - COOPE - UFRJ
 * Criacao: 12/04/2012
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

#include "svdlib.h"
#include "pure_c_svd.h"

#include <assert.h>

PG_FUNCTION_INFO_V1(pgm_svd);
Datum pgm_svd(PG_FUNCTION_ARGS);

Datum pgm_svd(PG_FUNCTION_ARGS){
    PGM_Matriz_Double *Matriz = (PGM_Matriz_Double*) PG_GETARG_POINTER(0);
    int numFeatures = PG_GETARG_INT32(1);

    MemoryContext contextoAnterior = MemoryContextSwitchTo( CurTransactionContext );

    PGM_svdrec *pgm_rec =  pure_c_SVD( Matriz, numFeatures );

    MemoryContextSwitchTo( contextoAnterior );

    {

        Datum valor[3];
        bool *tup_isnull;
        TupleDesc tupdesc;
        HeapTuple tupla;

        if (get_call_result_type (fcinfo , NULL , &tupdesc) != TYPEFUNC_COMPOSITE)
            elog(ERROR , "Não foi possivel concluir a construção da tupla! reveja os data types");

        valor[0] = (Datum) pgm_rec->Ut;
        valor[1] = (Datum) pgm_rec->S;
        valor[2] = (Datum) pgm_rec->Vt;

        BlessTupleDesc (tupdesc);
        tup_isnull = palloc0 ( tupdesc->natts * sizeof(bool));

        tupla = heap_form_tuple ( tupdesc ,  valor , tup_isnull );

        PG_RETURN_DATUM (HeapTupleGetDatum ( tupla ));
    }

}

PG_FUNCTION_INFO_V1(pgm_svd_matrix_reconstruct);
Datum pgm_svd_matrix_reconstruct(PG_FUNCTION_ARGS);

Datum pgm_svd_matrix_reconstruct(PG_FUNCTION_ARGS){
    PGM_Matriz_Double* Ut = (PGM_Matriz_Double*) PG_GETARG_POINTER(0);
    PGM_Vetor_Double*   S = (PGM_Vetor_Double*)  PG_GETARG_POINTER(1);
    PGM_Matriz_Double* Vt = (PGM_Matriz_Double*) PG_GETARG_POINTER(2);
    int numFeatures = PG_GETARG_INT32(3);
    PGM_svdrec rec = { Ut, S, Vt };
    MemoryContext contextoAnterior = MemoryContextSwitchTo( CurTransactionContext );

    PGM_Matriz_Double *m = pure_c_SVD_matrix_reconstruct( &rec, numFeatures );

    MemoryContextSwitchTo( contextoAnterior );

    PG_RETURN_POINTER( m );
}

