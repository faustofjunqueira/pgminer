/*
 * Autor: GZS, FFJ
 * GCCBD - COOPE - UFRJ
 * Criacao: 12/01/2012
 */
#include "postgres.h"
#include "fmgr.h"
#include "utils/geo_decls.h"
#include "utils/errcodes.h" //warning
#include <math.h>
#include "../Util/pgm_matriz.h"
#include "../Util/pgm_matriz_util.h"
#include "pure_c_kmeans.h"

#define PGM_SQRDIFLINHA( resultado, matriz_a, linha_a, matriz_b, linha_b ) \
  { \
    double* md = PGM_LINHA_MATRIZ ( matriz_a , linha_a ); \
    double* mo = PGM_LINHA_MATRIZ ( matriz_b , linha_b ); \
    int i, tamanho = (matriz_a)->n_colunas; \
    resultado = 0; \
    for ( i = 0 ; i < tamanho; i++) { \
      double aux = md[i] - mo[i]; \
      resultado += aux * aux; \
    } \
  }


int change_group( PGM_Matriz_Double *centroides, PGM_Matriz_Double *matriz, int *grupo,
                 int ponto, double *somatorio_distancias ){
    int i, novo_grupo = 0;
    double menor_distancia, distancia_atual;

    PGM_SQRDIFLINHA( menor_distancia, centroides, 0, matriz, ponto);

    for ( i = 1; i < centroides->n_linhas; i++){
        PGM_SQRDIFLINHA(distancia_atual, centroides, i, matriz, ponto);
        if ( menor_distancia > distancia_atual){
            menor_distancia = distancia_atual;
            novo_grupo = i;
        }
    }

    *somatorio_distancias += menor_distancia;

    if( grupo[ponto] != novo_grupo ) {
        grupo[ponto] = novo_grupo;
        return 1;
    }
    else
        return 0;
}

int search_empty_group( int *n_elem , int k){
    int i;
    int resultado = 0;
    for(i=0; i<k; i++)
        if( n_elem[i] == 0)
            resultado = i;
    return resultado;
}

double *search_point_away(PGM_Matriz_Double *matriz){
    PGM_Matriz_Double *ponto_zero = pgm_create_matrix_double( 1, matriz->n_colunas );

    double maior_distancia = 0, distancia_atual = 0, *ponto_mais_longe = PGM_LINHA_MATRIZ(matriz, 0);

    PGM_SQRDIFLINHA(maior_distancia, ponto_zero, 0, matriz, 0);
    {
        int i;
        for( i = 1; i < matriz->n_linhas; i++){
            PGM_SQRDIFLINHA(distancia_atual, ponto_zero, 0, matriz, i);
            if (maior_distancia < distancia_atual){
                maior_distancia = distancia_atual;
                ponto_mais_longe = PGM_LINHA_MATRIZ(matriz, i);
            }
        }
    }
    pgm_destroy_matrix_double(ponto_zero);
    return ponto_mais_longe;

}

int calculate_centroid ( PGM_Matriz_Double *centroides , PGM_Matriz_Double *matriz , int *grupo , int *n_elem_grupo ){
    int i, n_grupos = 0;

    memset ( centroides->valor, 0, sizeof(double)*centroides->n_linhas*centroides->n_colunas);
    memset ( n_elem_grupo , 0 , sizeof(int) * centroides->n_linhas);

    for( i = 0; i < matriz->n_linhas; i++){
        PGM_SOMALINHA_MATRIZ ( centroides, grupo[i], matriz, i);
        n_elem_grupo[grupo[i]]++;
    }

    for ( i = 0; i < centroides->n_linhas; i++)
			if (n_elem_grupo[i] > 0) {
        int j;
        double *dimensao = PGM_LINHA_MATRIZ( centroides , i);
        n_grupos++;
        for ( j = 0; j < centroides->n_colunas; j++)
            dimensao[j] /= n_elem_grupo[i];
			}

    return n_grupos;
}

void init_group(PGM_Matriz_Double *centroides , PGM_Matriz_Double *matriz , int *grupo , int *n_elem_grupo, int k, int semente){
    switch ( semente ){
        //semente = 1 - para sortear k pontos para serem os centros dos grupos
        case 1: {
                  int i;
                  for( i = 0; i < k; i++ ) {
                    int linha;
                    do {
                      linha = rand() % matriz->n_linhas;
                    } while( grupo[linha]==1 );

                    grupo[linha] = 1;

                    PGM_COPIALINHA_MATRIZ( centroides, i, matriz, linha );

                  }
                }
                break;
        //semente = 2 - para sortear cada ponto em um grupo
        case 2: {
                  int i;

                  for( i = 0; i < matriz->n_linhas; i++ )
                    grupo[i] = rand() % k;

                  calculate_centroid( centroides, matriz, grupo, n_elem_grupo);
                }
                break;
    }
    memset(grupo, 0, sizeof(int)*matriz->n_linhas);
}

int create_new_group (PGM_Matriz_Double *centroides, PGM_Matriz_Double *matriz, int grupo_vazio){
    int i;
    double *centro_vazio = PGM_LINHA_MATRIZ(centroides, grupo_vazio),
           *novo_centro = search_point_away(matriz);

    for (i=0; i<centroides->n_colunas; i++)
        centro_vazio[i] = novo_centro[i];

    return 1;
}


int pure_c_kmeans( PGM_Matriz_Double *matriz, int k, int semente, int fim,
                   PGM_Matriz_Double *centroides, int *grupo, double *somatorio_distancias ) {
    int alterou = 1;
    int *n_elem_grupo = (int*) palloc0 ( sizeof(int) * k);

    init_group(centroides, matriz, grupo, n_elem_grupo, k, semente);
    while( alterou ){
        int i;
        alterou = 0;
        *somatorio_distancias = 0;

        for( i = 0; i < matriz->n_linhas; i++)
            alterou += change_group(centroides, matriz, grupo, i, somatorio_distancias);

        if(calculate_centroid(centroides, matriz, grupo, n_elem_grupo) < k){

            switch(fim){
                case 1:
                    break;

                case 2:
                    alterou += create_new_group(centroides, matriz, search_empty_group(n_elem_grupo, k));
                    break;

                case 3:
                    return 1;
            }
        }
    }
    pfree (n_elem_grupo);
    return 0;
}

