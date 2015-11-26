/*
 * Autor: GZS, FFJ
 * GCCBD - COOPE - UFRJ
 * Criacao: 12/01/2012
 */
#include "postgres.h"       //standard
#include "fmgr.h"           //standard
#include "utils/geo_decls.h"//standard
#include "utils/array.h"    //arrays
#include "utils/errcodes.h" //warning
#include "catalog/pg_type.h"// OID
#include "utils/lsyscache.h"// construi array type
#include "funcapi.h"        // trabalhar com row
#include "access/heapam.h"  //Construção de Tupla
#include "pgm_matriz.h"
#include "pgm_matriz_util.h"
#include "pgm_malloc.h"
#include <string.h>
#include <assert.h>

typedef struct StMemoryBlock {
  int stackNumber;
  void *block;
}MemoryBlock;

typedef struct StMemorySet {
  int top;
  int capacity;
  int currStackNumber;
  MemoryBlock *mem;
}MemorySet;

MemorySet *getMemorySet( void );
void memorySetMark( void );
void memorySetPush( void *block );
void memorySetRemove( void* block );
int naLista( void* block, int n, void* blocks[] );
void memorySetReleaseAllBut( int n, void* blocks[] );

int in_vetor_int( int valor, PGM_Vetor_Int *vetor ) ;

MemorySet *getMemorySet( void ) {
  static MemorySet* memorySet = NULL;

  if( memorySet == NULL ) {
    memorySet = pgm_malloc( sizeof( MemorySet ) );
    memorySet->top = 0;
    memorySet->capacity = 1024;
    memorySet->currStackNumber = 0;
    memorySet->mem = (MemoryBlock *) pgm_malloc( memorySet->capacity*sizeof( MemoryBlock ) );
  }

  return memorySet;
}

void memorySetMark( void ) {
  getMemorySet()->currStackNumber++;
}

int naLista( void* block, int n, void* blocks[] ) {
  int i ;

  for( i = 0; i < n; i++ )
    if( blocks[i] == block )
      return 1;

  return 0;
}

void memorySetReleaseAllBut( int n, void* blocks[] ) {
  MemorySet *ms = getMemorySet();
  int i;

  elog( INFO, "release: %d", ms->currStackNumber );
  for( i = 0; i < ms->top; i++ )
    if( ms->mem[i].stackNumber == ms->currStackNumber )
      if( !naLista( ms->mem[i].block, n, blocks ) ) {
        pfree( ms->mem[i].block );
        ms->mem[i].block = NULL;
        elog( INFO, "Free: %ld", (long) ms->mem[i].block );
      }

}

void memorySetPush( void *block ) {
  MemorySet *ms = getMemorySet();

  if( ms->top >= ms->capacity ) {
    MemoryBlock *oldMem = ms->mem;
    int oldCapacity = ms->capacity;
    int i;

    ms->capacity *= 2;
    ms->mem = (MemoryBlock *) pgm_malloc( ms->capacity*sizeof( MemoryBlock ) );
    for( i = 0; i < oldCapacity; i++ )
      ms->mem[i] = oldMem[i];
  }

  ms->mem[ms->top].stackNumber = ms->currStackNumber;
  ms->mem[ms->top].block = block;
}

void memorySetRemove( void* block ) {
  MemorySet *ms = getMemorySet();
  int i;

  for( i = 0; i < ms->top; i++ )
    if( ms->mem[i].block == block ) {
      ms->top--;
      ms->mem[i] = ms->mem[ms->top];
    }
}

PGM_Matriz_Int* pgm_create_matrix_int( int n_linhas, int n_colunas ) {
  PGM_Matriz_Int *aux = (PGM_Matriz_Int*) pgm_malloc( sizeof(PGM_Matriz_Int) );

  aux->n_linhas  = n_linhas;
  aux->n_colunas = n_colunas;

  if(!PointerIsValid(  aux->valor = (int *) pgm_malloc( n_linhas * n_colunas * sizeof( int ) ))){
  	elog(ERROR, "Nao foi possivel alocar memoria");
  }

  return aux;
}

void pgm_destroy_matrix_int (PGM_Matriz_Int *matriz){
    pfree(matriz->valor);
    pfree(matriz);
}

