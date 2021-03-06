#include "../dfg_fc/dfg_fc.h"

#include <math.h>
#include <string>
#include <map>

#include "topk.h"
#include "dfg_utils.h"
#include <ANN/ANN.h>

using namespace std;

/*
	Calcula o percentual de acerto do sistema a partir do vetor
	correto (expected) e do previsto pelo sistema (predicted).
*/
float8
dfg_calculatePACC(PGM_Vetor_Double *predicted, PGM_Vetor_Double *expected)
{
    int i;
    float8 result = 0.0;

    if (predicted->n_elems != expected->n_elems) {
        ereport( ERROR, ( errcode( ERRCODE_INVALID_PARAMETER_VALUE ),
                 errmsg( "Dimensions not matching" ) ) );
        return 0.0;
    }

    for (i=0; i<predicted->n_elems; ++i)
        if ( PGM_ELEM_VETOR( predicted, i ) == PGM_ELEM_VETOR( expected, i ) )
            result += 1.0;

    return result / predicted->n_elems;
}

/*
	Calcula o score CBD do sistema a partir do vetor correto (expected) e do
	previsto pelo sistema (predicted). Esta é a métrica utilizada pelo kaggle
	para o problema "What o you know?".
*/
float8
dfg_calculateCBD(PGM_Vetor_Double *predicted, PGM_Vetor_Double *expected)
{
    int i;
    float8 result = 0.0;

    if (predicted->n_elems != expected->n_elems) {
        ereport( ERROR, ( errcode( ERRCODE_INVALID_PARAMETER_VALUE ),
                 errmsg( "Dimensions not matching" ) ) );
        return 0.0;
    }

    for (i=0; i<predicted->n_elems; ++i) {
        const double p = CAP( PGM_ELEM_VETOR( predicted, i ), 0.99, 0.01 );
        const double e = PGM_ELEM_VETOR( expected, i );
        result += e*log10(p) + (1-e)*log10(1-p);
    }

    return -result/predicted->n_elems;
}

/*
	Cria uma matriz de profiles para os estudantes.

	inputs: Matriz de tamanho Nx2. A primeira coluna deve ser o id do estudante
			e a segunda o id da questão.
	mapI: Mapeia o id do usuário para a linha da matriz de profiles
	mapJ: Mapeia o id da questão para a coluna da matriz de profiles
*/
PGM_Matriz_Double *
dfg_createProfilesMatrix( PGM_Matriz_Double * inputs, PGM_Vetor_Double * mapI, PGM_Vetor_Double * mapJ )
{
    int i;
    int lines, columns;
    PGM_Matriz_Double * presences;

    lines = mapI->n_elems;
    columns = mapJ->n_elems;

    PROTECTED_CONTEXT (
        presences = pgm_create_matrix_double( lines, columns );
    );

    map<int, int> mi;
    for (int i=0;i<mapI->n_elems;++i) {
        mi[round( PGM_ELEM_VETOR( mapI, i ) )] = i;
    }

    map<int, int> mj;
    for (int i=0;i<mapJ->n_elems;++i) {
        mj[round( PGM_ELEM_VETOR( mapJ, i ) )] = i;
    }

    for (i=0; i<inputs->n_linhas; ++i) {
        const int i2 = mi[round( PGM_ELEM_MATRIZ( inputs, i, 0 ) )];
        const int j2 = mj[round( PGM_ELEM_MATRIZ( inputs, i, 1 ) )];
        PGM_ELEM_MATRIZ( presences, i2, j2 ) = 1;
    }

    return presences;
}

