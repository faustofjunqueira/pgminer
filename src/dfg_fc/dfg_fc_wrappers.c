#include "dfg_fc_wrappers.h"
#include "dfg_fc.h"

PG_FUNCTION_INFO_V1( dfg_calculatePACC_wrapper );
Datum dfg_calculatePACC_wrapper( PG_FUNCTION_ARGS )
{
    PGM_Vetor_Double *p = (PGM_Vetor_Double*) PG_GETARG_POINTER(0);
    PGM_Vetor_Double *e = (PGM_Vetor_Double*) PG_GETARG_POINTER(1);
    PG_RETURN_FLOAT8( dfg_calculatePACC( p, e ) );
}

PG_FUNCTION_INFO_V1( dfg_calculateCBD_wrapper );
Datum dfg_calculateCBD_wrapper( PG_FUNCTION_ARGS )
{
    PGM_Vetor_Double *p = (PGM_Vetor_Double*) PG_GETARG_POINTER(0);
    PGM_Vetor_Double *e = (PGM_Vetor_Double*) PG_GETARG_POINTER(1);
    PG_RETURN_FLOAT8( dfg_calculateCBD( p, e ) );
}

PG_FUNCTION_INFO_V1( dfg_createProfilesMatrix_wrapper );
Datum dfg_createProfilesMatrix_wrapper( PG_FUNCTION_ARGS )
{
    PGM_Matriz_Double *inputs = (PGM_Matriz_Double*) PG_GETARG_POINTER(0);
    PGM_Vetor_Double *mapI = (PGM_Vetor_Double*) PG_GETARG_POINTER(1);
    PGM_Vetor_Double *mapJ = (PGM_Vetor_Double*) PG_GETARG_POINTER(2);
    PG_RETURN_POINTER( dfg_createProfilesMatrix( inputs, mapI, mapJ ) );
}

PG_FUNCTION_INFO_V1( dfg_createPCorrectMatrix_wrapper );
Datum dfg_createPCorrectMatrix_wrapper( PG_FUNCTION_ARGS )
{
    PGM_Matriz_Double *inputs = (PGM_Matriz_Double*) PG_GETARG_POINTER(0);
    PGM_Vetor_Double *targets = (PGM_Vetor_Double*) PG_GETARG_POINTER(1);
    PGM_Vetor_Double *mapI = (PGM_Vetor_Double*) PG_GETARG_POINTER(2);
    PGM_Vetor_Double *mapJ = (PGM_Vetor_Double*) PG_GETARG_POINTER(3);
    PG_RETURN_POINTER( dfg_createPCorrectMatrix( inputs, targets, mapI, mapJ ) );
}

PG_FUNCTION_INFO_V1( dfg_findKNearests_wrapper );
Datum dfg_findKNearests_wrapper( PG_FUNCTION_ARGS )
{
    float8 eps = PG_GETARG_FLOAT8(0);
    int32 numNearests = PG_GETARG_INT32(1);
    PGM_Matriz_Double *coordinates = (PGM_Matriz_Double*) PG_GETARG_POINTER(2);
    PGM_Matriz_Double *coordinates_query = (PGM_Matriz_Double*) PG_GETARG_POINTER(3);
    PG_RETURN_POINTER( dfg_findKNearests( eps, numNearests, coordinates, coordinates_query ) );
}

PG_FUNCTION_INFO_V1( dfg_recommend_wrapper );
Datum dfg_recommend_wrapper( PG_FUNCTION_ARGS )
{
    PGM_Matriz_Double *knearests = (PGM_Matriz_Double*) PG_GETARG_POINTER(0);
    PGM_Matriz_Double *pCorrect = (PGM_Matriz_Double*) PG_GETARG_POINTER(1);
    PGM_Matriz_Double *inputs = (PGM_Matriz_Double*) PG_GETARG_POINTER(2);
    PGM_Vetor_Double  *mapJ = (PGM_Vetor_Double*) PG_GETARG_POINTER(3);
    PG_RETURN_POINTER( dfg_recommend( knearests, pCorrect, inputs, mapJ ) );
}

PG_FUNCTION_INFO_V1( dfg_buildCoordinatesQuery_wrapper );
Datum dfg_buildCoordinatesQuery_wrapper( PG_FUNCTION_ARGS )
{
    PGM_Matriz_Double *coordinates = (PGM_Matriz_Double*) PG_GETARG_POINTER(0);
    PGM_Matriz_Double *queryInputs = (PGM_Matriz_Double*) PG_GETARG_POINTER(1);
    PGM_Vetor_Double *mapI = (PGM_Vetor_Double*) PG_GETARG_POINTER(2);
    PG_RETURN_POINTER( dfg_buildCoordinatesQuery( coordinates, queryInputs, mapI ) );
}

PG_FUNCTION_INFO_V1( dfg_findKNearests2_wrapper );
Datum dfg_findKNearests2_wrapper( PG_FUNCTION_ARGS )
{
    int32 numNearests = PG_GETARG_INT32(0);
    PGM_Matriz_Double *coordinates = (PGM_Matriz_Double*) PG_GETARG_POINTER(1);
    PGM_Matriz_Double *coordinates_query = (PGM_Matriz_Double*) PG_GETARG_POINTER(2);
    PG_RETURN_POINTER( dfg_findKNearests2( numNearests, coordinates, coordinates_query ) );
}

PG_FUNCTION_INFO_V1( dfg_confusionmat_wrapper );
Datum dfg_confusionmat_wrapper( PG_FUNCTION_ARGS )
{
    PGM_Vetor_Double *targets = (PGM_Vetor_Double*) PG_GETARG_POINTER(0);
    PGM_Vetor_Double *predictions = (PGM_Vetor_Double*) PG_GETARG_POINTER(1);
    PG_RETURN_POINTER( dfg_confusionmat( targets, predictions ) );
}

PG_FUNCTION_INFO_V1( dfg_createMapping_wrapper );
Datum dfg_createMapping_wrapper( PG_FUNCTION_ARGS )
{
    PGM_Matriz_Double *inputs = (PGM_Matriz_Double*) PG_GETARG_POINTER(0);
    int32 column = PG_GETARG_INT32(1);
    PG_RETURN_POINTER( dfg_createMapping( inputs, column ) );
}