PGM_Matriz_Int* pgm_ArrayType2PGM_Matriz_Int( ArrayType *a ){
    PGM_Matriz_Int *aux = (PGM_Matriz_Int*) pgm_malloc ( sizeof(PGM_Matriz_Int));
    assert (ARR_NDIM(a) == 2);
    aux->n_linhas = ARR_DIMS(a)[0];
    aux->n_colunas = ARR_DIMS(a)[1];
    aux->valor      = (int*) ARR_DATA_PTR (a);
    return aux;
}

ArrayType*  pgm_PGM_Matriz_Int2ArrayType( PGM_Matriz_Int* m ){
    int4         ndims          = 2;
    bool        *isnull         = (bool*) pgm_malloc ( sizeof(bool) * m->n_linhas * m->n_colunas);
    int4         dims[MAXDIM];
    int4         lbs[MAXDIM];
    Oid          IDtipo         = INT4OID;
    int16 elmlen; bool elmbyval; char elmalign;
    Datum *vetor = (Datum*) pgm_malloc (sizeof(Datum) * m->n_linhas*m->n_colunas);
    {
        int i;
        for(i=0; i< m->n_linhas*m->n_colunas; i++)
            vetor[i] = Int32GetDatum(m->valor[i]);
    }
    dims[0] = m->n_linhas;
    dims[1] = m->n_colunas;
    lbs[0] = 1;
    lbs[1] = 1;

    get_typlenbyvalalign ( IDtipo , &elmlen , &elmbyval , &elmalign);
    return construct_md_array ( vetor , isnull , ndims , dims , lbs ,
                                IDtipo ,elmlen , elmbyval , elmalign );
}

void pgm_copy_PGM_Vetor_Double_allocated(PGM_Vetor_Double *a, PGM_Vetor_Double *b){

	int i;

	for ( i = 0; i < a->n_elems; i++){
			a->valor[i] = b->valor[i];
	}
}

PGM_Vetor_Double *pgm_copy_PGM_Vetor_Double(PGM_Vetor_Double *a){
	PGM_Vetor_Double *copy = pgm_create_PGM_Vetor_Double(a->n_elems);
	int i;

	for ( i = 0; i < a->n_elems; i++){
			copy->valor[i] = a->valor[i];
	}

	return copy;
}

PGM_Matriz_Float* pgm_create_matrix_float( int n_linhas, int n_colunas ) {
  PGM_Matriz_Float *aux = (PGM_Matriz_Float*) pgm_malloc( sizeof(PGM_Matriz_Float) );

  aux->n_linhas  = n_linhas;
  aux->n_colunas = n_colunas;

  if(!PointerIsValid(  aux->valor = (float *) pgm_malloc( n_linhas * n_colunas * sizeof( float ) ))){
  	elog(ERROR, "Nao foi possivel alocar memoria");
  }

  return aux;
}

void pgm_destroy_matrix_float (PGM_Matriz_Float *matriz){
    pfree(matriz->valor);
    pfree(matriz);
}

PGM_Matriz_Float *pgm_copy_PGM_Matriz_Float(PGM_Matriz_Float *a){
	PGM_Matriz_Float *copy = pgm_create_matrix_float(a->n_linhas, a->n_colunas);
	int i, total = a->n_linhas*a->n_colunas;
	for ( i = 0; i < total; i++ ){
        copy->valor[i] = a->valor[i];
	}

	return copy;
}

PGM_Matriz_Float* pgm_ArrayType2PGM_Matriz_Float( ArrayType *a ){
    PGM_Matriz_Float *aux = (PGM_Matriz_Float*) pgm_malloc ( sizeof(PGM_Matriz_Float));
    assert (ARR_NDIM(a) == 2);
    aux->n_linhas = ARR_DIMS(a)[0];
    aux->n_colunas = ARR_DIMS(a)[1];
    aux->valor      = (float*) ARR_DATA_PTR (a);
    return aux;
}

PGM_Matriz_Double *pgm_copy_PGM_Matriz_Double(PGM_Matriz_Double *a){
	PGM_Matriz_Double *copy = pgm_create_matrix_double(a->n_linhas, a->n_colunas);
	int i, total = a->n_linhas*a->n_colunas;
	for ( i = 0; i < total; i++ ){
			copy->valor[i] = a->valor[i];
	}

	return copy;
}