/*
	Cria a matriz com os percentuais de acerto de cada estudante para cada
	questão. Se o estudante nunca respondeu a uma questão seu valor é -1.

	inputs: Matriz de tamanho Nx2. A primeira coluna deve ser o id do estudante
			e a segunda o id da questão.
	targets: 1 ou 0 se o estudante acertou ou não a n-esima linha da matriz
			inputs.
	mapI: Mapeia o id do usuário para a linha da matriz de profiles
	mapJ: Mapeia o id da questão para a coluna da matriz de profiles
*/
PGM_Matriz_Double *
dfg_createPCorrectMatrix( PGM_Matriz_Double * inputs, PGM_Vetor_Double * targets,
        PGM_Vetor_Double * mapI, PGM_Vetor_Double * mapJ )
{
    int i, j;
    int lines, columns;
    PGM_Matriz_Double * correctness;
    PGM_Matriz_Double * frequencies;

    if (inputs->n_linhas != targets->n_elems) {
        ereport( ERROR, ( errcode( ERRCODE_INVALID_PARAMETER_VALUE ),
                 errmsg( "Dimensions not matching" ) ) );
        return NULL;
    }

    lines = mapI->n_elems;
    columns = mapJ->n_elems;

    PROTECTED_CONTEXT (
        frequencies = pgm_create_matrix_double( lines, columns );
        correctness = pgm_create_matrix_double( lines, columns );
    );

    map<int, int> mi;
    for (int i=0;i<mapI->n_elems;++i) {
        mi[round( PGM_ELEM_VETOR( mapI, i ) )] = i;
    }

    map<int, int> mj;
    for (int i=0;i<mapJ->n_elems;++i) {
        mj[round( PGM_ELEM_VETOR( mapJ, i ) )] = i;
    }

    for (i=0; i<inputs->n_linhas; ++i) {
        const int i2 = mi[round( PGM_ELEM_MATRIZ( inputs, i, 0 ) )];
        const int j2 = mj[round( PGM_ELEM_MATRIZ( inputs, i, 1 ) )];
        const int t = PGM_ELEM_VETOR( targets, i );

        PGM_ELEM_MATRIZ( correctness, i2, j2 ) += t;
        PGM_ELEM_MATRIZ( frequencies, i2, j2 ) += 1;
    }

    for (i=0; i<lines; ++i) {
        for (j=0; j<columns; ++j) {
            if (PGM_ELEM_MATRIZ( frequencies, i, j ) == 0) {
                PGM_ELEM_MATRIZ( correctness, i, j ) = -1;
            }
            else {
                PGM_ELEM_MATRIZ( correctness, i, j ) =
                        PGM_ELEM_MATRIZ( correctness, i, j ) /
                        PGM_ELEM_MATRIZ( frequencies, i, j );
            }
        }
    }

    return correctness;
}

/*
	Retorna os k vizinhos mais próximos para cada ponto de consulta. O resultado
	é a concatenação vertical de matrizes de dimensão Kx2, onde a primeira
	coluna corresponde ao indice da linha do vizinho na matriz coordinates e a
	segunda correponde a sua distancia invertida.
	Utiliza a distância euclidiana e a KDTree.

	coordinates: Coordenadas dos pontos candidatos a mais próximos.
	coordinates_query: Coordenadas dos pontos de consulta
*/
PGM_Matriz_Double *
dfg_findKNearests( double eps, int numNearests,
        PGM_Matriz_Double * coordinates, PGM_Matriz_Double * coordinates_query )
{
    int                    nPts;                     // actual number of data points
    ANNpointArray          dataPts;                  // data points
    ANNpoint               queryPt;                  // query point
    ANNidxArray            nnIdx;                    // near neighbor indices
    ANNdistArray           dists;                    // near neighbor distances
    ANNkd_tree*            kdTree;

    int i, j;
    int columns = coordinates->n_colunas;
    int lines = coordinates->n_linhas;

    queryPt = annAllocPt(columns);            // allocate query point
    dataPts = annAllocPts(lines, columns);    // allocate data points
    nnIdx = new ANNidx[numNearests];          // allocate near neigh indices
    dists = new ANNdist[numNearests];         // allocate near neighbor dists
    nPts = lines;                             // read data points

    for (i=0;i<coordinates->n_linhas;++i) {
        for (j=0;j<coordinates->n_colunas;++j) {
            dataPts[i][j] = PGM_ELEM_MATRIZ( coordinates, i, j );
        }
    }

    kdTree = new ANNkd_tree(                     // build search structure
                    dataPts,                     // the data points
                    nPts,                        // number of points
                    columns);                    // dimension of space

    PGM_Matriz_Double * results;

    PROTECTED_CONTEXT (
        results = pgm_create_matrix_double( coordinates_query->n_linhas * numNearests, 2 );
    );

    for (i=0;i<coordinates_query->n_linhas;++i) {      // Para cada item de entrada
        for (j=0;j<coordinates_query->n_colunas;++j) {
            queryPt[j] = PGM_ELEM_MATRIZ( coordinates_query, i, j );
        }

        kdTree->annkSearch(                       // search
                queryPt,                          // query point
                numNearests,                      // number of near neighbors
                nnIdx,                            // nearest neighbors (returned)
                dists,                            // distance (returned)
                eps);                             // error bound

        for (j=0;j<numNearests;++j) {
            PGM_ELEM_MATRIZ( results, (i*numNearests+j), 0 ) = nnIdx[j];

            // Distancia invertida
            PGM_ELEM_MATRIZ( results, (i*numNearests+j), 1 ) =
                    1.0-dists[j]/(4*coordinates_query->n_colunas);

            // Distancia comum
            //PGM_ELEM_MATRIZ( results, (i*numNearests+j), 1 ) = dists[j];
        }
    }

    delete [] nnIdx;                              // clean things up
    delete [] dists;
    delete kdTree;
    annClose();

    return results;
}

