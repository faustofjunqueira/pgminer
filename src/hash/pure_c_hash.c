#include "pure_c_hash.h"

#include "../Util/pgm_malloc.h"
#include "../Util/pgm_matriz_util.h"

PGM_IntHashSet *pgm_int_hashset_init(){
	PGM_IntHashSet *st = NULL;
	return st;
}

char pgm_int_hashset_find(PGM_IntHashSet *hash_table, int id){
	PGM_IntHashSet *output_pointer;
	HASH_FIND_INT( hash_table, &id, output_pointer );
	return output_pointer ? 1 : 0;
}

void pgm_int_hashset_add(PGM_IntHashSet **hash_table, int id){
	if (!pgm_int_hashset_find(*hash_table,id)){
	  PGM_IntHashSet *input = (PGM_IntHashSet*) pgm_malloc(sizeof(PGM_IntHashSet));
	  input->id = id;
	  HASH_ADD_INT( *hash_table, id, input );
	}
}

PGM_IntHashSet *pgm_int_hashset_add_vector(PGM_Vetor_Int *id){
	PGM_IntHashSet *hash_table = pgm_int_hashset_init();
	int i;
	for(i = 0; i < id->n_elems; i++){
		pgm_int_hashset_add(&hash_table,PGM_ELEM_VETOR(id,i));
	}
	return hash_table;
}

PGM_Vetor_Int *PGM_IntHashSet22PGM_Vetor_Int(PGM_IntHashSet *hash_table){
    PGM_IntHashSet *st = NULL;
    unsigned int num_size = HASH_COUNT(hash_table);
    PGM_Vetor_Int *vector = pgm_create_PGM_Vetor_Int(num_size);
    int i;
    for(i = 0, st = hash_table; i < num_size; i++, st = st->hh.next){
        PGM_ELEM_VETOR(vector,i) = st->id;
    }
    return vector;
}