PGM_Matriz_Double* pgm_create_matrix_double( int n_linhas, int n_colunas ) {
  PGM_Matriz_Double *aux = (PGM_Matriz_Double*) pgm_malloc( sizeof(PGM_Matriz_Double) );

  aux->n_linhas  = n_linhas;
  aux->n_colunas = n_colunas;

  if(!PointerIsValid(  aux->valor = (double *) pgm_malloc( n_linhas * n_colunas * sizeof( double ) ))){
  	elog(ERROR, "Nao foi possivel alocar memoria");
  }

  return aux;
}

PGM_Matriz_Double *matrix_realloc(PGM_Matriz_Double *m, int new_n_row, int new_n_col){
	PGM_Matriz_Double *new_matrix = pgm_create_matrix_double(new_n_row,new_n_col);

	int i,j;

	for (i = 0; i < m->n_linhas; i++)
		for( j = 0; j < m->n_colunas; j++) PGM_ELEM_MATRIZ(new_matrix, i , j) = PGM_ELEM_MATRIZ(m, i, j);
	pgm_destroy_matrix_double(m);

	return new_matrix;
}

void pgm_destroy_matrix_double (PGM_Matriz_Double *matriz){
    pfree(matriz->valor);
    pfree(matriz);
}

void pgm_destroy_PGM_Vetor_Int(PGM_Vetor_Int* vetor){
    pfree(vetor->valor);
    pfree(vetor);
}

PGM_Vetor_Int* pgm_create_PGM_Vetor_Int( int n_elems){
	PGM_Vetor_Int *aux = (PGM_Vetor_Int*) pgm_malloc (sizeof(PGM_Vetor_Int));

	aux->n_elems   = n_elems;

	if(!PointerIsValid(  aux->valor     = ( int *) pgm_malloc( n_elems * sizeof( int ) ))){
  	elog(ERROR, "Nao foi possivel alocar memoria");
  }

    return aux;
}

PGM_Vetor_Double* pgm_create_PGM_Vetor_Double( int n_elems){
	PGM_Vetor_Double *aux = (PGM_Vetor_Double*) pgm_malloc (sizeof(PGM_Vetor_Double));

	aux->n_elems  = n_elems;

 	if(!PointerIsValid(  aux->valor     = ( double *) pgm_malloc( n_elems * sizeof( double ) ))){
  	elog(ERROR, "Nao foi possivel alocar memoria");
  }

    return aux;
}

PGM_Vetor_Double* pgm_Array2PGM_Vetor_Double( int n_elems, double vetor[]) {
    PGM_Vetor_Double *aux = (PGM_Vetor_Double*) palloc (sizeof(PGM_Vetor_Double));

    aux->n_elems  = n_elems;
    aux->valor     = vetor;

    return aux;
}

PGM_Vetor_Int* pgm_Array2PGM_Vetor_Int( int n_elems, int vetor[] ){
    PGM_Vetor_Int *aux = (PGM_Vetor_Int*) palloc (sizeof(PGM_Vetor_Int));

    aux->n_elems  = n_elems;
    aux->valor     = vetor;

    return aux;
}

PGM_Matriz_Double* pgm_ArrayType2PGM_Matriz_Double( ArrayType *a ){
    PGM_Matriz_Double *aux = (PGM_Matriz_Double*) palloc0 ( sizeof(PGM_Matriz_Double));

    if( ARR_NDIM(a) == 2 ) {
      aux->n_linhas = ARR_DIMS(a)[0];
      aux->n_colunas = ARR_DIMS(a)[1];
      aux->valor      = (double*) ARR_DATA_PTR (a);
    }

    return aux;
}

PGM_Vetor_Int* pgm_ArrayType2PGM_Vetor_Int( ArrayType *a ){
    PGM_Vetor_Int *aux = (PGM_Vetor_Int*) palloc0 ( sizeof(PGM_Vetor_Int));

    if( ARR_NDIM(a) == 1 ) {
      aux->n_elems = ARR_DIMS(a)[0];
      aux->valor = (int*) ARR_DATA_PTR (a);
    }

    return aux;
}