/*
	Retorna os k vizinhos mais próximos para cada ponto de consulta. O resultado
	é a concatenação vertical de matrizes de dimensão Kx2, onde a primeira
	coluna corresponde ao indice da linha do vizinho na matriz coordinates e a
	segunda correponde a sua distancia invertida.
	Utiliza a distância de cosseno.

	coordinates: Coordenadas dos pontos candidatos a mais próximos.
	coordinates_query: Coordenadas dos pontos de consulta
*/
PGM_Matriz_Double *
dfg_findKNearests2( int numNearests, PGM_Matriz_Double * coordinates, PGM_Matriz_Double * coordinates_query )
{
    int i, j;
    PGM_Matriz_Double * results;

    if (coordinates->n_colunas != coordinates_query->n_colunas) {
        ereport( ERROR, ( errcode( ERRCODE_INVALID_PARAMETER_VALUE ),
                 errmsg( "Dimensions not matching" ) ) );
        return NULL;
    }

    if (numNearests > coordinates->n_linhas) {
        ereport( ERROR, ( errcode( ERRCODE_INVALID_PARAMETER_VALUE ),
                 errmsg( "numNearests is bigger than coordinates" ) ) );
        return NULL;
    }

    PROTECTED_CONTEXT (
        results = pgm_create_matrix_double( coordinates_query->n_linhas * numNearests, 2 );
    );

    PGM_Vetor_Double v1;
    PGM_Vetor_Double v2;
    TopK<int> topk( numNearests );

    v1.n_elems = coordinates->n_colunas;
    v2.n_elems = coordinates->n_colunas;

    for (i=0;i<coordinates_query->n_linhas;++i) {             // Para cada consulta
        v1.valor = PGM_LINHA_MATRIZ( coordinates_query, i );

        for (int i3=0;i3<coordinates->n_linhas;++i3) {        // Para cada linha
            v2.valor = PGM_LINHA_MATRIZ( coordinates, i3 );
            topk.push( dfg_pgm_cos( &v1, &v2 ), i3 );
        }

        int id;
        double w;
        for (j=0;j<numNearests;++j) {
            topk.pop( w, id );
            PGM_ELEM_MATRIZ( results, (i*numNearests+j), 0 ) = id;
            PGM_ELEM_MATRIZ( results, (i*numNearests+j), 1 ) = w;
        }
    }

    return results;
}

/*
	Realiza a recomendação utilizando os k vizinhos mais próximos e seus
	percentuais de acerto.
*/
PGM_Vetor_Double *
dfg_recommend( PGM_Matriz_Double * knearests, PGM_Matriz_Double * pCorrect,
        PGM_Matriz_Double * inputs, PGM_Vetor_Double * mapJ )
{
    int i, j;
    PGM_Vetor_Double * predictions;
    int k = knearests->n_linhas / inputs->n_linhas;

    PROTECTED_CONTEXT (
        predictions = pgm_create_PGM_Vetor_Double( inputs->n_linhas );
    );

    map<int, int> mj;
    for (int i=0;i<mapJ->n_elems;++i) {
        mj[round( PGM_ELEM_VETOR( mapJ, i ) )] = i;
    }

    for (i=0; i<inputs->n_linhas; ++i) {
        const int j2 = mj[round( PGM_ELEM_MATRIZ( inputs, i, 1 ) )];

        int id;
        double w;
        double rate;
        double numerador = 0.0;
        double denominador = 0.0;
        for (j=0; j<k; ++j) {
            id = round( PGM_ELEM_MATRIZ( knearests, (i*k+j), 0 ) );
            w = PGM_ELEM_MATRIZ( knearests, (i*k+j), 1 );
            rate = PGM_ELEM_MATRIZ( pCorrect, id, j2 );

            if (rate >= -0.5) {
                numerador += w * rate;
                denominador += w;
            }
        }

        if (denominador == 0.0)
            PGM_ELEM_VETOR( predictions, i ) = 1;
        else
            PGM_ELEM_VETOR( predictions, i ) = round( numerador / denominador );
    }

    return predictions;
}

