/*
 * Autor: FFJ, GZS
 * GCCBD - COOPE - UFRJ
 * Criacao: 02/04/2012
 */
#include "postgres.h"       //standard
#include "fmgr.h"           //standard
#include "utils/memutils.h"//standard
#include "utils/geo_decls.h"//standard
#include "utils/array.h"    //arrays
#include "utils/errcodes.h" //warning
#include "catalog/pg_type.h"// OID
#include "utils/lsyscache.h"// construi array type
#include "funcapi.h"        // trabalhar com row
#include "access/heapam.h"  //Construção de Tupla

#include "../pgm_matriz.h"
#include "../pgm_matriz_util.h"



PG_FUNCTION_INFO_V1(pgm_matrix_get);
/*Function: pgm_matrix_get
        Converte uma PGM_Matriz_Double para ArrayType
	Parameter:
		PGM_Matriz_Double* matrix - Matriz

	Return:
		ArrayType
 */
Datum pgm_matrix_get(PG_FUNCTION_ARGS);
Datum pgm_matrix_get(PG_FUNCTION_ARGS){
    PGM_Matriz_Double *m = (PGM_Matriz_Double*) PG_GETARG_INT64(0);
    PG_RETURN_ARRAYTYPE_P(pgm_PGM_Matriz_Double2ArrayType(m));
}

PG_FUNCTION_INFO_V1(pgm_matrix_ncols);
/*Function: pgm_matrix_ncols
        Consegue o numero de colunas de uma dada matriz
	Parameter:
		PGM_Matriz_Double* matriz

	Return:
		int - numero de colunas da matriz
 */
Datum pgm_matrix_ncols(PG_FUNCTION_ARGS);
Datum pgm_matrix_ncols(PG_FUNCTION_ARGS){
    PGM_Matriz_Double *matrix = (PGM_Matriz_Double*) PG_GETARG_POINTER(0);

    PG_RETURN_INT32( matrix->n_colunas );
}

PG_FUNCTION_INFO_V1(pgm_matrix_nlines);
/*Function: pgm_matrix_nlines
        Consegue o numero de linhas de uma dada matriz
	Parameter:
		PGM_Matriz_Double* matriz

	Return:
		int - numero de linhas da matriz
 */
Datum pgm_matrix_nlines(PG_FUNCTION_ARGS);
Datum pgm_matrix_nlines(PG_FUNCTION_ARGS){
    PGM_Matriz_Double *matrix = (PGM_Matriz_Double*) PG_GETARG_POINTER(0);

    PG_RETURN_INT32( matrix->n_linhas );
}

PG_FUNCTION_INFO_V1(pgm_matrix_create);
/*Function: pgm_matrix_create
        Cria uma matriz
	Parameter:
        int n_lines = numero de linhas da matriz
        int n_cols = numero de colunas da matriz

	Return:
		PGM_Matriz_Double* - Matriz criada
 */
Datum pgm_matrix_create(PG_FUNCTION_ARGS);
Datum pgm_matrix_create(PG_FUNCTION_ARGS){
    int n_lines = PG_GETARG_INT32( 0 );
    int n_cols = PG_GETARG_INT32( 1 );

    MemoryContext contextoAnterior = MemoryContextSwitchTo( CurTransactionContext );
    PGM_Matriz_Double *matrix = pgm_create_matrix_double( n_lines, n_cols );
    MemoryContextSwitchTo( contextoAnterior );

    PG_RETURN_POINTER( matrix );
}

PG_FUNCTION_INFO_V1(pgm_matrix_set_elem);
/*Function: pgm_matrix_set_elem
        Muda o valor da matriz
	Parameter:
        PGM_Matriz_Double *matrix - a matriz
        int line = numero da linhas do elemento
        int col = numero da colunas do elemento
        double value - valor a ser colocado na matriz
	Return:
		PGM_Matriz_Double* - Matriz criada
 */
Datum pgm_matrix_set_elem(PG_FUNCTION_ARGS);
Datum pgm_matrix_set_elem(PG_FUNCTION_ARGS){
    PGM_Matriz_Double *matrix = (PGM_Matriz_Double*) PG_GETARG_POINTER(0);
    int line = PG_GETARG_INT32( 1 );
    int col = PG_GETARG_INT32( 2 );
    double value = PG_GETARG_FLOAT8( 3 );

    PGM_SET_ELEM_MATRIZ( matrix, line, col, value );

    PG_RETURN_POINTER( matrix );
}

