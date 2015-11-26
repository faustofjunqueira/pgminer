/* Matriz.h: estrutura para interface c x postgresql/c
 *
 * Autor: GZS, FFJ
 * GCCBD - COOPE - UFRJ
 * Criacao: 12/01/2012
 */

#ifndef PGM_MATRIZ_H
#define PGM_MATRIZ_H
/*Title: Matrizes e Vetores*/

/* Group: Matrizes
		Arrays Bi-Dimencionais.
*/

/*Struct: PGM_Matriz_Double
		PGM_Matriz_Double é uma matriz bidimencional utilizada para armazenar valores *double*. Similar à utilizar *array[][]*,
		ela é se comporta melhor poís ja tem as funções de criação, destruição, e converções para o PostgreSQL.

		Para acessar um elemento da PGM_Matriz_Double utilize PGM_ELEM_MATRIZ(matriz, linha, coluna).

		int n_linhas - Numero de linhas da Matriz.
		int n_colunas - Numero de Colunas da Matriz.
        double *valor - Array com os valores da Matriz.

  	See Also:
  	- <pgm_destroy_matrix_double>
    - <pgm_create_matrix_double>
  	- <pgm_ArrayType2PGM_Matriz_Double>
  	- <pgm_PGM_Matriz_Double2ArrayType>

*/
typedef struct {
  int n_linhas;
  int n_colunas;
  double *valor;
} PGM_Matriz_Double;

typedef struct {
  int n_linhas;
  int n_colunas;
  int *valor;
} PGM_Matriz_Int;

typedef struct {
  int n_linhas;
  int n_colunas;
  float *valor;
} PGM_Matriz_Float;

typedef struct {
  int max_dim;
  int n_linhas;
  int n_colunas;
  void *valor;
} PGM_Matriz_GPU;

/* Group: Vetores
		Arrays unidimencionais.
*/

/*Struct: PGM_Vetor_Int
		PGM_Vetor_Int é um vetor unidimencional, cujo armazena valores *int*. Similar à utilizar *array[]*,
		ela é se comporta melhor poís ja tem as funções de criação, destruição, e converções para o PostgreSQL.

		Para acessar um elemento da PGM_Vetor_Int utilize PGM_ELEM_VETOR(matriz, posicao).

		int n_elems - Numero de elementos do vetor.
  	double *valor - Array com os valores da matriz.

  	See Also:
  	- <pgm_destroy_PGM_Vetor_Int>
		- <pgm_create_PGM_Vetor_Int>
  	- <pgm_ArrayType2PGM_Vetor_Int>
  	- <pgm_PGM_Vetor_Int2ArrayType>

*/
typedef struct {
  int n_elems;
  int *valor;
} PGM_Vetor_Int;

/*Struct: PGM_Vetor_Double
		PGM_Vetor_Double é um vetor unidimencional, cujo armazena valores *double*. Similar à utilizar *array[]*,
		ela é se comporta melhor poís ja tem as funções de criação, destruição, e converções para o PostgreSQL.

		Para acessar um elemento da PGM_Vetor_Double utilize PGM_ELEM_VETOR(matriz, posicao).

		int n_elems - Numero de elementos do vetor.
  	double *valor - Array com os valores da matriz.

  	See Also:
  	- <pgm_destroy_PGM_Vetor_Double>
		- <pgm_create_PGM_Vetor_Double>
  	- <pgm_ArrayType2PGM_Vetor_Double>
  	- <pgm_PGM_Vetor_Double2ArrayType>

*/
typedef struct {
  int n_elems;
  double *valor;
} PGM_Vetor_Double;

#define PGM_ELEM_MATRIZ( matriz, linha, coluna ) \
  ((matriz)->valor[(matriz)->n_colunas * (linha) + (coluna)])

#define PGM_ELEM_VETOR( matriz, posicao ) \
  ((matriz)->valor[posicao])

#define PGM_LINHA_MATRIZ( matriz, linha ) \
  ((matriz)->valor + (matriz)->n_colunas * (linha))

#define PGM_SET_ELEM_MATRIZ(matriz, linha, coluna, value)\
   (PGM_ELEM_MATRIZ( matriz, linha, coluna ) = (value))

#define PGM_SET_ELEM_VETOR(matriz, posicao, value)\
   (PGM_ELEM_VETOR( (matriz), (posicao) ) = (value))

#define PGM_COPIALINHA_MATRIZ( matriz_destino, linha_destino, matriz_origem, linha_origem ) \
  { \
    double* md = PGM_LINHA_MATRIZ ( matriz_destino , linha_destino ); \
    double* mo = PGM_LINHA_MATRIZ ( matriz_origem , linha_origem ); \
    int i, tamanho = (matriz_destino)->n_colunas; \
    for ( i = 0 ; i < tamanho; i++) \
        md[i] = mo[i]; \
  }

#define PGM_SOMALINHA_MATRIZ( matriz_destino, linha_destino, matriz_origem, linha_origem ) \
  { \
    double* md = PGM_LINHA_MATRIZ ( matriz_destino , linha_destino ); \
    double* mo = PGM_LINHA_MATRIZ ( matriz_origem , linha_origem ); \
    int i, tamanho = (matriz_destino)->n_colunas; \
    for ( i = 0 ; i < tamanho; i++) \
        md[i] += mo[i]; \
  }


#endif