/*
	Constrói a matriz de coordenadas de consultas.

	coordinates: A matriz de coordenadas (profiles) de cada estudante.
	inputs: Matriz de dimensão Nx2 referente ao conjunto que desejamos prever.
	        A primeira coluna deve conter o id do estudante e a segunda o id da
	        questão.
    mapI: Mapeia o id do estudante a uma linha na matriz de coodenadas.
*/
PGM_Matriz_Double *
dfg_buildCoordinatesQuery( PGM_Matriz_Double * coordinates, PGM_Matriz_Double * inputs, PGM_Vetor_Double * mapI )
{
    PGM_Matriz_Double * coordinates_query;

    PROTECTED_CONTEXT (
        coordinates_query = pgm_create_matrix_double( inputs->n_linhas, coordinates->n_colunas );
    );

    map<int, int> mi;
    for (int i=0;i<mapI->n_elems;++i) {
        mi[round( PGM_ELEM_VETOR( mapI, i ) )] = i;
    }

    for (int i=0;i<inputs->n_linhas;++i) {
        int i2 = PGM_ELEM_MATRIZ( inputs, i, 0 );

        if (mi.find(i2) == mi.end()) {
            for (int j=0;j<coordinates->n_colunas;++j)
                PGM_ELEM_MATRIZ( coordinates_query, i, j ) = 1.0;
        }
        else {
            i2 = mi[i2];
            for (int j=0;j<coordinates->n_colunas;++j)
                PGM_ELEM_MATRIZ( coordinates_query, i, j ) =
                        PGM_ELEM_MATRIZ( coordinates, i2, j );
        }
    }

    return coordinates_query;
}

/*
	Gera a matriz de confusão do sistema a partir do vetor correto (expected)
	e do previsto pelo sistema (predicted).
*/
PGM_Matriz_Double *
dfg_confusionmat(PGM_Vetor_Double * targets, PGM_Vetor_Double * predictions)
{
    if (targets->n_elems != predictions->n_elems) {
        ereport( ERROR, ( errcode( ERRCODE_INVALID_PARAMETER_VALUE ),
                 errmsg( "Dimensions not matching" ) ) );
        return NULL;
    }

    PGM_Matriz_Double * m;

    PROTECTED_CONTEXT (
        m = pgm_create_matrix_double( 3, 3 );
    );

    for (int k=0;k<targets->n_elems;++k) {
        int i = PGM_ELEM_VETOR( predictions, k );
        int j = PGM_ELEM_VETOR( targets, k );
        PGM_ELEM_MATRIZ( m, i, j ) += 1;
    }

    PGM_ELEM_MATRIZ( m, 2, 2 ) = dfg_pgm_trace( m ) / dfg_pgm_sum( m );

    PGM_ELEM_MATRIZ( m, 0, 2 ) = PGM_ELEM_MATRIZ( m, 0, 0 ) / dfg_pgm_rowSum( m, 0 );
    PGM_ELEM_MATRIZ( m, 1, 2 ) = PGM_ELEM_MATRIZ( m, 1, 1 ) / dfg_pgm_rowSum( m, 1 );

    PGM_ELEM_MATRIZ( m, 2, 0 ) = PGM_ELEM_MATRIZ( m, 0, 0 ) / dfg_pgm_columnSum( m, 0 );
    PGM_ELEM_MATRIZ( m, 2, 1 ) = PGM_ELEM_MATRIZ( m, 1, 1 ) / dfg_pgm_columnSum( m, 1 );

    return m;
}

/*
	Cria uma mapeamento de ids esparsos para ids densos.
	O resultado é um vetor onde cada posição corresponde ao novo id (denso) e o
	seu conteúdo é o id antigo (esparso).

	inputs: A matriz que contem os ids esparsos.
	column: A coluna da matriz inputs que contém os ids esparsos.
*/
PGM_Vetor_Double *
dfg_createMapping( PGM_Matriz_Double * inputs, int column )
{
    PGM_Vetor_Double * mapping;

    int newId = 0;
    map<int, int> m;
    for (int i=0;i<inputs->n_linhas;++i) {
        const int i2 = PGM_ELEM_MATRIZ( inputs, i, column );
        if (m.find( i2 ) == m.end()) {
            m[i2] = newId++;
        }
    }

    PROTECTED_CONTEXT (
        mapping = pgm_create_PGM_Vetor_Double( newId );
    );

    for (std::map<int,int>::const_iterator it=m.begin();it!=m.end();++it) {
        PGM_ELEM_VETOR( mapping, it->second ) = it->first;
    }

    return mapping;
}

