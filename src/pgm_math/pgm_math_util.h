#ifndef PGM_MATH_UTIL_H
#define PGM_MATH_UTIL_H

void center_matrix( PGM_Matriz_Double *m, PGM_Vetor_Double *v );
/*Function: center_matrix
        Centraliza  amtriz de dados M em torno de V
        (subtrai V de M, para cada linha)
	Parameter:
		PGM_Matriz_Double* M - Matriz
		PGM_Vetor_Double*  V - vetor
	Return:
		void
 */

void matrix_stats(  PGM_Matriz_Double* A, PGM_Matriz_Double **L, PGM_Matriz_Double **C, PGM_Matriz_Double **M );
/*Function: matrix_stats
        Calcula MIN, MAX, AVG, STDDEV_SAMP, STDDEV_POP, COUNT (estatisticas) para uma Matriz

	Parameter:
		PGM_Matriz_Double* A - Matriz
		PGM_Matriz_Double** L - Estatisticas das linhas
		PGM_Matriz_Double** C - Estatisticas das colunas
		PGM_Matriz_Double** M - Estatisticas da matriz
	Return:
		void
 */

void lsh_cos( PGM_Matriz_Double* A, PGM_Matriz_Double* B, PGM_Matriz_Double *C );

/*Function: cos_ndimensional

	Parameter:
		PGM_Matriz_Double* A
		PGM_Matriz_Double* B
	Return:
		void* - Ponteiro para a memoria alocada.
 */
PGM_Vetor_Double *cos_ndimensional (PGM_Matriz_Double* A, PGM_Matriz_Double* B);

/*Function: cos_vetorial
		Calcula o cosseno do angulo entre duas Matrizes
        Ambos devem ter as mesmas dimenções
	Parameter:
		PGM_Vetor_Double *A - vetor A
		PGM_Vetor_Double *B - vetor B

	Return:
		double - valor do cosseno
 */
double cos_vetorial (PGM_Vetor_Double *A,PGM_Vetor_Double *B);

/*Function: weighted_mean
		Calcula o cosseno do angulo entre dois vetores
        Ambos devem ter as mesmas dimenções
	Parameter:
		PGM_Matriz_Double *A - matriz A
		double p - peso de A
		PGM_Matriz_Double *B - matriz B
		double q - peso de B

	Return:
		PGM_Matriz_Double* - Ponteiro para a matriz das medias
 */
PGM_Matriz_Double *weighted_mean(PGM_Matriz_Double *A, double p,PGM_Matriz_Double *B, double q);

/*Function: max
		Retorna o maior valor entre dois numeros
	Parameter:
		int a
		int b
	Return:
		int - maior valor
 */
int max(int a, int b);

/*Function: min
		Retorna o menor valor entre dois numeros
	Parameter:
		int a
		int b
	Return:
		int - menor valor
 */
int min(int a, int b);

#endif // PGM_MATH_UTIL_H
