/****PGMINER INCLUDES******/
#include "postgres.h"        //standard
#include "fmgr.h"            //standard
#include "utils/memutils.h"  //standard
#include "utils/geo_decls.h" //standard
#include "utils/errcodes.h"  //warning
#include "catalog/pg_type.h"// OID
#include "utils/lsyscache.h"// construi array type
#include "funcapi.h"        // trabalhar com row
#include "access/heapam.h"  //Construção de Tupla

#include "../Util/pgm_matriz.h"
#include "../Util/pgm_matriz_util.h"
#include "../Util/pgm_malloc.h"

#include "pgm_som_util.h"


PG_FUNCTION_INFO_V1(pgm_som);
Datum pgm_som(PG_FUNCTION_ARGS);

/*
CREATE OR REPLACE FUNCTION pgm_som(data bigint, neuronios bigint,Dstart integer,iter_start integer, eta_start double precision,Dend integer, iter_end integer, eta_end float8, conscience boolean,out neuronios bigint, out cluster bigint )
  RETURNS record AS
'$libdir/pgminer.so', 'pgm_som'
  LANGUAGE c VOLATILE STRICT
  COST 1;
*/
//O Retorno eh uma matriz com os pesos de todos os nodes!
Datum pgm_som(PG_FUNCTION_ARGS){
    MemoryContext contextoAnterior;
    PGM_Matriz_Double *out;
    PGM_Vetor_Int *cluster_final;
    PGM_Matriz_Double *data =(PGM_Matriz_Double*) PG_GETARG_POINTER(0); //VAI SER MATRIZ
    PGM_Vetor_Int *neuronios = (PGM_Vetor_Int*) PG_GETARG_POINTER(1);
    //----------------------------------
    int INDIM = data->n_colunas;
    int nInputs =  data->n_linhas;
    int Dstart = PG_GETARG_INT32(2);
    int iter_start = PG_GETARG_INT32(3);
    double eta_start = PG_GETARG_FLOAT8(4);
    int Dend = PG_GETARG_INT32(5);
    int iter_end = PG_GETARG_INT32(6);
    double eta_end = PG_GETARG_FLOAT8(7);
    //----------------------------------

    int i,j,k;
    double menor_distancia,distancia_atual;
	int conscience = (int) PG_GETARG_BOOL(8);

    struct network *p = (struct network*) pgm_malloc (sizeof(struct network));
	p->indim = INDIM;	/* # input dimensions */
	p->netdim = neuronios->n_elems;		/* # network dimensions */

    if( (p->num_nodes = (int *)pgm_malloc(p->netdim * sizeof(int))) == NULL)
	{
	    deallocate_network(p);
		elog(ERROR,"Nao foi possivel alocar memoria para num_nodes");
	}
    p->totalnodes = 1;
	for( i=0; i<p->netdim; i++)
	{
	    p->num_nodes[i] = neuronios->valor[i];
	    p->totalnodes *= p->num_nodes[i];
	}

    p->nodes = (struct node *)pgm_malloc(p->totalnodes * sizeof(struct node));
	if( p->nodes == NULL )
	{	deallocate_network(p);
		elog(ERROR,"Nao foi possivel alocar memoria para nodes");
	}
	for( i=0; i<p->totalnodes; i++)
	{	p->nodes[i].weight = NULL;
		p->nodes[i].pos = NULL;
		p->nodes[i].num_hits = 0;
	}
	for( i=0; i<p->totalnodes; i++ )
	{   p->nodes[i].weight = (double *)pgm_malloc(p->indim * sizeof(double));
	    p->nodes[i].pos = (int *)pgm_malloc(p->netdim * sizeof(int));
	    if( p->nodes[i].weight == NULL || p->nodes[i].pos == NULL )
	    {	deallocate_network(p);
            elog(ERROR,"Nao foi possivel alocar memoria para pesos");
	    }
	}

	/* The training parameters */
	p->Dstart = Dstart; p->Dend = Dend;
	p->iter_start = iter_start; p->iter_end = iter_end;
	p->eta_start = eta_start; p->eta_end = eta_end;
	p->conscience = conscience;		/* Is the conscience mech. active? */

	/* The inputs */
	p->num_inputs = nInputs;		/* Number of inputs */

	p->inputs = (struct input_s *)pgm_malloc(p->num_inputs*sizeof(struct input_s));
	if( p->inputs == NULL )
	{	deallocate_network(p);
		elog(ERROR,"Nao foi possivel alocar memoria para inputs");
	}
	for( i=0; i<p->num_inputs; i++)
	    p->inputs[i].input = NULL;
	for( i=0; i<p->num_inputs; i++ )
	{   p->inputs[i].input = (double *)pgm_malloc(p->indim * sizeof(double));
	    if( p->inputs[i].input == NULL )
	    {	deallocate_network(p);
            elog(ERROR,"Nao foi possivel alocar memoria para inputs");
	    }
	}

    for( i=0; i<p->num_inputs; i++)
	   for( j=0; j<p->indim; j++)
                p->inputs[i].input[j] = PGM_ELEM_MATRIZ(data,i,j);


    contextoAnterior = MemoryContextSwitchTo( CurTransactionContext );
    out = run_SOM(p);
    cluster_final = pgm_create_PGM_Vetor_Int(data->n_linhas);

    for( i=0; i< cluster_final->n_elems; i++){
        double *registro = PGM_LINHA_MATRIZ(data,i);
        j = 0;
        menor_distancia = get_euclidean(registro,p->nodes[j].weight,data->n_colunas);
        cluster_final->valor[i] = j;
        for(j = 1; j < p->totalnodes; j++){
            distancia_atual = get_euclidean(registro,p->nodes[j].weight,data->n_colunas);
            if(menor_distancia > distancia_atual){
                menor_distancia = distancia_atual;
                cluster_final->valor[i] = j;
            }
        }
    }

    for(k=0; k< cluster_final->n_elems; k++){
        cluster_final->valor[k]++;
    }

    MemoryContextSwitchTo( contextoAnterior );
    {

        Datum valor[2];
        bool *tup_isnull;
        TupleDesc tupdesc;
        HeapTuple tupla;

        if (get_call_result_type (fcinfo , NULL , &tupdesc) != TYPEFUNC_COMPOSITE)
            elog(ERROR , "Não foi possivel concluir a construção da tupla! reveja os data types");

        valor[0] = PointerGetDatum( out );
        valor[1] = PointerGetDatum( cluster_final );

        BlessTupleDesc (tupdesc);
        tup_isnull = palloc0 ( tupdesc->natts * sizeof(bool));

        tupla = heap_form_tuple ( tupdesc ,  valor , tup_isnull );

        PG_RETURN_DATUM (HeapTupleGetDatum ( tupla ));
    }

}

