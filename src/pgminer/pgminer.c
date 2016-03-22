/* PGM_kmeans
 *
 * Autor: GZS, FFJ
 * GCCBD - COOPE - UFRJ
 * Criacao: 12/04/2012
 * Arquivo concatenas nesse projeto
   -> pgm_kmeans
   -> util
   -> pgm_svd
 */

#include "postgres.h"       //standard
#include "fmgr.h"           //standard
#include "utils/memutils.h"//standard
#include "utils/geo_decls.h"//standard

#define PGMINER

#ifdef PG_MODULE_MAGIC
PG_MODULE_MAGIC;
#endif


PG_FUNCTION_INFO_V1(teste_hstore);
Datum teste_hstore(PG_FUNCTION_ARGS);

Datum teste_hstore(PG_FUNCTION_ARGS){
    char *t = PG_GETARG_CSTRING(0);
    PG_RETURN_CSTRING(t);
}