PG_FUNCTION_INFO_V1(pgm_matrix_get_elem);
/*Function: pgm_matrix_set_elem
        Consegue o valor da Matriz
	Parameter:
        PGM_Matriz_Double *matrix - a matriz
        int line = numero da linhas do elemento
        int col = numero da colunas do elemento
	Return:
		double value - retorna o valor buscado
 */
Datum pgm_matrix_get_elem( PG_FUNCTION_ARGS );
Datum pgm_matrix_get_elem( PG_FUNCTION_ARGS ){
    PGM_Matriz_Double *matrix = (PGM_Matriz_Double*) PG_GETARG_POINTER(0);
    int line = PG_GETARG_INT32( 1 );
    int col = PG_GETARG_INT32( 2 );

    PG_RETURN_FLOAT8( PGM_ELEM_MATRIZ( matrix, line, col ) );
}

PG_FUNCTION_INFO_V1(pgm_matrix_free);
/*Function: pgm_matrix_free
        Libera a memoria de uma matriz
	Parameter:
        PGM_Matriz_Double *matrix - a matriz
	Return:
		void
 */
Datum pgm_matrix_free( PG_FUNCTION_ARGS );
Datum pgm_matrix_free( PG_FUNCTION_ARGS ){
    PGM_Matriz_Double* m = (PGM_Matriz_Double*) PG_GETARG_POINTER (0);
    pgm_destroy_matrix_double(m);

    PG_RETURN_VOID();
}

PG_FUNCTION_INFO_V1(pgm_matrix_set_line);
/*Function: pgm_matrix_set_line
        Troca os valores de uma linha na matriz
	Parameter:
        PGM_Matriz_Double *matrix - a matriz
        ArrayType array - Um array de dimenção 1, e com a mesma quantidade de elementos da linha da matriz
	Return:
		PGM_Matriz_Double* - a matriz com a linha trocada
 */
Datum pgm_matrix_set_line( PG_FUNCTION_ARGS );
Datum pgm_matrix_set_line( PG_FUNCTION_ARGS ){
    PGM_Matriz_Double *matrix = (PGM_Matriz_Double*) PG_GETARG_POINTER(0);
    int line = PG_GETARG_INT32( 1 );
    ArrayType *array = PG_GETARG_ARRAYTYPE_P(2);
    double *a = PGM_LINHA_MATRIZ( matrix, line ), *b;
    PGM_Vetor_Double *valores;

    if (ARR_NDIM(array) != 1)
        elog(ERROR, "O array deve possuir somente uma dimensao");

    if (matrix->n_colunas != ARR_DIMS(array)[0])
        elog(ERROR, "Tamanho do array é diferente do numero de dimensoes da matriz/tabela! %d %d", matrix->n_colunas, ARR_DIMS(PG_GETARG_ARRAYTYPE_P(2))[0] );

    valores = pgm_ArrayType2PGM_Vetor_Double( array );
    b = valores->valor;

    {
        int i;
        for( i=0; i < matrix->n_colunas; i++ )
            a[i] = b[i];
    }

    PG_RETURN_POINTER( matrix );
}

PG_FUNCTION_INFO_V1(pgm_matrix_get_line);
/*Function: pgm_matrix_get_line
        Consegue os valores de uma linha da Matriz
	Parameter:
        PGM_Matriz_Double *matrix - a matriz
        int line - numero da linha, lembrando que a primeira linha eh a zero
	Return:
		ArrayType - retorna os valores da linha
 */
Datum pgm_matrix_get_line( PG_FUNCTION_ARGS );
Datum pgm_matrix_get_line( PG_FUNCTION_ARGS ){
    PGM_Matriz_Double *m = (PGM_Matriz_Double*) PG_GETARG_POINTER(0);
    int line = PG_GETARG_INT32( 1 );
    PGM_Vetor_Double *result = pgm_create_PGM_Vetor_Double( m->n_colunas );
    double *line_matrix = PGM_LINHA_MATRIZ(m,line);

    {
        int i;
        for(i=0; i<m->n_colunas; i++)
            result->valor[i] = line_matrix[i];
    }


    PG_RETURN_ARRAYTYPE_P(pgm_PGM_Vetor_Double2ArrayType(result));
}

