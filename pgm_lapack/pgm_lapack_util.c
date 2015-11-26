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
#include <sys/time.h>

#include "../Util/pgm_matriz.h"
#include "../Util/pgm_matriz_util.h"
#include "../Util/pgm_malloc.h"
#include "../pgm_SVD/pure_c_svd.h"
#include "../pgm_math/pgm_math_util.h"
#include "pgm_lapack_util.h"

#include "lapacke.h"

PGM_svdrec* lapack_svd(PGM_Matriz_Double* matrix){

	int numberOfSingularValues = matrix->n_linhas < matrix->n_colunas ? matrix->n_linhas : matrix->n_colunas;//pega a menor dimenção da matrix
	PGM_svdrec* result = pgm_create_PGM_svdrec(matrix->n_linhas, matrix->n_linhas, numberOfSingularValues, matrix->n_colunas, matrix->n_colunas);
	int lwork = 3*min(matrix->n_linhas,matrix->n_colunas) + max(max(matrix->n_linhas,matrix->n_colunas),4*min(matrix->n_linhas,matrix->n_colunas)*min(matrix->n_linhas,matrix->n_colunas)+3*min(matrix->n_linhas,matrix->n_colunas)+max(matrix->n_linhas,matrix->n_colunas));
	int *iwork = (int*) pgm_malloc(8*numberOfSingularValues);
	double *work = (double*) pgm_malloc(lwork * sizeof(double));
	int info = 0;
    elog(INFO,"Aki1");
	dgesdd_("A", &matrix->n_linhas, &matrix->n_colunas, matrix->valor, &matrix->n_linhas, result->S->valor, result->Ut->valor, &matrix->n_linhas, result->Vt->valor, &matrix->n_colunas, work, &lwork, iwork, &info);
	elog(INFO,"Aki1");
	if (info < 0){
		elog(ERROR, "Argumento %d incorreto", -(info));
		return NULL;
	}else if(info > 0){
		elog(ERROR, "DBDSDC did not converge, updating process failed.");
		return NULL;
	}

	pgm_free(work);
	pgm_free(iwork);

	return result;
}


int ordinary_least_squares(PGM_Matriz_Double *a, PGM_Matriz_Double *b , OUT_PGM_Vetor_Double *s){
	int info, lwork, rank;
	double rcond = -1.0;
	double wkopt;
	double* work;

	int *iwork = (int*) malloc (sizeof(int)*(3*a->n_colunas*0+11*a->n_colunas));

	lwork = -1;

	dgelsd_( &a->n_colunas, &a->n_linhas, &b->n_linhas, a->valor, &a->n_colunas, b->valor, &b->n_colunas, s->valor, &rcond, &rank, &wkopt, &lwork,iwork, &info );
	lwork = (int)wkopt;
	work = (double*)malloc( lwork*sizeof(double) );
	dgelsd_( &a->n_colunas, &a->n_linhas, &b->n_linhas, a->valor, &a->n_colunas, b->valor, &b->n_colunas, s->valor, &rcond, &rank, work, &lwork,iwork, &info );
	if( info > 0 ) {
		elog(WARNING,"The algorithm computing SVD failed to converge;\n" );
		elog(ERROR,"the least squares solution could not be computed.\n" );
	}
	return rank;
}


void matrix_inverse(double* A, int N){
	int *IPIV = (int*) pgm_malloc(sizeof(int)*(N+1));
	int LWORK = N*N;
	double *WORK = (double*) pgm_malloc(sizeof(double) * LWORK);
	int information;

	dgetrf_(&N,&N,A,&N,IPIV,&information);

	if(information != 0){
		printf("Nao tem inversa\n");
		return;
	}

	dgetri_(&N,A,&N,IPIV,WORK,&LWORK,&information);

	if(information != 0){
		printf("Nao tem inversa\n");
		return;
	}

	if(information > 0) elog( ERROR, "Matrix is not invertible" );

}
