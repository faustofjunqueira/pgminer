
#include "../Util/pgm_matriz.h"
#include "../Util/pgm_matriz_util.h"
#include "svdlib.h"



typedef struct {

  PGM_Matriz_Double* Ut;
  PGM_Vetor_Double *S;
  PGM_Matriz_Double* Vt;
} PGM_svdrec;



/*Title: Funções - SVD
	Colokar a definição
*/

/*
	Function: SVDRec2PGM_svdrec
		Converte a estrutura SVDRec para a <PGM_svdrec>.

	Parameter:
		SVDRec rec - Estrutura a ser convertida

	Return:
		PGM_svdrec* - Ponteiro para PGM_svdrec
 */
PGM_svdrec* SVDRec2PGM_svdrec( SVDRec rec );

/*
	Function: PGM_Matriz_Double2DMat
		Converte uma <PGM_Matriz_Double> para DMat.

	Parameter:
		PGM_Matriz_Double *m - matriz a ser convertida.

	Return:
		DMat
 */
DMat PGM_Matriz_Double2DMat(PGM_Matriz_Double *m);

/*
	Function: DMat2PGM_Matriz_Double
		Converte uma DMat para <PGM_Matriz_Double>.

	Parameter:
		DMat dense - Estrutura a ser convertida.

	Return:
		PGM_Matriz_Double* - Ponteiro para PGM_Matriz_Double.
 */
PGM_Matriz_Double *DMat2PGM_Matriz_Double(DMat dense);

/*
	Function: pure_c_SVD
		Dispara o algoritmo do SVD.

	Parameter:
		PGM_svdrec *rec -
		int numFeatures -

	Return:
		PGM_Matriz_Double* - Ponteiro para PGM_Matriz_Double.
 */
PGM_svdrec* pure_c_SVD( PGM_Matriz_Double *m, int numFeatures );

/*
	Function: pure_c_SVD_matrix_reconstruct
		Dispara o algoritmo do SVD.

	Parameter:
		PGM_svdrec *rec -
		int numFeatures -

	Return:
		PGM_Matriz_Double* - Ponteiro para PGM_Matriz_Double.

	See Also:
		<pure_c_SVD>
 */
PGM_Matriz_Double *pure_c_SVD_matrix_reconstruct( PGM_svdrec *rec, int numFeatures );

PGM_svdrec* pgm_create_PGM_svdrec(int u_linha, int u_coluna, int s_elem, int v_linha, int v_coluna);
/* Struct: PGM_svdrec

	Estrutura correspondente à SVDRec

  PGM_Matriz_Double* Ut -  corresponde ao Dmat Ut
  PGM_Vetor_Double *S - corresponde ao double *S e int d
  PGM_Matriz_Double* Vt - corresponde ao Dmat Vt

	See Also:
		<svdrec>.
*/
