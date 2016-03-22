/*
 * Autor: GZS, FFJ
 * GCCBD - COOPE - UFRJ
 * Criacao: 12/01/2012
 */
#ifndef PGM_MATRIZ_UTIL_H
#define PGM_MATRIZ_UTIL_H

#include "postgres.h"
#include "utils/array.h"
#include "catalog/pg_type.h"// OID
#include "utils/lsyscache.h"// construi array type
#include "pgm_matriz.h"

#define printMatrix(m){\
	int i, j;\
	char *buffer = (char*) pgm_malloc(sizeof(char) * m->n_linhas * m->n_colunas * 20);\
	char *temp = (char*) pgm_malloc (sizeof(char) * 15);\
	memset(buffer, 0, sizeof(char) * m->n_linhas * m->n_colunas * 20);\
	for ( i=0 ; i < m->n_linhas; i++){\
		for( j = 0; j < m->n_colunas; j++){\
			sprintf(temp, "%lf - ", PGM_ELEM_MATRIZ(m,i,j));\
			strcat(buffer,temp);\
			memset(temp, 0, sizeof(char)*20);\
		}\
		strcat(buffer, "\n");\
	}\
	elog(INFO, "%s", buffer);\
}\

typedef PGM_Vetor_Double OUT_PGM_Vetor_Double;

/*Function: pgm_copy_PGM_Vetor_Double_allocated
		Copia dos valores dos <PGM_Vetor_Double> de b para a.

	Parameter:
		PGM_Vetor_Double *a - destino.
		PGM_Vetor_Double *b - origem.

 */
void pgm_copy_PGM_Vetor_Double_allocated(PGM_Vetor_Double *a, PGM_Vetor_Double *b);

/*Function: pgm_create_matrix_float
		Cria uma <PGM_Matriz_Float> com o numero de linhas e colunadas declarada.

	Parameter:
		int n_linhas - numero de linhas da matriz.
		int n_colunas - numero de colunas da matriz.
	Return:
		PGM_Matriz_Float* - Ponteiro para a matriz criada.
 */
PGM_Matriz_Float* pgm_create_matrix_float( int n_linhas, int n_colunas );

/*Function: pgm_destroy_matrix_float
		Desaloca uma <PGM_Matriz_Float>

	Parameter:
		PGM_Matriz_Float *matriz - Matriz a ser desalocada.

 */
void pgm_destroy_matrix_float (PGM_Matriz_Float *matriz);

PGM_Matriz_Float *pgm_copy_PGM_Matriz_Float(PGM_Matriz_Float *a);

PGM_Matriz_Double *matrix_realloc(PGM_Matriz_Double *m, int new_n_row, int new_n_col);

/*Function: pgm_destroy_matrix_double
		Desaloca uma <PGM_Matriz_Double>

	Parameter:
		PGM_Matriz_Double *matriz - Matriz a ser desalocada.

 */
void pgm_destroy_matrix_double (PGM_Matriz_Double *matriz);

/*Function: pgm_create_matrix_double
		Cria uma <PGM_Matriz_Double> com o numero de linhas e colunadas declarada.

	Parameter:
		int n_linhas - numero de linhas da matriz.
		int n_colunas - numero de colunas da matriz.
	Return:
		PGM_Matriz_Double* - Ponteiro para a matriz criada.
 */
PGM_Matriz_Double* pgm_create_matrix_double( int n_linhas, int n_colunas );

/*Function: pgm_ArrayType2PGM_Matriz_Float
		Converte um ArrayType para <PGM_Matriz_Float>.

	Parameter:
		ArrayType *a - Arraytype a ser convertido.
	Return:
		PGM_Matriz_Float* - Ponteiro para a matriz.
 */
 ArrayType*  pgm_PGM_Matriz_Float2ArrayType( PGM_Matriz_Float* m );

 /*Function: pgm_ArrayType2PGM_Matriz_Double
		Converte um ArrayType para <PGM_Matriz_Float>.

	Parameter:
		ArrayType *a - Arraytype a ser convertido.
	Return:
		PGM_Matriz_Float* - Ponteiro para a matriz.
 */
 PGM_Matriz_Float* pgm_ArrayType2PGM_Matriz_Float( ArrayType *a );

/*Function: pgm_ArrayType2PGM_Matriz_Double
		Converte um ArrayType para <PGM_Matriz_Double>.

	Parameter:
		ArrayType *a - Arraytype a ser convertido.
	Return:
		PGM_Matriz_Double* - Ponteiro para a matriz.
 */
