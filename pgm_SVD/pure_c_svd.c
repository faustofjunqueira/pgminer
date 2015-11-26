#include "postgres.h"       //standard
#include "fmgr.h"           //standard
#include "utils/memutils.h"//standard
#include "utils/geo_decls.h"//standard

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdarg.h>
#include <strings.h>
#include <unistd.h>
#include "svdlib.h"
#include <sys/time.h>

#include "../Util/pgm_matriz.h"
#include "../Util/pgm_matriz_util.h"
#include "../Util/pgm_malloc.h"
#include "pure_c_svd.h"

PGM_svdrec* SVDRec2PGM_svdrec( SVDRec rec ) {
  PGM_svdrec* aux = (PGM_svdrec*) palloc( sizeof( PGM_svdrec ) );

  aux->Ut = DMat2PGM_Matriz_Double( rec->Ut );
  aux->S  = pgm_Array2PGM_Vetor_Double( rec->d, rec->S );
  aux->Vt = DMat2PGM_Matriz_Double( rec->Vt );

  return aux;
}

PGM_svdrec* pgm_create_PGM_svdrec(int u_linha, int u_coluna, int s_elem, int v_linha, int v_coluna){
	PGM_svdrec *result = (PGM_svdrec*) pgm_malloc (sizeof(PGM_svdrec));
    result->Ut = pgm_create_matrix_double(u_linha, u_coluna);
    result->S = pgm_create_PGM_Vetor_Double(s_elem);
    result->Vt = pgm_create_matrix_double(v_linha, v_coluna);

	return result;
}

DMat PGM_Matriz_Double2DMat(PGM_Matriz_Double *m){
    int i;
    DMat dense = (DMat) palloc (sizeof(struct dmat));
    dense->rows = m->n_linhas;
    dense->cols = m->n_colunas;
    dense->value = (double**) palloc (sizeof(double*) * dense->rows);

    for( i = 0; i < dense->rows; i++){
        dense->value[i] = PGM_LINHA_MATRIZ(m,i);
    }
    return dense;
}

PGM_Matriz_Double *DMat2PGM_Matriz_Double(DMat dense){
    PGM_Matriz_Double *m = (PGM_Matriz_Double*) palloc (sizeof(PGM_Matriz_Double));
    m->n_linhas = dense->rows;
    m->n_colunas = dense->cols;
    m->valor = dense->value[0];
    return m;
}

PGM_svdrec* pure_c_SVD( PGM_Matriz_Double *m, int numFeatures ) {
  DMat denseMatrix = PGM_Matriz_Double2DMat( m );
  SMat sparseMatrix = svdConvertDtoS( denseMatrix );
  SVDRec rec = svdLAS2A( sparseMatrix, numFeatures );
  PGM_svdrec* pgm_rec = SVDRec2PGM_svdrec( rec );
  svdFreeSMat( sparseMatrix );
  return pgm_rec;
}

PGM_Matriz_Double* pure_c_SVD_matrix_reconstruct( PGM_svdrec* rec, int numFeatures ) {
  PGM_Matriz_Double* m = pgm_create_matrix_double( rec->Ut->n_colunas, rec->Vt->n_colunas  );
  int i, k, j;

  if( numFeatures > rec->S->n_elems )
    numFeatures = rec->S->n_elems;

  for(  k = 0; k < numFeatures; k++ )
    for(  i = 0; i < rec->Ut->n_colunas; i++ )
      for(  j = 0; j < rec->Vt->n_colunas; j++ )
        PGM_ELEM_MATRIZ( m , i, j ) += PGM_ELEM_VETOR( rec->S, k ) * PGM_ELEM_MATRIZ( rec->Vt, k, j ) * PGM_ELEM_MATRIZ( rec->Ut, k, i );

  return m;
}
