/** Criado em 24/07/2012
 * Fausto Junqueira e Geraldo Zimbão
 *
 */

#ifndef PGM_MALLOC_H
#define PGM_MALLOC_H

/*Title: Alocando Memória...
*/

/*Function: pgm_malloc
		Aloca um bloco de memoria de tamanho desejado, em bytes.

		Similar ao *malloc* da stdlib.h, porém adaptada para o PostgreSQL.

	Parameter:
		size_t size - Numero de bytes desejados. Tal número deve ser inteiro e positivo.

	Return:
		void* - Ponteiro para a memoria alocada
 */
void* pgm_malloc( size_t size );

/*Function: pgm_calloc
		Aloca um bloco de memoria de tamanho desejado, em bytes.

		Similar ao *calloc* da stdlib.h, porém adaptada para o PostgreSQL.

	Parameter:
		size_t num - Numero de elementos a ser alocado.
		size_t size - Tamanho do elemento a ser alocado.

	Return:
		void* - Ponteiro para a memoria alocada.
 */
void* pgm_calloc( size_t num, size_t size );

/*Function: pgm_realloc
		Troca o tamanho do bloco memoria alocada.

		Similar ao *realloc* da stdlib.h, porém adaptada para o PostgreSQL.

	Parameter:
		void* ptr - Memoria anteriormente alocada, com <pgm_malloc>, <pgm_calloc> ou <pgm_realloc>.
		size_t size - Novo tamanho para o bloco de memoria, em bytes.

	Return:
		void* - Ponteiro para a memoria alocada.
 */
void* pgm_realloc( void* ptr, size_t size );

/*Function: pgm_free
		Libera o bloco de memoria alocada

		Similar ao *free* da stdlib.h, porém adaptada para o PostgreSQL.

	Parameter:
		void* ptr - Memoria anteriormente alocada, com <pgm_malloc>, <pgm_calloc> ou <pgm_realloc>.

 */
void  pgm_free( void * ptr );

#endif