PGM_Vetor_Double* pgm_ArrayType2PGM_Vetor_Double( ArrayType *a ){
    PGM_Vetor_Double *aux = (PGM_Vetor_Double*) palloc0 ( sizeof(PGM_Vetor_Double));
    if( ARR_NDIM(a) == 1) {
      aux->n_elems = ARR_DIMS(a)[0];
      aux->valor      = (double*) ARR_DATA_PTR (a);
    }
    return aux;
}

ArrayType*  pgm_PGM_Matriz_Double2ArrayType( PGM_Matriz_Double* m ){
    int4         ndims          = 2;
    bool        *isnull         = (bool*) pgm_malloc ( sizeof(bool) * m->n_linhas * m->n_colunas);
    int4         dims[MAXDIM];
    int4         lbs[MAXDIM];
    Oid          IDtipo         = FLOAT8OID;
    int16 elmlen; bool elmbyval; char elmalign;

    dims[0] = m->n_linhas;
    dims[1] = m->n_colunas;
    lbs[0] = 1;
    lbs[1] = 1;

    get_typlenbyvalalign ( IDtipo , &elmlen , &elmbyval , &elmalign);
    return construct_md_array ((Datum*) m->valor , isnull , ndims , dims , lbs ,
                                IDtipo ,elmlen , elmbyval , elmalign );
}

ArrayType*  pgm_PGM_Matriz_Float2ArrayType( PGM_Matriz_Float* m ){
    int4         ndims          = 2;
    bool        *isnull         = (bool*) pgm_malloc ( sizeof(bool) * m->n_linhas * m->n_colunas);
    int4         dims[MAXDIM];
    int4         lbs[MAXDIM];
    Oid          IDtipo         = FLOAT4OID;
    int16 elmlen; bool elmbyval; char elmalign;
    Datum *vetor = (Datum*) pgm_malloc (sizeof(Datum) * m->n_linhas*m->n_colunas);
    {
        int i;
        for(i=0; i< m->n_linhas*m->n_colunas; i++)
            vetor[i] = Float4GetDatum(m->valor[i]);
    }
    dims[0] = m->n_linhas;
    dims[1] = m->n_colunas;
    lbs[0] = 1;
    lbs[1] = 1;

    get_typlenbyvalalign ( IDtipo , &elmlen , &elmbyval , &elmalign);
    return construct_md_array ( vetor , isnull , ndims , dims , lbs ,
                                IDtipo ,elmlen , elmbyval , elmalign );
}

ArrayType* pgm_PGM_Vetor_Int2ArrayType( PGM_Vetor_Int* v ){
    int4         ndims          = 1;
    bool        *isnull         = (bool*) pgm_malloc ( sizeof(bool) * v->n_elems );
    int4         dims[MAXDIM];
    int4         lbs[MAXDIM];
    Oid          IDtipo         = INT4OID;
    int16 elmlen; bool elmbyval; char elmalign;
    Datum *vetor = (Datum*) palloc (sizeof(Datum) * v->n_elems);
    {
        int i;
        for(i=0; i<v->n_elems; i++)
            vetor[i] = Int32GetDatum(v->valor[i]);
    }

    dims[0] = v->n_elems;
    lbs[0] = 1;

    get_typlenbyvalalign ( IDtipo , &elmlen , &elmbyval , &elmalign);

    return construct_md_array ( vetor, isnull, ndims, dims, lbs,
                                IDtipo,elmlen, elmbyval, elmalign );
}

ArrayType* pgm_PGM_Vetor_Double2ArrayType( PGM_Vetor_Double* v ){
    int4         ndims          = 1;
    bool        *isnull         = (bool*) pgm_malloc ( sizeof(bool)*v->n_elems );
    int4         dims[MAXDIM];
    int4         lbs[MAXDIM];
    Oid          IDtipo         = FLOAT8OID;
    int16 elmlen; bool elmbyval; char elmalign;

    dims[0] = v->n_elems;
    lbs[0] = 1;

    get_typlenbyvalalign ( IDtipo , &elmlen , &elmbyval , &elmalign);
    return construct_md_array ((Datum*) v->valor, isnull , ndims , dims , lbs ,
                                IDtipo ,elmlen , elmbyval , elmalign );
}