PGM_Matriz_Double* pgm_ArrayType2PGM_Matriz_Double( ArrayType *a );

/*Function: pgm_PGM_Matriz_Double2ArrayType
		Converte um  <PGM_Matriz_Double> para ArrayType.

	Parameter:
		PGM_Matriz_Double* m - matriz a ser convertido.
	Return:
		PGM_Matriz_Double* - Ponteiro para a matriz.
 */
ArrayType*  pgm_PGM_Matriz_Double2ArrayType( PGM_Matriz_Double* m );


/*Function: pgm_destroy_PGM_Vetor_Int
		Desaloca um <PGM_Vetor_Int>;

	Parameter:
		PGM_Vetor_Int* vetor - vetor a ser desalocado.

 */
void pgm_destroy_PGM_Vetor_Int(PGM_Vetor_Int* vetor);

/*Function: pgm_create_PGM_Vetor_Int
		Cria um <PGM_Vetor_Int> com o numero de elementos desejado.

	Parameter:
		 int n_elems - numero de elementos.
	Return:
		PGM_Vetor_Int* - Ponteiro para o novo PGM_Vetor_Int.
 */
PGM_Vetor_Int* pgm_create_PGM_Vetor_Int( int n_elems);

/*Function: pgm_Array2PGM_Vetor_Int
		Converte um array para <PGM_Vetor_Int>.

	Parameter:
		 int n_elems - Numero de elementos do array a ser convertido.
		 int vetor[] - array a ser convertido.
	Return:
		PGM_Vetor_Int* - Ponteiro para o novo PGM_Vetor_Int.
 */
PGM_Vetor_Int* pgm_Array2PGM_Vetor_Int( int n_elems, int vetor[] );

/*Function: pgm_ArrayType2PGM_Vetor_Int
		Converte um ArraType para <PGM_Vetor_Int>.

	Parameter:
		 ArrayType *a - ArrayType a ser convertido.
	Return:
		PGM_Vetor_Int* - Ponteiro para o PGM_Vetor_Int.
 */
PGM_Vetor_Int* pgm_ArrayType2PGM_Vetor_Int( ArrayType *a );

/*Function: pgm_PGM_Vetor_Int2ArrayType
		Converte um <PGM_Vetor_Int> para ArraType.

	Parameter:
		PGM_Vetor_Int* - PGM_Vetor_Int a ser convertido.
	Return:
		ArrayType *a - Ponteiro para o ArrayType.
 */
ArrayType* pgm_PGM_Vetor_Int2ArrayType( PGM_Vetor_Int* v );

/*Function: pgm_destroy_PGM_Vetor_Double
		Desaloca um <PGM_Vetor_Double>

	Parameter:
		PGM_Vetor_Double* vetor - PGM_Vetor_Double a ser desalocado.

 */
void pgm_destroy_PGM_Vetor_Double(PGM_Vetor_Double* vetor);

/*Function: pgm_create_PGM_Vetor_Double
		Cria um <PGM_Vetor_Double> com o numero de elementos desejado.

	Parameter:
		 int n_elems - numero de elementos.
	Return:
		PGM_Vetor_Double* - Ponteiro para o novo PGM_Vetor_Double.
 */
PGM_Vetor_Double* pgm_create_PGM_Vetor_Double( int n_elems);

/*Function: pgm_Array2PGM_Vetor_Double
		Converte um array para <PGM_Vetor_Double>.

	Parameter:
		 int n_elems - Numero de elementos do array a ser convertido.
		 int vetor[] - array a ser convertido.
	Return:
		PGM_Vetor_Double* - Ponteiro para o novo PGM_Vetor_Double.
 */
PGM_Vetor_Double* pgm_Array2PGM_Vetor_Double( int n_elems, double vetor[]);

/*Function: pgm_ArrayType2PGM_Vetor_Double
		Converte um ArraType para <PGM_Vetor_Double>.

	Parameter:
		 ArrayType *a - ArrayType a ser convertido.
	Return:
		PGM_Vetor_Double* - Ponteiro para o PGM_Vetor_Double.
 */
PGM_Vetor_Double* pgm_ArrayType2PGM_Vetor_Double( ArrayType *a );

/*Function: pgm_PGM_Vetor_Double2ArrayType
		Converte um <PGM_Vetor_Double> para ArraType.

	Parameter:
		PGM_Vetor_Double* - PGM_Vetor_Double a ser convertido.
	Return:
		ArrayType *a - Ponteiro para o ArrayType.
 */
