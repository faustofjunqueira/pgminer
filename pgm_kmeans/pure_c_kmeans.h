/*
 * Autor: GZS, FFJ
 * GCCBD - COOPE - UFRJ
 * Criacao: 12/01/2012
 */

#ifndef PURE_C_KMEANS_H
#define PURE_C_KMEANS_H

/*
	Title: K - Means
	COLOKAR AINDA A DEFINIÇÂO DO K-MEANS
*/

/*
	Function: pure_c_kmeans
		Dispara o k means.

	Parameter:
		PGM_Matriz_Double *m - matriz de pontos.
		int k - quantidade de centroidese.
		int seed:
					- 1 - para sortear k pontos para serem os centros dos grupos.
					- 2 - para sortear cada ponto em um grupo.
		int end
					- 1 - se um Grupo Secar, Elimine-o.
					- 2 - se um Grupo Secar, cria um grupo com o elemento mais longe de seu centroide.
					- 3 - se um Grupo Secar, Erro.

	Return:
		- 0 quando não houver erros.
    - 1 significa que o PostgreSQL irá dar "ERRO: Grupo x Secou!", onde x é o grupo vazio.
 */
int pure_c_kmeans( PGM_Matriz_Double *m, int k, int seed, int end,
                   PGM_Matriz_Double *centroides, int *grupo, double *somatorio_distancias );

/*
	Function: change_group
		Troca o ponto para o grupo do centróide mais próximos.

	Parameter:
		PGM_Matriz_Double *m - matriz de pontos.
		PGM_Matriz_Double *centroides - matriz de centroides.
		int *grupo - em qual grupo esta cada ponto.
			Ex: o ponto 5 esta no grupo 1, logo grupo[5] = 1;
		int ponto - ponto que mudará de posição.
		double *somatorio_distancias = é o somatorio das distancias dos pontos.

	Return:
		- 0 se o ponto não trocou de grupo.
    - 1 se o ponto mudou de grupo.
 */
int change_group( PGM_Matriz_Double *centroides, PGM_Matriz_Double *matriz, int *grupo, int ponto, double *somatorio_distancias);

/*
	Function: search_empty_group
		Procura se tem algum grupo vazio.

		(Somente utilizada para end igual a 2).

	Parameter:
		int *n_elem_grupo - numero de elemento em cada grupo.
		int k = quantidade de centroides.

	Return:
		int - O grupo que esta vazio.

	See Also:
		<pure_c_kmeans> - end = 2;
*/
int search_empty_group(int *n_elem_grupo, int k);

/*
	Function: calculate_centroid
		Efetua o calculo da distancia dos centroideis e os reposiciona.

	Parameter:
		PGM_Matriz_Double *m - matriz de pontos.
		PGM_Matriz_Double *centroides - matriz de centroides.
		int *grupo - em qual grupo esta cada ponto.

			Ex: o ponto 5 esta no grupo 1, logo grupo[5] = 1;

		int *n_elem_grupo - numero de elemento em cada grupo.

	Return:
		int - A quantidade de grupos que não estão vazios.

*/
int calculate_centroid ( PGM_Matriz_Double *centroides , PGM_Matriz_Double *matriz , int *grupo , int *n_elem_grupo );

/*
	Function: init_group
		Inicia o array grupo, de acordo com a escolha da semente.

	Parameter:
		PGM_Matriz_Double *centroides - matriz de centroides.
		PGM_Matriz_Double *m - matriz de pontos.
		int *grupo - em qual grupo esta cada ponto.

			Ex: o ponto 5 esta no grupo 1, logo grupo[5] = 1;

		int *n_elem_grupo - numero de elemento em cada grupo.
		int k - numero de centróides
		int semente:
					- 1 - para sortear k pontos para serem os centros dos grupos.
					- 2 - para sortear cada ponto em um grupo.

*/
void init_group(PGM_Matriz_Double *centroides , PGM_Matriz_Double *matriz , int *grupo , int *n_elem_grupo, int k, int semente);

/*
	Function: search_point_away
		Busca o ponto mais distante a partir do ponto (0, 0).

	Parameter:
		PGM_Matriz_Double *m - matriz de pontos.

	Return:
		double* - Linha da matriz que corresponde ao ponto mais distante.

*/
double *search_point_away(PGM_Matriz_Double *matriz);

/*
	Function: create_new_group
		Cria um grupo com o elemento mais longe de seu centroide.

	Parameter:
		PGM_Matriz_Double *centroides - matriz de centroides.
		PGM_Matriz_Double *m - matriz de pontos.
		int grupo_vazio - O grupo que está vazio.

	Return:
		1

	See Also:
		<pure_c_kmeans>

*/
int create_new_group (PGM_Matriz_Double *centroides, PGM_Matriz_Double *matriz, int grupo_vazio);

#endif
