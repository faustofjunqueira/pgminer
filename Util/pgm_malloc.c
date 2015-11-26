

#include "postgres.h"//standard
#include "fmgr.h"//standard
#include "utils/geo_decls.h"//standard
#include "pgm_malloc.h"

void* pgm_malloc( size_t size ) {
  void *r;
  if(!PointerIsValid(r = palloc0( size )))
    elog(ERROR, "Nao foi possivel alocar a memoria");
  return r;
}

void* pgm_calloc( size_t num, size_t size ) {
  return palloc0( num * size );
}

void* pgm_realloc( void* ptr, size_t size ) {
  void *aux = palloc( size );

  memcpy( aux, ptr, size );
  pfree( ptr );

  return aux;
}

void  pgm_free( void * ptr ) {
  pfree( ptr );
}