PG_FUNCTION_INFO_V1(pgm_matrix_get_col);
/*Function: pgm_matrix_get_col
        Consegue os valores de uma coluna da Matriz
	Parameter:
        PGM_Matriz_Double *matrix - a matriz
        int coluna - numero da coluna, lembrando que a primeira coluna eh a zero
	Return:
		ArrayType - retorna os valores da coluna
 */
Datum pgm_matrix_get_col( PG_FUNCTION_ARGS );
Datum pgm_matrix_get_col( PG_FUNCTION_ARGS ){
    PGM_Matriz_Double *m = (PGM_Matriz_Double*) PG_GETARG_POINTER(0);
    int col = PG_GETARG_INT32( 1 );
    PGM_Vetor_Double *result = pgm_create_PGM_Vetor_Double( m->n_linhas );

    {
        int i;
        for(i=0; i<m->n_linhas; i++)
            result->valor[i] = PGM_ELEM_MATRIZ( m, i, col );
    }


    PG_RETURN_ARRAYTYPE_P(pgm_PGM_Vetor_Double2ArrayType(result));
}
//MATRIX FLOAT
PG_FUNCTION_INFO_V1(pgm_matrix_float_create);
/*Function: pgm_matrix_float_create
        Cria uma matriz
	Parameter:
        int n_lines = numero de linhas da matriz
        int n_cols = numero de colunas da matriz

	Return:
		PGM_Matriz_Float* - Matriz criada
 */
Datum pgm_matrix_float_create(PG_FUNCTION_ARGS);
Datum pgm_matrix_float_create(PG_FUNCTION_ARGS){
    int n_lines = PG_GETARG_INT32( 0 );
    int n_cols = PG_GETARG_INT32( 1 );

    MemoryContext contextoAnterior = MemoryContextSwitchTo( CurTransactionContext );
    PGM_Matriz_Float *matrix = pgm_create_matrix_float( n_lines, n_cols );
    MemoryContextSwitchTo( contextoAnterior );

    PG_RETURN_POINTER( matrix );
}

PG_FUNCTION_INFO_V1(pgm_matrix_float_set_elem);
/*Function: pgm_matrix_float_set_elem
        Muda o valor da matriz
	Parameter:
        PGM_Matriz_Float *matrix - a matriz
        int line = numero da linhas do elemento
        int col = numero da colunas do elemento
        double value - valor a ser colocado na matriz
	Return:
		PGM_Matriz_Float* - Matriz criada
 */
Datum pgm_matrix_float_set_elem(PG_FUNCTION_ARGS);
Datum pgm_matrix_float_set_elem(PG_FUNCTION_ARGS){
    PGM_Matriz_Float *matrix = (PGM_Matriz_Float*) PG_GETARG_POINTER(0);
    int line = PG_GETARG_INT32( 1 );
    int col = PG_GETARG_INT32( 2 );
    float value = PG_GETARG_FLOAT4( 3 );

    PGM_SET_ELEM_MATRIZ( matrix, line, col, value );

    PG_RETURN_POINTER( matrix );
}

// VETOR INTEGER

PG_FUNCTION_INFO_V1(pgm_array2vector_int);
/*Function: pgm_array2vector_double
        Converte uma ArrayType em um PGM_Vetor_Double
	Parameter:
        ArrayType array
	Return:
		PGM_Vetor_Double*
 */
Datum pgm_array2vector_int( PG_FUNCTION_ARGS );
Datum pgm_array2vector_int( PG_FUNCTION_ARGS ){
    MemoryContext contextoAnterior = MemoryContextSwitchTo( CurTransactionContext );
    PGM_Vetor_Int *vector = pgm_ArrayType2PGM_Vetor_Int( PG_GETARG_ARRAYTYPE_P(0) );

    MemoryContextSwitchTo( contextoAnterior );

    PG_RETURN_POINTER( vector );
}

