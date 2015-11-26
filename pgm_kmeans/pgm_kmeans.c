/* PGM_kmeans
 *
 * Autor: GZS, FFJ
 * GCCBD - COOPE - UFRJ
 * Criacao: 12/01/2012
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

#include "../Util/pgm_matriz.h"
#include "../Util/pgm_matriz_util.h"
#include "pure_c_kmeans.h"

#include <assert.h>

PG_FUNCTION_INFO_V1(pgm_kmeans);
Datum pgm_kmeans(PG_FUNCTION_ARGS);

Datum pgm_kmeans(PG_FUNCTION_ARGS){

    PGM_Matriz_Double *pontos = (PGM_Matriz_Double*) PG_GETARG_INT64(0);
    int k = PG_GETARG_INT32(1);
    int tipo_semente = PG_GETARG_INT32(2);
    int acao_fim_grupo = PG_GETARG_INT32(3);
    double somatorio_distancias = 0;

    MemoryContext contextoAnterior = MemoryContextSwitchTo( CurTransactionContext );
    PGM_Matriz_Double *centroides = pgm_create_matrix_double( k, pontos->n_colunas );
    PGM_Vetor_Int *grupos = pgm_create_PGM_Vetor_Int(pontos->n_linhas);

    MemoryContextSwitchTo( contextoAnterior );


    assert( sizeof( int32 ) == sizeof( int ) );
    assert( sizeof( float8 ) == sizeof( double ) );

    if ( k >= pontos->n_linhas)
        elog(ERROR , "O numero de grupos deve ser menor que o numero de elementos");

    if ( k < 2 )
        elog (ERROR , "No minimo 2 grupos");

    if ( tipo_semente != 2 && tipo_semente != 1)
        elog (ERROR , "O tipo da semente esta indefinida, selecione \n  |1|-para Random Centroides \n  |2|-para Random Pontos");

    if ( acao_fim_grupo != 1 && acao_fim_grupo != 2  && acao_fim_grupo != 3 )
        elog (ERROR , "acao_fim_grupo nao esta definida, \n |1|- para Grupo Secar, Elimine-o \n |2|- para Grupo Secar, Divide o maior em 2 Grupos \n |3|- Se o Grupo Secar, Da Erro");

    if( pure_c_kmeans( pontos, k, tipo_semente, acao_fim_grupo, centroides, grupos->valor,
                       &somatorio_distancias ) ) {
        elog ( ERROR , "Exite(m) Grupo(s) que se encontra(m) vazio(s)");
    }
    {

        Datum valor[3];
        bool *tup_isnull;
        TupleDesc tupdesc;
        HeapTuple tupla;

        if (get_call_result_type (fcinfo , NULL , &tupdesc) != TYPEFUNC_COMPOSITE)
            elog(ERROR , "Não foi possivel concluir a construção da tupla! reveja os data types");

        valor[0] = PointerGetDatum( centroides );
        valor[1] = PointerGetDatum( grupos );
        valor[2] = Float8GetDatum( somatorio_distancias );

        BlessTupleDesc (tupdesc);
        tup_isnull = palloc0 ( tupdesc->natts * sizeof(bool));

        tupla = heap_form_tuple ( tupdesc ,  valor , tup_isnull );

        PG_RETURN_DATUM (HeapTupleGetDatum ( tupla ));
    }
}
