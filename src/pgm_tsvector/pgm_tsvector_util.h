#ifndef PGM_TSVECTOR_UTIL_H_INCLUDED
#define PGM_TSVECTOR_UTIL_H_INCLUDED

#include "postgres.h"
#include "utils/array.h"
#include "catalog/pg_type.h"// OID
#include "utils/lsyscache.h"// construi array type
#include "tsearch/ts_type.h" //ts_vector

#include "../Util/pgm_string.h"

/*Function: tsvector_words2arraytype
		Captura os tokens do TSVector e converte num ArrayType
	Parameter:
		TSVector - TSVector contendo os tokens
    Return:
        ArrayType

 */
ArrayType *tsvector_words2arraytype(TSVector in);

/*Function: tsvector_words2pgm_vector_string
		Captura os tokens do TSVector e converte num PGM_Vetor_String
	Parameter:
		TSVector - TSVector contendo os tokens
    Return:
        PGM_Vetor_String

 */
PGM_Vetor_String *tsvector_words2pgm_vector_string(TSVector in);

/*Function: cria_vetor_presenca
		Cria um vetor booleano com as presenças dos termos passados pelo vetor no dicionario.
	Parameter:
		PGM_Vetor_String *vetor - Vetor com as palavras a serem comparadas pelo dicionario
		PGM_Vetor_String *_dict - O dicionario
    Return:
        PGM_Vetor_Int
 */
PGM_Vetor_Int* cria_vetor_presenca(PGM_Vetor_String *vetor, PGM_Vetor_String *_dict);


PGM_Vetor_Int *tsvector_words2frequency(TSVector in, PGM_Vetor_String *_dict);

void tsvector2presence_frequency(TSVector vector, PGM_Vetor_String *dict, PGM_Vetor_Int **presence, PGM_Vetor_Int **frequency, int flag);
#endif // PGM_TSVECTOR_UTIL_H_INCLUDED
