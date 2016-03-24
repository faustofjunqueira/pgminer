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
#include "tsearch/ts_type.h" //ts_vector

#include "pgm_string.h"
#include "pgm_types.h"
#include "pgm_malloc.h"

PGM_Vetor_String* pgm_create_vetor_string(int n_elem){
    PGM_Vetor_String* a = (PGM_Vetor_String*) pgm_malloc (sizeof(PGM_Vetor_String));
    a->n_elem = n_elem;
    a->array = (Datum*) pgm_malloc (sizeof(Datum) * n_elem);
    return a;
}

void pgm_charptr_set(PGM_Vetor_String *vetor, int index,char* elem){
    if(index < vetor->n_elem){
        vetor->array[index] = CStringGetDatum(elem);
    }else
        elog(ERROR,"Valor do index maior que esperado (%d) - (%d)",index, vetor->n_elem);
}

int pgm_charptr_realloc(PGM_Vetor_String* vetor, int novo_l){
    if ( novo_l <= vetor->n_elem){
        Datum *aux = (Datum*) pgm_malloc(sizeof(Datum)* novo_l);
        Datum *tmp = vetor->array;
        vetor->array = aux;
        aux = tmp;
        memcpy( vetor->array, aux, sizeof(Datum)*novo_l );
        pfree( aux );
        vetor->n_elem = novo_l;
        return 1;
    } else return 0;

}

char **pgm_vetor_string2charptr(PGM_Vetor_String *string){
    char **r = (char**) pgm_malloc(sizeof(char*) * string->n_elem);
    int i;
    for(i=0; i < string->n_elem; i++)
        r[i] = pgm_charptr_get_elem(string,i);
    return r;
}

PGM_Vetor_String* pgm_ArrayType2PGM_Vetor_String(ArrayType *array){
    if( ARR_NDIM(array) == 1 ) {
      PGM_Vetor_String* result = pgm_create_vetor_string(ARR_DIMS(array)[0]);
      int i;
      char* pt = ARR_DATA_PTR(array);
      for(i = 0; i < result->n_elem; i++){
        pgm_charptr_set(result,i,pt);
        pt += strlen(pt) + 1;
      }
      return result;
    }else elog(ERROR,"Nao foi possivel converter arraytype em pgm_vetor_string");
    return NULL;
}

ArrayType *pgm_PGM_Vetor_String2ArrayType(PGM_Vetor_String *v){
    int4         ndims          = 1;
    bool        *isnull         = (bool*) pgm_malloc ( sizeof(bool)*v->n_elem );
    int4         dims[MAXDIM];
    int4         lbs[MAXDIM];
    Oid          IDtipo         = CSTRINGOID;
    int16 elmlen; bool elmbyval; char elmalign;

    dims[0] = v->n_elem;
    lbs[0] = 1;

    get_typlenbyvalalign ( IDtipo , &elmlen , &elmbyval , &elmalign);
    elog(INFO,"aki");
    return construct_md_array (v->array, isnull , ndims , dims , lbs ,
                                IDtipo ,elmlen , elmbyval , elmalign );

}

PG_FUNCTION_INFO_V1(pgm_vetor_cstring_create);
Datum pgm_vetor_cstring_create(PG_FUNCTION_ARGS);

Datum pgm_vetor_cstring_create(PG_FUNCTION_ARGS){
    int n_elem = PG_GETARG_INT32(0);
    MemoryContext contextoAnterior = MemoryContextSwitchTo( CurTransactionContext );
    PGM_Vetor_String *v = pgm_create_vetor_string(n_elem);
    MemoryContextSwitchTo( contextoAnterior );
    PG_RETURN_POINTER(v);
}

PG_FUNCTION_INFO_V1(pgm_vetor_cstring_set_elem);
Datum pgm_vetor_cstring_set_elem(PG_FUNCTION_ARGS);

Datum pgm_vetor_cstring_set_elem(PG_FUNCTION_ARGS){
    int pos = PG_GETARG_INT32(1);
    PGM_Vetor_String *v = (PGM_Vetor_String*) PG_GETARG_POINTER(0);
    char *msg = PG_GETARG_CSTRING(2);
    if(v->n_elem < pos)
        elog(ERROR,"posição nao alocada no vetor\n");
    v->array[pos] = CStringGetDatum(msg);
    PG_RETURN_CSTRING("OK!");
}

PG_FUNCTION_INFO_V1(pgm_vetor_cstring_get_elem);
Datum pgm_vetor_cstring_get_elem(PG_FUNCTION_ARGS);

Datum pgm_vetor_cstring_get_elem(PG_FUNCTION_ARGS){
    int pos = PG_GETARG_INT32(1);
    PGM_Vetor_String *v = (PGM_Vetor_String*) PG_GETARG_POINTER(0);
    if(v->n_elem < pos)
        elog(ERROR,"posição nao alocada no vetor\n");
    PG_RETURN_CSTRING(DatumGetCString(v->array[pos]));
}

PG_FUNCTION_INFO_V1(pgm_vetor_cstring_destroy);
Datum pgm_vetor_cstring_destroy(PG_FUNCTION_ARGS);

Datum pgm_vetor_cstring_destroy(PG_FUNCTION_ARGS){
    PGM_Vetor_String *v = (PGM_Vetor_String*) PG_GETARG_POINTER(0);
    pgm_free(v->array);
    pgm_free(v);
    PG_RETURN_CSTRING("OK!");
}

PG_FUNCTION_INFO_V1(pgm_vetor_cstring2array);
Datum pgm_vetor_cstring2array(PG_FUNCTION_ARGS);

Datum pgm_vetor_cstring2array(PG_FUNCTION_ARGS){
    PGM_Vetor_String *v = (PGM_Vetor_String*) PG_GETARG_POINTER(0);
    PG_RETURN_ARRAYTYPE_P(pgm_PGM_Vetor_String2ArrayType(v));
}
