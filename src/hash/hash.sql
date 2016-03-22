CREATE OR REPLACE FUNCTION pgm_inthashset_pertence(hash_table bigint, id integer)
  RETURNS boolean AS
'$libdir/pgminer.so', 'PGM_Int_Hashset_Find'
  LANGUAGE c VOLATILE STRICT
  COST 1;

-- drop FUNCTION pgm_inthashset_insere(hash_table bigint, id integer)
CREATE OR REPLACE FUNCTION pgm_inthashset_insere( id integer, hash_table bigint default 0)
  RETURNS bigint AS
'$libdir/pgminer.so', 'PGM_Int_Hashset_Add'
  LANGUAGE c VOLATILE STRICT
  COST 1;

CREATE OR REPLACE FUNCTION pgm_inthashset_insere(vector bigint)
  RETURNS bigint AS
'$libdir/pgminer.so', 'PGM_Int_Hashset_Add_Vector'
  LANGUAGE c VOLATILE STRICT
  COST 1;

CREATE OR REPLACE FUNCTION pgm_hash2vector_int(hashtable bigint)
  RETURNS bigint AS
'$libdir/pgminer.so', 'pgm_PGM_IntHashSet22PGM_Vetor_Int'
  LANGUAGE c VOLATILE STRICT
  COST 1;

----------------------------- TESTE --------------------------------------------------
begin;

select pgm_inthashset_insere(16);
-- hash: 140232765440336
select pgm_hash2vector_int(140232765440368)
-- vetor 140232765441024
select pgm_vector_int2array(pgm_hash2vector_int(140232767161200))

select pgm_inthashset_insere(5,140232765440336);

select pgm_inthashset_insere(pgm_array2vector_int(array[5,9,7,8,4,3,55]))