void pgm_destroy_PGM_Vetor_Double(PGM_Vetor_Double* vetor){
    pfree(vetor->valor);
    pfree(vetor);
}

PGM_Matriz_Double *matrix_transpose2(PGM_Matriz_Double *trans_b, int line, int col){
	PGM_Matriz_Double *x = pgm_create_matrix_double(line, col);
	int i, j;
	for( i=0; i < x->n_linhas; i++){
		for( j = 0; j < x->n_colunas; j++){
			PGM_ELEM_MATRIZ(x, i, j) = PGM_ELEM_MATRIZ(trans_b, j, i);
		}
	}
	return x;
}

PGM_Matriz_Double *matrix_transpose(PGM_Matriz_Double *m){
	int i, j;
	PGM_Matriz_Double *transpose = pgm_create_matrix_double(m->n_colunas, m->n_linhas);
	for( i = 0; i < m->n_linhas; i++)
		for( j = 0; j < m->n_colunas; j++)
			PGM_ELEM_MATRIZ(transpose, j,i) = PGM_ELEM_MATRIZ(m, i,j);

	return transpose;
}

double get_min_term(PGM_Matriz_Double *m){
	int size = m->n_linhas * m->n_colunas;
	double min = m->valor[0];
	int i;
	for(i=1; i < size; i++){
		if(min > m->valor[i]) min = m->valor[i];
	}
	return min;
}

double get_max_term(PGM_Matriz_Double *m){
	int size = m->n_linhas * m->n_colunas;
	double max = m->valor[0];
	int i;
	for(i=1; i < size; i++){
		if(max < m->valor[i]) max = m->valor[i];
	}
	return max;
}

double get_matrix_means(PGM_Matriz_Double *m){
	int size = m->n_linhas * m->n_colunas;
	double sum = 0;
	int i;
	for(i=0; i < size; i++){
		sum += m->valor[i];
	}
	return sum/size;
}

void matrix_apply_scale(PGM_Matriz_Double *matrix, double f){
	int i, n = matrix->n_linhas*matrix->n_colunas;
	for (i = 0; i < n; i++) matrix->valor[i] *= f;

}

void matrix_set_col_value(PGM_Matriz_Double *matrix, int column, double value){
	if( column > matrix->n_colunas) elog(ERROR, "matrix_set_col_value: Nao existe a coluna %d na matriz.(coluna <= %d)", column, matrix->n_colunas);
	else{
		int i;
		for ( i = 0; i < matrix->n_linhas; i++) PGM_ELEM_MATRIZ(matrix, i, column) = value;
	}
}

void matrix_set_line_value(PGM_Matriz_Double *matrix, int line, double value){
	if( line > matrix->n_linhas) elog(ERROR, "matrix_set_line_value: Nao existe a linha %d na matriz.(linha <= %d)", line, matrix->n_linhas);
	else{
		int i;
		for ( i = 0; i < matrix->n_colunas; i++) PGM_ELEM_MATRIZ(matrix, line, i) = value;
	}
}

int in_vetor_int( int valor, PGM_Vetor_Int *vetor ) {
  int i ;

  for( i = 0; i < vetor->n_elems; i++ )
    if( vetor->valor[i] == valor )
      return 1;

  return 0;
}

PGM_Matriz_Double *matrix_remove_line_col( PGM_Matriz_Double *matrix, PGM_Vetor_Int *linhas, PGM_Vetor_Int *colunas ) {
  PGM_Matriz_Double *res = pgm_create_matrix_double( matrix->n_linhas - linhas->n_elems, matrix->n_colunas - colunas->n_elems );
  int i, j, ni, nj;

  for( i = ni = 0; i < matrix->n_linhas; i++ )
    if( !in_vetor_int( i, linhas ) ) {
      for( j = nj = 0; j < matrix->n_colunas; j++ )
        if( !in_vetor_int( j, colunas ) ) {
          PGM_ELEM_MATRIZ( res, ni, nj ) = PGM_ELEM_MATRIZ( matrix, i, j );

          nj++;
        }

      ni++;
    }

    return res;
}