ArrayType* pgm_PGM_Vetor_Double2ArrayType( PGM_Vetor_Double* v );

/*Function: matrix_transpose
		Transpoe a matriz.
	Parameter:
		PGM_Matriz_Double* m - matriz a ser transposta.
    Return:
        PGM_Matriz_Double* - ponteiro para a matriz transposta
 */
PGM_Matriz_Double *matrix_transpose(PGM_Matriz_Double *m);
/*Function: pgm_copy_PGM_Vetor_Double
		Duplica um PGM_Vetor_Double
	Parameter:
		PGM_Vetor_Double* v - vetor
    Return:
        PGM_Vetor_Double* - ponteiro para o novo vetor
*/
PGM_Vetor_Double *pgm_copy_PGM_Vetor_Double(PGM_Vetor_Double *a);
/*Function: pgm_copy_PGM_Matriz_Double
		Duplica um PGM_Matriz_Double
	Parameter:
		PGM_Matriz_Double* a - vetor
    Return:
        PGM_Matriz_Double* - ponteiro para a nova matriz
*/
PGM_Matriz_Double *pgm_copy_PGM_Matriz_Double(PGM_Matriz_Double *a);
/*Function: pgm_copy_PGM_Matriz_Double
		Duplica um PGM_Matriz_Double
	Parameter:
		PGM_Matriz_Double* a - matriz
    Return:
        PGM_Matriz_Double* - ponteiro para a nova matriz
*/
double get_min_term(PGM_Matriz_Double *m);
/*Function: get_max_term
		Busca o menor valor da matriz
	Parameter:
		PGM_Matriz_Double* a - matriz
    Return:
        double - menor valor da matriz
*/
double get_max_term(PGM_Matriz_Double *m);
/*Function: get_matrix_means
		Calcula a media entre os valores da matriz
	Parameter:
		PGM_Matriz_Double* a - matriz
    Return:
        double - media dos valores da matriz
*/
double get_matrix_means(PGM_Matriz_Double *m);
/*Function: matrix_apply_scale
		Aplica uma escala a uma determinada matriz.
	Parameter:
		PGM_Matriz_Double* matrix - matriz
		double f - escala a ser aplicada
    Return:
        double - media dos valores da matriz
*/
void matrix_apply_scale(PGM_Matriz_Double *matrix, double f);
/*Function: matrix_set_col_value
		Muda os valores de uma coluna de uma dada matriz
	Parameter:
		PGM_Matriz_Double* matrix - matriz
		int column - numero da coluna ser mudada. Lembrando que a primeiro coluna eh zero.
		double value - valor a ser colocado na matriz

*/
void matrix_set_col_value(PGM_Matriz_Double *matrix, int column, double value);

/*Function: matrix_set_line_value
		Muda os valores de uma linha de uma dada matriz
	Parameter:
		PGM_Matriz_Double* matrix - matriz
		int line - numero da linha ser mudada. Lembrando que a primeira linha eh zero.
		double value - valor a ser colocado na matriz
*/
void matrix_set_line_value(PGM_Matriz_Double *matrix, int line, double value);
/*Function: matrix_transpose2
		Transpoe parte de uma matriz, limitada pela line e col.
	Parameter:
		PGM_Matriz_Double* trans_b
		int line - numero de linhas que serão transposta
		int col - numero de colunas que serao transposta
	Return:
        	PGM_Matriz_Double* - matriz transposta
*/
PGM_Matriz_Double *matrix_transpose2(PGM_Matriz_Double *trans_b, int line, int col);

/*Function: matrix_remove_line_col
		Cria uma nova matriz removendo 'linhas' e 'colunas'.
	Parameter:
		PGM_Matriz_Double* matrix
		PGM_Vetor_Int *linhas - indices das linhas que serão removidas
		PGM_Vetor_Int *colunas - indices das colunas que serao removidas
	Return:
        	PGM_Matriz_Double* - matriz nova
*/
PGM_Matriz_Double *matrix_remove_line_col( PGM_Matriz_Double *matrix, PGM_Vetor_Int *linhas, PGM_Vetor_Int *colunas );

PGM_Matriz_Int* pgm_create_matrix_int( int n_linhas, int n_colunas );
void pgm_destroy_matrix_int (PGM_Matriz_Int *matriz);
PGM_Matriz_Int* pgm_ArrayType2PGM_Matriz_Int( ArrayType *a );
ArrayType*  pgm_PGM_Matriz_Int2ArrayType( PGM_Matriz_Int* m );
#endif


