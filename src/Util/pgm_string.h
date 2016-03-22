#ifndef PGM_STRING_H_INCLUDED
#define PGM_STRING_H_INCLUDED

#include "postgres.h"       //standard
#include "fmgr.h"           //standard
#include "utils/memutils.h"//standard
#include "utils/geo_decls.h"//standard
#include "catalog/pg_type.h"// OID
#include "utils/array.h"    //arrays
#include "tsearch/ts_type.h" //ts_vector

typedef struct{
    int n_elem;
    Datum *array;

}PGM_Vetor_String;

#define pgm_charptr_get_elem(vetor,pos) ((char*) DatumGetCString(vetor->array[(pos)]))

/*Function: pgm_create_vetor_string
		Cria um PGM_Vetor_String com n_elem posições alocadas
	Parameter:
		int n_elem - numero de posição que serao alocadas
    Return:
        PGM_Vetor_String

 */
PGM_Vetor_String* pgm_create_vetor_string(int n_elem);

/*Function: pgm_charptr_set
		Adiciona uma string (char *) em um PGM_Vetor_String
        Obs: Nao é permitido dar free na area de memoria apontada por elem
	Parameter:
		PGM_Vetor_String *vetor - vetor ao qual será adicionado a string
		String elem - String alocada que será adicionada
		int index - posição no vetor que será adicionado, lembrando que se index for menor que vetor->n_elem dará erro.
    Return:
        int - Se 1 sucesso 0 erro!

 */
void pgm_charptr_set(PGM_Vetor_String *vetor, int index,char* elem);

/*Function: pgm_charptr_realloc
		Redimenciona o vetor.
	Parameter:
		PGM_Vetor_String *vetor - Vetor que sera dimencionado
		int novo_l - novo tamanho
    Return:
        int - Se 1 sucesso 0 erro!
 */
int pgm_charptr_realloc(PGM_Vetor_String* vetor, int novo_l);

/*Function: pgm_vetor_string2charptr
		converte um PGM_Vetor_String para um char**
	Parameter:
		PGM_Vetor_String *vetor - vetor que sera convertido, lembrando que a struct nao devem ser desalocadas
    Return:
        char**
 */
char **pgm_vetor_string2charptr(PGM_Vetor_String *string);

PGM_Vetor_String* pgm_ArrayType2PGM_Vetor_String(ArrayType *array);
ArrayType *pgm_PGM_Vetor_String2ArrayType(PGM_Vetor_String *v);
#endif // PGM_STRING_H_INCLUDED
