#include "postgres.h"
#include "fmgr.h"
#include "utils/geo_decls.h"
#include "utils/errcodes.h" //warning
#include <math.h>
#include "../Util/pgm_matriz.h"
#include "../Util/pgm_matriz_util.h"
#include "pgm_math_util.h"

void compute_stats( double v[], double min, double max, double sum_x, double sum_x2, double count, double count_nz );


void  center_matrix( PGM_Matriz_Double *m, PGM_Vetor_Double *v ) {
    int i, j;

    for( i = 0; i < m->n_linhas; i++ )
        for( j = 0; j < m->n_colunas; j++ )
            PGM_ELEM_MATRIZ( m, i, j ) -= PGM_ELEM_VETOR( v, j );
}


// MIN, MAX, SUM_X, SUM_X2, COUNT, AVG, STDDEV_SAMP, STDDEV_POP, COUNT_NZ, AVG_NZ, STDDEV_SAMP_NZ, STDDEV_POP_NZ
void compute_stats( double v[], double min, double max, double sum_x, double sum_x2, double count, double count_nz ) {
    v[0] = min;
    v[1] = max;
    v[2] = sum_x;
    v[3] = sum_x2;
    v[4] = count;
    v[5] = sum_x/count;
    v[6] = (count > 1 ? sqrt((count*sum_x2-sum_x*sum_x)/(count*(count-1))) : 0 );
    v[7] = (count > 1 ? sqrt((count*sum_x2-sum_x*sum_x)/(count*count)) : 0 );
    v[8] = count_nz;
    v[9] = (count_nz > 1 ? sum_x/count_nz : 0 );
    v[10] = (count_nz > 2 ? sqrt((count_nz*sum_x2-sum_x*sum_x)/(count_nz*(count_nz-1))) : 0 );
    v[11] = (count_nz > 1 ? sqrt((count_nz*sum_x2-sum_x*sum_x)/(count_nz*count_nz)) : 0 );
}

void matrix_stats(  PGM_Matriz_Double* A, PGM_Matriz_Double **L, PGM_Matriz_Double **C, PGM_Matriz_Double **M ) {
    int i, j;
    *L = pgm_create_matrix_double( A->n_linhas, 12 );
    *C = pgm_create_matrix_double( A->n_colunas, 12 );
    *M = pgm_create_matrix_double( 1, 12 );

    for( i = 0; i < A->n_linhas; i++ ) {
        double x = PGM_ELEM_MATRIZ( A, i, 0 );
        double min = x;
        double max = x;
        double sum_x = x;
        double sum_x2 = x*x;
        double count = A->n_colunas;
        double count_nz = (x == 0 ? 0 : 1);
        double v[12] = { 0 };

        for( j = 1; j < A->n_colunas; j++ ){
            x = PGM_ELEM_MATRIZ( A, i, j );
            if( x < min ) min = x;
            if( x > max ) max = x;
            sum_x += x;
            sum_x2 += x*x;
            if( x != 0 ) count_nz++;
        }

        compute_stats( v, min, max, sum_x, sum_x2, count, count_nz );

        for( j = 0; j < 12; j++ )
          PGM_SET_ELEM_MATRIZ( *L, i, j, v[j] );
    }

    for( i = 0; i < A->n_colunas; i++ ) {
        double x = PGM_ELEM_MATRIZ( A, 0, i );
        double min = x;
        double max = x;
        double sum_x = x;
        double sum_x2 = x*x;
        double count_nz = (x == 0 ? 0 : 1);
        double count = A->n_linhas;
        double v[12] = { 0 };

        for( j = 1; j < A->n_linhas; j++ ){
            x = PGM_ELEM_MATRIZ( A, j, i );
            if( x < min ) min = x;
            if( x > max ) max = x;
            sum_x += x;
            sum_x2 += x*x;
            if( x != 0 ) count_nz++;
        }

        compute_stats( v, min, max, sum_x, sum_x2, count, count_nz );

        for( j = 0; j < 12; j++ )
          PGM_SET_ELEM_MATRIZ( *C, i, j, v[j] );
    }

    {
        double x = PGM_ELEM_MATRIZ( A, 0, 0 );
        double min = x;
        double max = x;
        double sum_x = 0;
        double sum_x2 = 0;
        double count_nz = 0;
        double count = A->n_linhas * A->n_colunas;
        double v[12] = { 0 };

        for( i = 0; i < A->n_linhas; i++ )
            for( j = 0; j < A->n_colunas; j++ ) {
                x = PGM_ELEM_MATRIZ( A, i, j );
                if( x < min ) min = x;
                if( x > max ) max = x;
                sum_x += x;
                sum_x2 += x*x;
                if( x != 0 ) count_nz++;
        }

        compute_stats( v, min, max, sum_x, sum_x2, count, count_nz );

        for( j = 0; j < 12; j++ )
          PGM_SET_ELEM_MATRIZ( *M, 0, j, v[j] );
    }

}