PG_FUNCTION_INFO_V1(pgm_vector_int_get);
/*Function: pgm_vector_int_get
        Converte uma PGM_Vetor_Int para ArrayType
	Parameter:
		PGM_Vetor_Int* vetor - Vetor

	Return:
		ArrayType
 */
Datum pgm_vector_int_get(PG_FUNCTION_ARGS);
Datum pgm_vector_int_get(PG_FUNCTION_ARGS){
    PGM_Vetor_Int *m = (PGM_Vetor_Int*) PG_GETARG_INT64(0);
    PG_RETURN_ARRAYTYPE_P(pgm_PGM_Vetor_Int2ArrayType(m));
}

PG_FUNCTION_INFO_V1(pgm_vetor_int_elem);
/*Function: pgm_vetor_int_elem
        Consegue o numero de elementos de um vetor
	Parameter:
		PGM_Vetor_Int* vetor - Vetor

	Return:
		ArrayType
 */
Datum pgm_vetor_int_elem( PG_FUNCTION_ARGS );
Datum pgm_vetor_int_elem( PG_FUNCTION_ARGS ){

    PGM_Vetor_Int *vetor = (PGM_Vetor_Int*) PG_GETARG_POINTER(0);

    PG_RETURN_INT32( vetor->n_elems);
}

PG_FUNCTION_INFO_V1(pgm_vetor_int_create);
/*Function: pgm_vetor_int_create
        Cria um PGM_Vetor_Int com n_elem zerados
	Parameter:
		int n_elem - numero de elementos

	Return:
		ArrayType
 */
Datum pgm_vetor_int_create( PG_FUNCTION_ARGS );
Datum pgm_vetor_int_create( PG_FUNCTION_ARGS ){
    int n_elems = PG_GETARG_INT32(0);
    MemoryContext contextoAnterior = MemoryContextSwitchTo( CurTransactionContext );
    PGM_Vetor_Int* vetor = pgm_create_PGM_Vetor_Int(n_elems);
    MemoryContextSwitchTo( contextoAnterior );

    PG_RETURN_POINTER(vetor);
}

PG_FUNCTION_INFO_V1(pgm_vetor_int_set_elem);
/*Function: pgm_vetor_int_set_elem
        Troca o valor de um elemento do vetor
	Parameter:
        PGM_Vetor_Int *vetor
		int elem - numero de indice do elementos
		int value - valor a ser colocado

	Return:
		PGM_Vetor_Int*
 */
Datum pgm_vetor_int_set_elem(PG_FUNCTION_ARGS);
Datum pgm_vetor_int_set_elem(PG_FUNCTION_ARGS){
    PGM_Vetor_Int *vetor = (PGM_Vetor_Int*) PG_GETARG_POINTER(0);
    int posicao = PG_GETARG_INT32(1);
    int value = PG_GETARG_INT32(2);

    PGM_SET_ELEM_VETOR( vetor, posicao, value );

    PG_RETURN_POINTER( vetor );
}

PG_FUNCTION_INFO_V1(pgm_vetor_int_get_elem);
/*Function: pgm_vetor_int_get_elem
        Consegue o valor de um elemento do vetor
	Parameter:
        PGM_Vetor_Int *vetor
		int elem - numero de indice do elementos
	Return:
		int
 */
Datum pgm_vetor_int_get_elem( PG_FUNCTION_ARGS );
Datum pgm_vetor_int_get_elem( PG_FUNCTION_ARGS ){
    PGM_Vetor_Int *vetor = (PGM_Vetor_Int*) PG_GETARG_POINTER(0);
    int posicao = PG_GETARG_INT32( 1 );
    int value = PGM_ELEM_VETOR( vetor, posicao);

    PG_RETURN_INT32( value );
}

PG_FUNCTION_INFO_V1(pgm_vetor_int_free);
/*Function: pgm_vetor_int_free
        Libera a memoria de um PGM_Vetor_Int
	Parameter:
        PGM_Vetor_Int *vetor

 */
Datum pgm_vetor_int_free( PG_FUNCTION_ARGS );
Datum pgm_vetor_int_free( PG_FUNCTION_ARGS ){
    PGM_Vetor_Int* vetor = (PGM_Vetor_Int*) PG_GETARG_POINTER (0);
    pgm_destroy_PGM_Vetor_Int(vetor);

    PG_RETURN_VOID();
}

