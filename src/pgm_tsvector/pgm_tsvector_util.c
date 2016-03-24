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

#include "../Util/pgm_malloc.h"
#include "../Util/pgm_string.h"
#include "../Util/pgm_matriz.h"
#include "../Util/pgm_matriz_util.h"
#include "../Util/pgm_types.h"
#include "pgm_tsvector_util.h"

ArrayType *tsvector_words2arraytype(TSVector in){
    int i = 0;
    char* pt;
    int4         ndims          = 1;
    bool        *isnull         = (bool*) pgm_malloc ( sizeof(bool) * in->size);
    int4         dims[MAXDIM];
    int4         lbs[MAXDIM];
    Oid          IDtipo         = CSTRINGOID;
    int16 elmlen;
    bool elmbyval;
    char elmalign;

    PGM_Vetor_String *buffer = pgm_create_vetor_string(in->size);

    for( i = 0; i < in->size; i++){
        char* aux = (char*) pgm_malloc (sizeof(char)*(in->entries[i].len + 2));
        pt =(char*) (STRPTR(in) + in->entries[i].pos);
        strncpy(aux,pt,(int)in->entries[i].len);
        pgm_charptr_set(buffer,i,aux);
    }

    dims[0] = in->size;
    lbs[0] = 1;

    get_typlenbyvalalign ( IDtipo , &elmlen , &elmbyval , &elmalign);

    return construct_md_array ( buffer->array , isnull , ndims , dims , lbs ,
                                IDtipo ,elmlen , elmbyval , elmalign );
}

PGM_Vetor_String *tsvector_words2pgm_vector_string(TSVector in){
    int i = 0;
    char* pt;

    PGM_Vetor_String *buffer = pgm_create_vetor_string(in->size);

    for( i = 0; i < in->size; i++){
        char* aux = (char*) pgm_malloc (sizeof(char)*(in->entries[i].len + 2));
        pt =(char*) (STRPTR(in) + in->entries[i].pos);
        strncpy(aux,pt,(int)in->entries[i].len);
        pgm_charptr_set(buffer,i,aux);
    }

    return buffer;
}

PGM_Vetor_Int* cria_vetor_presenca(PGM_Vetor_String *vetor, PGM_Vetor_String *_dict){

    PGM_Vetor_Int *result = pgm_create_PGM_Vetor_Int(_dict->n_elem);
    char **dict = pgm_vetor_string2charptr(_dict);
    int i;

    for(i=0; i < vetor->n_elem; i++){
        int k;
        char* elem = pgm_charptr_get_elem(vetor,i);

        for( k = 0; k < _dict->n_elem; k++)
          if( strcmp( elem, dict[k]) == 0 ) {
              result->valor[k] = 1;
              break;
          }
   }
    return result;
}

PGM_Vetor_Int *tsvector_words2frequency(TSVector in, PGM_Vetor_String *_dict){
    PGM_Vetor_Int *result = pgm_create_PGM_Vetor_Int(in->size);
    int i;
    for (i = 0; i < in->size; i++){
        WordEntryPosVector *pt = (WordEntryPosVector*) (STRPTR(in) + SHORTALIGN((in->entries[i]).pos + (in->entries[i]).len));
        result->valor[i] = pt->npos;
     }
    return result;
}

void tsvector2presence_frequency(TSVector vector, PGM_Vetor_String *dict, PGM_Vetor_Int **presence, PGM_Vetor_Int **frequency, int flag){
    int i,j;
    switch(flag){
        case 1: *(presence) = pgm_create_PGM_Vetor_Int(dict->n_elem); break;
        case 2: *(frequency) = pgm_create_PGM_Vetor_Int(dict->n_elem); break;
        case 3: *(presence) = pgm_create_PGM_Vetor_Int(dict->n_elem);
                *(frequency) = pgm_create_PGM_Vetor_Int(dict->n_elem);break;
        default: elog(ERROR, "Flag indisponivel");
    }
    for( i = 0; i < vector->size; i++){
        for(j = 0; j < dict->n_elem; j++){
            char *word =(char*) (STRPTR(vector) + vector->entries[i].pos);
            int len_word = vector->entries[i].len;
            if(strncmp(word,pgm_charptr_get_elem(dict,j), len_word) == 0){
                switch(flag){
                    case 1: (*(presence))->valor[j] = 1;break;
                    case 2: (*(frequency))->valor[j] = ((WordEntryPosVector*) (STRPTR(vector) + SHORTALIGN((vector->entries[i]).pos + (vector->entries[i]).len)))->npos; // frequencia do entries do tsvector;
                            break;
                    case 3: (*(presence))->valor[j] = 1;
                            (*(frequency))->valor[j] = ((WordEntryPosVector*) (STRPTR(vector) + SHORTALIGN((vector->entries[i]).pos + (vector->entries[i]).len)))->npos; // frequencia do entries do tsvector;
                            break;
                }
                break;
            }
        }

    }
}
