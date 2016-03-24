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
#include "tsearch/ts_type.h" //ts_vector

#include "../Util/pgm_matriz.h"
#include "../Util/pgm_matriz_util.h"
#include "../Util/pgm_malloc.h"
#include "../Util/pgm_string.h"

#include "../pgm_tsvector/pgm_tsvector_util.h"

PG_FUNCTION_INFO_V1(pgm_tsvector2words);
Datum pgm_tsvector2words(PG_FUNCTION_ARGS);

Datum pgm_tsvector2words(PG_FUNCTION_ARGS){
    TSVector in = PG_GETARG_TSVECTOR(0);
    MemoryContext contextoAnterior = MemoryContextSwitchTo( CurTransactionContext );
    ArrayType *a = tsvector_words2arraytype(in);
    MemoryContextSwitchTo( contextoAnterior  );
    PG_RETURN_ARRAYTYPE_P( a );
}

PG_FUNCTION_INFO_V1(pgm_create_vector_presence);
Datum pgm_create_vector_presence(PG_FUNCTION_ARGS);

Datum pgm_create_vector_presence(PG_FUNCTION_ARGS){
    PGM_Vetor_String *mensagem = pgm_ArrayType2PGM_Vetor_String(PG_GETARG_ARRAYTYPE_P(0));
    PGM_Vetor_String *dict  = pgm_ArrayType2PGM_Vetor_String(PG_GETARG_ARRAYTYPE_P(1));
    MemoryContext contextoAnterior = MemoryContextSwitchTo( CurTransactionContext );
    PGM_Vetor_Int *result = cria_vetor_presenca(mensagem,dict);
    MemoryContextSwitchTo( contextoAnterior  );
    PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(pgm_create_vector_presence_frequency);
Datum pgm_create_vector_presence_frequency(PG_FUNCTION_ARGS);

Datum pgm_create_vector_presence_frequency(PG_FUNCTION_ARGS){
    TSVector vector = PG_GETARG_TSVECTOR(0);
    PGM_Vetor_String *dict = pgm_ArrayType2PGM_Vetor_String(PG_GETARG_ARRAYTYPE_P(1));
    int flag = PG_GETARG_INT32(2);
    PGM_Vetor_Int *presence = NULL, *frequency = NULL;
    MemoryContext contextoAnterior = MemoryContextSwitchTo( CurTransactionContext );
    tsvector2presence_frequency(vector, dict, &presence, &frequency, flag);
    MemoryContextSwitchTo( contextoAnterior  );
    {

        Datum valor[2];
        bool *tup_isnull;
        TupleDesc tupdesc;
        HeapTuple tupla;

        if (get_call_result_type (fcinfo , NULL , &tupdesc) != TYPEFUNC_COMPOSITE)
            elog(ERROR , "Não foi possivel concluir a construção da tupla! reveja os data types");

        valor[0] = PointerGetDatum( presence );
        valor[1] = PointerGetDatum( frequency );


        BlessTupleDesc (tupdesc);
        tup_isnull = palloc0 ( tupdesc->natts * sizeof(bool));
        tup_isnull[0] = presence == NULL ? 1 : 0;
        tup_isnull[1] = frequency == NULL ? 1 : 0;
        tupla = heap_form_tuple ( tupdesc ,  valor , tup_isnull );

        PG_RETURN_DATUM (HeapTupleGetDatum ( tupla ));
    }

}

