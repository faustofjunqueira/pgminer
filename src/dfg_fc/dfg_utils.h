#ifndef DFG_UTILS_H
#define DFG_UTILS_H

#include "utils/memutils.h"//standard
#include "../Util/pgm_matriz.h"
#include "../Util/pgm_matriz_util.h"

/*
	Limita o valor v ao limite superior t ou inferior b
*/
#define CAP(v,t,b) ((v)>=(t)?(t):(v)<=(b)?(b):(v))

/*
	Retorna o menor valor, a ou b
*/
#define MIN(a,b) ((a)<(b)?(a):(b))

/*
	Atalho para criar um contexto persistente no postgres.
*/
#define PROTECTED_CONTEXT(operation) \
    {MemoryContext __oldContext=MemoryContextSwitchTo(CurTransactionContext);\
	operation;MemoryContextSwitchTo(__oldContext);}

/*
	Retorna o valor máximo na coluna column da matriz m
*/
inline double
dfg_pgm_getMaxInColumn(const PGM_Matriz_Double * const m, const int column)
{
    int i;
    double result;
    
    if (m->n_linhas == 0 || m->n_colunas == 0)
        return 0.0;
    
    result = PGM_ELEM_MATRIZ( m, 0, column );
    
    for (i=1;i<m->n_linhas;++i)
        if ( PGM_ELEM_MATRIZ( m, i, column ) > result )
            result = PGM_ELEM_MATRIZ( m, i, column );
    
    return result;
}

/*
	Retorna o produto vetorial entre dois vetores
*/
inline double
dfg_pgm_dotProduct(const PGM_Vetor_Double * const v1,
        const PGM_Vetor_Double * const v2)
{
    double result = 0.0;
    for (int i=0; i<v1->n_elems; ++i)
        result += PGM_ELEM_VETOR( v1, i ) * PGM_ELEM_VETOR( v2, i );
    return result;
}

/*
	Retorna a norma L2 (distancia euclidiana) entre dois vetores (pontos)
*/
inline double 
dfg_pgm_normL2(const PGM_Vetor_Double * const v1)
{
    return sqrt( dfg_pgm_dotProduct( v1, v1 ) );
}

/*
	Retorna o cosseno entre dois vetores
*/
inline double
dfg_pgm_cos(const PGM_Vetor_Double * const v1, const PGM_Vetor_Double * const v2)
{
    const double dotProd = dfg_pgm_dotProduct(v1, v2);
    const double normL2V1 = dfg_pgm_normL2(v1);
    const double normL2V2 = dfg_pgm_normL2(v2);
    return (normL2V1==0.0||normL2V2==0.0)?(0):(dotProd)/(normL2V1*normL2V2);
}

/*
	Retorna a soma de todos os elementos na coluna column da matriz m
*/
inline double
dfg_pgm_columnSum(const PGM_Matriz_Double * const m, const int column)
{
    double sum = 0.0;
    
    for (int i=0;i<m->n_linhas;++i)
        sum += PGM_ELEM_MATRIZ( m, i, column );
    
    return sum;
}

/*
	Retorna a soma de todos os elementos na linha row da matriz m
*/
inline double
dfg_pgm_rowSum(const PGM_Matriz_Double * const m, const int row)
{
    double sum = 0.0;
    
    for (int i=0;i<m->n_colunas;++i)
        sum += PGM_ELEM_MATRIZ( m, row, i );
    
    return sum;
}

/*
	Retorna a soma de todos os elementos na matriz m
*/
inline double
dfg_pgm_sum(const PGM_Matriz_Double * const m)
{
    double sum = 0.0;
    
    for (int i=0;i<m->n_linhas;++i)
        sum += dfg_pgm_rowSum( m, i );
    
    return sum;
}

/*
	Retorna a soma de todos os elementos na diagonal principal da matriz m
*/
inline double
dfg_pgm_trace(const PGM_Matriz_Double * const m)
{
    double sum = 0.0;
    int dim = m->n_linhas>m->n_colunas?m->n_colunas:m->n_linhas;
    
    for (int i=0;i<dim;++i)
        sum += PGM_ELEM_MATRIZ( m, i, i );
    
    return sum;
}

#endif // DFG_UTILS_H
