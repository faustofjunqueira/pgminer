#include "postgres.h"       //standard
#include "fmgr.h"           //standard
#include "utils/memutils.h"//standard
#include "utils/geo_decls.h"//standard
#include "utils/errcodes.h" //warning

//#include "../Util/pgm_matriz_util.h"
#include "pure_c_hash.h"


PG_FUNCTION_INFO_V1(PGM_Int_Hashset_Find);
Datum PGM_Int_Hashset_Find(PG_FUNCTION_ARGS);

Datum PGM_Int_Hashset_Find(PG_FUNCTION_ARGS){
    PGM_IntHashSet *hash_table = (PGM_IntHashSet*) PG_GETARG_POINTER(0);
    int id = PG_GETARG_INT32(1);
    PG_RETURN_BOOL(pgm_int_hashset_find(hash_table,id));
}

PG_FUNCTION_INFO_V1(PGM_Int_Hashset_Add);
Datum PGM_Int_Hashset_Add(PG_FUNCTION_ARGS);

Datum PGM_Int_Hashset_Add(PG_FUNCTION_ARGS){
    PGM_IntHashSet *hash_table = (PGM_IntHashSet*) PG_GETARG_POINTER(1);
    int id = PG_GETARG_INT32(0);
    MemoryContext contextoAnterior = MemoryContextSwitchTo( CurTransactionContext );
    pgm_int_hashset_add(&hash_table, id);
    MemoryContextSwitchTo( contextoAnterior );
    PG_RETURN_POINTER(hash_table);
}

PG_FUNCTION_INFO_V1(PGM_Int_Hashset_Add_Vector);
Datum PGM_Int_Hashset_Add_Vector(PG_FUNCTION_ARGS);

Datum PGM_Int_Hashset_Add_Vector(PG_FUNCTION_ARGS){
    PGM_Vetor_Int *vector = (PGM_Vetor_Int*) PG_GETARG_POINTER(0);
    MemoryContext contextoAnterior = MemoryContextSwitchTo( CurTransactionContext );
    PGM_IntHashSet *hash_table = pgm_int_hashset_add_vector(vector);
    MemoryContextSwitchTo( contextoAnterior );
    PG_RETURN_POINTER(hash_table);
}

PG_FUNCTION_INFO_V1(pgm_PGM_IntHashSet22PGM_Vetor_Int);
Datum pgm_PGM_IntHashSet22PGM_Vetor_Int(PG_FUNCTION_ARGS);

Datum pgm_PGM_IntHashSet22PGM_Vetor_Int(PG_FUNCTION_ARGS){
    PGM_IntHashSet *hash_table = (PGM_IntHashSet*) PG_GETARG_POINTER(0);
    MemoryContext contextoAnterior = MemoryContextSwitchTo( CurTransactionContext );
    PGM_Vetor_Int *vector = PGM_IntHashSet22PGM_Vetor_Int(hash_table);
    MemoryContextSwitchTo( contextoAnterior );
    PG_RETURN_POINTER(vector);
}