// VETOR DOUBLE

PG_FUNCTION_INFO_V1(pgm_vector_double_get);
/*Function: pgm_vector_double_get
        Converte uma PGM_Vetor_Double para ArrayType
	Parameter:
		PGM_Vetor_Double* vetor - Vetor

	Return:
		ArrayType
*/
Datum pgm_vector_double_get(PG_FUNCTION_ARGS);
Datum pgm_vector_double_get(PG_FUNCTION_ARGS){
    PGM_Vetor_Double *m = (PGM_Vetor_Double*) PG_GETARG_INT64(0);
    PG_RETURN_ARRAYTYPE_P(pgm_PGM_Vetor_Double2ArrayType(m));
}

PG_FUNCTION_INFO_V1(pgm_vetor_double_elem);
/*Function: pgm_vetor_double_elem
        Consegue o numero de elementos de um vetor
	Parameter:
		PGM_Vetor_Double* vetor - Vetor

	Return:
		ArrayType
 */
Datum pgm_vetor_double_elem( PG_FUNCTION_ARGS );
Datum pgm_vetor_double_elem( PG_FUNCTION_ARGS ){
    PGM_Vetor_Double *vetor = (PGM_Vetor_Double*) PG_GETARG_POINTER(0);

    PG_RETURN_INT32( vetor->n_elems);
}

PG_FUNCTION_INFO_V1(pgm_vetor_double_create);
/*Function: pgm_vetor_double_create
        Cria um PGM_Vetor_Double com n_elem zerados
	Parameter:
		int n_elem - numero de elementos

	Return:
		ArrayType
 */
Datum pgm_vetor_double_create( PG_FUNCTION_ARGS );
Datum pgm_vetor_double_create( PG_FUNCTION_ARGS ){
    int n_elems = PG_GETARG_INT32(0);
    MemoryContext contextoAnterior = MemoryContextSwitchTo( CurTransactionContext );
    PGM_Vetor_Double* vetor = pgm_create_PGM_Vetor_Double(n_elems);
    MemoryContextSwitchTo( contextoAnterior );

    PG_RETURN_POINTER(vetor);
}

PG_FUNCTION_INFO_V1(pgm_vetor_double_set_elem);
/*Function: pgm_vetor_double_set_elem
        Troca o valor de um elemento do vetor
	Parameter:
        PGM_Vetor_Double *vetor
		int elem - numero de indice do elementos
		int value - valor a ser colocado

	Return:
		PGM_Vetor_Double*
 */
Datum pgm_vetor_double_set_elem(PG_FUNCTION_ARGS);
Datum pgm_vetor_double_set_elem(PG_FUNCTION_ARGS){
    PGM_Vetor_Double *vetor = (PGM_Vetor_Double*) PG_GETARG_POINTER(0);
    int posicao = PG_GETARG_INT32(1);
    double value = PG_GETARG_FLOAT8(2);

    PGM_SET_ELEM_VETOR( vetor, posicao, value );

    PG_RETURN_POINTER( vetor );
}

PG_FUNCTION_INFO_V1(pgm_array2vector_double);
/*Function: pgm_array2vector_double
        Converte uma ArrayType em um PGM_Vetor_Double
	Parameter:
        ArrayType array
	Return:
		PGM_Vetor_Double*
 */
Datum pgm_array2vector_double( PG_FUNCTION_ARGS );
Datum pgm_array2vector_double( PG_FUNCTION_ARGS ){
    MemoryContext contextoAnterior = MemoryContextSwitchTo( CurTransactionContext );
    PGM_Vetor_Double *vector = pgm_ArrayType2PGM_Vetor_Double( PG_GETARG_ARRAYTYPE_P(0) );

    MemoryContextSwitchTo( contextoAnterior );

    PG_RETURN_POINTER( vector );
}

PG_FUNCTION_INFO_V1(pgm_vetor_double_get_elem);
/*Function: pgm_vetor_double_get_elem
        Consegue o valor de um elemento do vetor
	Parameter:
        PGM_Vetor_Double *vetor
		int elem - numero de indice do elementos
	Return:
		double
 */
