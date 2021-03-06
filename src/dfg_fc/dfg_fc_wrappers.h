#ifndef DFG_FC_WRAPPERS_H
#define DFG_FC_WRAPPERS_H

/*
	
	Descrição:
		Wrappers para encapsular as interfaces do postgres que invoca os 
		códigos em c++.
	
	Autores:
		Diego F. P. de Souza
		Fernanda C. Ribeiro
		Gustavo R. Lima
		
*/

#include "postgres.h"       //standard
#include "fmgr.h"           //standard

Datum dfg_calculateCBD_wrapper( PG_FUNCTION_ARGS );
Datum dfg_calculatePACC_wrapper( PG_FUNCTION_ARGS );
Datum dfg_createProfilesMatrix_wrapper( PG_FUNCTION_ARGS );
Datum dfg_createPCorrectMatrix_wrapper( PG_FUNCTION_ARGS );
Datum dfg_findKNearests_wrapper( PG_FUNCTION_ARGS );
Datum dfg_recommend_wrapper( PG_FUNCTION_ARGS );
Datum dfg_buildCoordinatesQuery_wrapper( PG_FUNCTION_ARGS );
Datum dfg_findKNearests2_wrapper( PG_FUNCTION_ARGS );
Datum dfg_confusionmat_wrapper( PG_FUNCTION_ARGS );
Datum dfg_createMapping_wrapper( PG_FUNCTION_ARGS );

#endif // DFG_FC_WRAPPERS_H

