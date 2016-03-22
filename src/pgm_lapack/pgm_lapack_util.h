#ifndef PGM_LAPACK_UTIL_H
#define PGM_LAPACK_UTIL_H
/*Function: lapack_svd
        Calcula o svd de uma matriz
	Parameter:
		PGM_Matriz_Double* matriz
	Return:
		PGM_svdrec* - Ponteiro para a PGM_svdrec.
 */
PGM_svdrec* lapack_svd(PGM_Matriz_Double* matrix);
/*Function: ordinary_least_squares
        Calcula os As = B (minimos quadrados).
        o resultado sai em s.
	Parameter:
		PGM_Matriz_Double* A
		PGM_Matriz_Double* B
		OUT_PGM_Vetor_Double *s
	Return:
		int - rank do OLS
 */
int ordinary_least_squares(PGM_Matriz_Double *a, PGM_Matriz_Double *b , OUT_PGM_Vetor_Double *s);
/*Function: matrix_inverse
        Inverte uma matriz quadrada
	Parameter:
		double* A - matriz quadrada
		int N - tamanho da dimenção
	Return:
		void
 */
void matrix_inverse(double* A, int N);
#endif // PGM_LAPACK_UTIL_H
