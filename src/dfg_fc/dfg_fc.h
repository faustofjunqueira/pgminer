#ifndef DFG_FC_H
#define DFG_FC_H

/*
	
	Descrição:
		Códigos para resolver o problema "What do you know?".
	
	Autores:
		Diego F. P. de Souza
		Fernanda C. Ribeiro
		Gustavo R. Lima
		
*/

#ifdef __cplusplus
extern "C" {
#endif


#include "../Util/pgm_matriz.h"
#include "../Util/pgm_matriz_util.h"


float8 dfg_calculateCBD( 
        PGM_Vetor_Double *predicted, PGM_Vetor_Double *expected );

float8 dfg_calculatePACC( 
        PGM_Vetor_Double *predicted, PGM_Vetor_Double *expected );

PGM_Matriz_Double * dfg_createProfilesMatrix( 
        PGM_Matriz_Double * inputs, 
        PGM_Vetor_Double * mapI, PGM_Vetor_Double * mapJ );

PGM_Matriz_Double * dfg_createPCorrectMatrix( 
        PGM_Matriz_Double * inputs, PGM_Vetor_Double * targets, 
        PGM_Vetor_Double * mapI, PGM_Vetor_Double * mapJ );

PGM_Matriz_Double * dfg_findKNearests( 
        double eps, int numNearests, PGM_Matriz_Double * coordinates, 
        PGM_Matriz_Double * coordinates_query );

PGM_Vetor_Double * dfg_recommend( PGM_Matriz_Double * knearests, 
        PGM_Matriz_Double * pCorrect, PGM_Matriz_Double * inputs, 
        PGM_Vetor_Double * mapJ );

PGM_Matriz_Double * dfg_buildCoordinatesQuery( 
        PGM_Matriz_Double * coordinates, PGM_Matriz_Double * inputs,
        PGM_Vetor_Double * mapI );

PGM_Matriz_Double * dfg_findKNearests2( 
        int numNearests, PGM_Matriz_Double * coordinates, 
        PGM_Matriz_Double * coordinates_query );

PGM_Matriz_Double * dfg_confusionmat( 
        PGM_Vetor_Double * targets, PGM_Vetor_Double * predictions );

PGM_Vetor_Double * dfg_createMapping( 
        PGM_Matriz_Double * inputs, int column );


#ifdef __cplusplus
}
#endif

#endif // DFG_FC_H