void lsh_cos( PGM_Matriz_Double* A, PGM_Matriz_Double* B, PGM_Matriz_Double *c ){
    PGM_Vetor_Double *A2 = pgm_create_PGM_Vetor_Double( A->n_linhas );
    PGM_Vetor_Double *B2 = pgm_create_PGM_Vetor_Double( B->n_colunas );
    int i, j;

    for ( i = 0; i < A->n_linhas; i++)
        for ( j = 0; j < A->n_colunas; j++)
            A2->valor[i] += PGM_ELEM_MATRIZ(A, i, j )*PGM_ELEM_MATRIZ(A, i, j);

    for ( i = 0; i < B->n_linhas; i++)
        for ( j = 0; j < B->n_colunas; j++)
            B2->valor[j] += PGM_ELEM_MATRIZ(B, i, j )*PGM_ELEM_MATRIZ(B, i, j);

    for( i = 0; i < c->n_linhas; i++ )
        for( j = 0; j < c->n_colunas; j++ )
            PGM_ELEM_MATRIZ(c, i, j) /= sqrt( A2->valor[i] * B2->valor[j] );

    pgm_destroy_PGM_Vetor_Double( A2 );
    pgm_destroy_PGM_Vetor_Double( B2 );

}


PGM_Vetor_Double *cos_ndimensional (PGM_Matriz_Double* A, PGM_Matriz_Double* B){
	PGM_Vetor_Double *cos_result = pgm_create_PGM_Vetor_Double(A->n_linhas);

	int i, j;
	for ( i = 0; i < A->n_linhas; i++){
    	double Sab = 0, Sa2 = 0, Sb2 = 0;
		for( j = 0; j < A->n_colunas; j++){
			Sab += PGM_ELEM_MATRIZ(A, i, j)*PGM_ELEM_MATRIZ(B, i, j);
			Sa2 += PGM_ELEM_MATRIZ(A, i, j)*PGM_ELEM_MATRIZ(A, i, j);
			Sb2 += PGM_ELEM_MATRIZ(B, i, j)*PGM_ELEM_MATRIZ(B, i, j);
		}
		cos_result->valor[i] = Sab/sqrt(Sa2*Sb2);
	}
	return cos_result;
}

int max(int a, int b){
	return a >= b ? a : b;
}

int min(int a, int b){
	return a <= b ? a : b;
}

PGM_Matriz_Double *weighted_mean(PGM_Matriz_Double *A, double p,PGM_Matriz_Double *B, double q){
	PGM_Matriz_Double *means = pgm_create_matrix_double(A->n_linhas, A->n_colunas);
	int i, j;
	double div = p + q;
	for ( i = 0; i < A->n_linhas; i++){
		for( j = 0; j < A->n_colunas; j++){
			PGM_ELEM_MATRIZ(means, i, j) = (PGM_ELEM_MATRIZ(A, i, j)*p + PGM_ELEM_MATRIZ(B, i, j)*q)/div;
		}
	}
	return means;
}

double cos_vetorial (PGM_Vetor_Double *A,PGM_Vetor_Double *B){
    double prod_escalar = 0;
    double soma_quadA = 0,soma_quadB = 0;
    int i;
    if ( A->n_elems != B->n_elems)
        elog(ERROR,"As dimensões são diferentes ( %d - %d", A->n_elems, B->n_elems);

    for ( i = 0; i < A->n_elems; i++){
        prod_escalar += A->valor[i]*B->valor[i];
        soma_quadA += A->valor[i]*A->valor[i];
        soma_quadB += B->valor[i]*B->valor[i];
    }

    return (soma_quadA == 0 || soma_quadB == 0)? 0 : prod_escalar/(sqrt(soma_quadA)*sqrt(soma_quadB));
}