Datum pgm_vetor_double_get_elem( PG_FUNCTION_ARGS );
Datum pgm_vetor_double_get_elem( PG_FUNCTION_ARGS ){
	PGM_Vetor_Double *vetor = (PGM_Vetor_Double*) PG_GETARG_POINTER(0);
	int posicao = PG_GETARG_INT32( 1 );
	double value = PGM_ELEM_VETOR( vetor, posicao);

	PG_RETURN_FLOAT8( value );
}

PG_FUNCTION_INFO_V1(pgm_vetor_double_free);
/*Function: pgm_vetor_int_free
        Libera a memoria de um PGM_Vetor_Double
	Parameter:
        PGM_Vetor_Double *vetor

 */
Datum pgm_vetor_double_free( PG_FUNCTION_ARGS );
Datum pgm_vetor_double_free( PG_FUNCTION_ARGS ){
    PGM_Vetor_Double* vetor = (PGM_Vetor_Double*) PG_GETARG_POINTER (0);
    pgm_destroy_PGM_Vetor_Double(vetor);

    PG_RETURN_INT32( 0 );
}



PG_FUNCTION_INFO_V1(pgm_vector_double_copy);
/*Function: pgm_vector_double_copy
        Duplica um vetor
	Parameter:
        PGM_Vetor_Double *vetor
    Return:
        PGM_Vetor_Double* - vetor duplicado

 */
Datum pgm_vector_double_copy(PG_FUNCTION_ARGS);
Datum pgm_vector_double_copy(PG_FUNCTION_ARGS){
	PGM_Vetor_Double *vector = (PGM_Vetor_Double*) PG_GETARG_POINTER(0);

	MemoryContext contextoAnterior = MemoryContextSwitchTo( CurTransactionContext );
	PGM_Vetor_Double *result = pgm_copy_PGM_Vetor_Double(vector);
	MemoryContextSwitchTo( contextoAnterior );

	PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(pgm_matrix_double_copy);
/*Function: pgm_matrix_double_copy
        Duplica uma matrix
	Parameter:
        PGM_Matrix_Double* matriz
    Return:
        PGM_Matrix_Double* - vetor duplicado
 */
Datum pgm_matrix_double_copy(PG_FUNCTION_ARGS);
Datum pgm_matrix_double_copy(PG_FUNCTION_ARGS){
	PGM_Matriz_Double *m = (PGM_Matriz_Double*) PG_GETARG_POINTER(0);

	MemoryContext contextoAnterior = MemoryContextSwitchTo( CurTransactionContext );
	PGM_Matriz_Double *result = pgm_copy_PGM_Matriz_Double(m);
	MemoryContextSwitchTo( contextoAnterior );

	PG_RETURN_POINTER(result);

}

PG_FUNCTION_INFO_V1(pgm_matrix_get_min_max_mean);
/*Function: pgm_matrix_double_copy
        Informa o elemento minimo, maximo e a media da matriz
	Parameter:
        PGM_Matrix_Double* matriz
    Return:
        double min - retorna o menor elemento da matrix
        double max - retorna o maior elemento da matrix
        double means - retorna a media dos elementos da matrix
 */
Datum pgm_matrix_get_min_max_mean(PG_FUNCTION_ARGS);
Datum pgm_matrix_get_min_max_mean(PG_FUNCTION_ARGS){
	PGM_Matriz_Double *m = (PGM_Matriz_Double*) PG_GETARG_POINTER(0);
	double min = get_min_term(m);
	double max = get_max_term(m);
	double mean = get_matrix_means(m);
	{

		Datum valor[3];
		bool *tup_isnull;
		TupleDesc tupdesc;
		HeapTuple tupla;

		if (get_call_result_type (fcinfo , NULL , &tupdesc) != TYPEFUNC_COMPOSITE)
				elog(ERROR , "Não foi possivel concluir a construção da tupla! reveja os data types");

		valor[0] = Float8GetDatum( min );
		valor[1] = Float8GetDatum( max );
		valor[2] = Float8GetDatum( mean );

		BlessTupleDesc (tupdesc);
		tup_isnull = palloc0 ( tupdesc->natts * sizeof(bool));

		tupla = heap_form_tuple ( tupdesc ,  valor , tup_isnull );

		PG_RETURN_DATUM (HeapTupleGetDatum ( tupla ));
	}
}
