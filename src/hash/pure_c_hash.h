#ifndef PURE_C_HASH_H
#define PURE_C_HASH_H

#include "uthash.h"
#include "../Util/pgm_matriz_util.h"

typedef struct{
	int id;
	UT_hash_handle hh;
}PGM_IntHashSet;

PGM_IntHashSet *pgm_int_hashset_init();
char pgm_int_hashset_find(PGM_IntHashSet *hash_table, int id);
void pgm_int_hashset_add(PGM_IntHashSet **hash_table, int id);
PGM_IntHashSet *pgm_int_hashset_add_vector(PGM_Vetor_Int *id);
PGM_IntHashSet *pgm_int_hashset_init();
PGM_Vetor_Int *PGM_IntHashSet22PGM_Vetor_Int(PGM_IntHashSet *hash_table);

#endif
