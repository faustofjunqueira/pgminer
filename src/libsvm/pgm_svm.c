/* SVM
 *
 * Autor: GZS, FFJ
 * GCCBD - COOPE - UFRJ
 */

#include "postgres.h"       //standard
#include "fmgr.h"           //standard
#include "utils/memutils.h"//standard
#include "utils/geo_decls.h"//standard
#include "utils/errcodes.h" //warning

#include "../Util/pgm_matriz.h"
#include "../Util/pgm_matriz_util.h"
#include "../Util/pgm_malloc.h"

#include "svm.h"
#include "svm_train.h"
#include "svm_predict.h"

PG_FUNCTION_INFO_V1(pgm_svm_train);
Datum pgm_svm_train(PG_FUNCTION_ARGS);

Datum pgm_svm_train(PG_FUNCTION_ARGS){

    PGM_Matriz_Double *matrix = (PGM_Matriz_Double*)PG_GETARG_POINTER(0);
    PGM_Vetor_Double *vector = (PGM_Vetor_Double*)PG_GETARG_POINTER(1);

    struct svm_parameter *param = (struct svm_parameter*) pgm_malloc (sizeof(struct svm_parameter));
    struct svm_problem* prob;
    //Cross Validation
    int cross_validation = 0,
        n_fold = PG_GETARG_INT32(14);

    if (n_fold < 2 && n_fold != 0) exit_with_help();
    else if( n_fold >= 2){
        cross_validation = 1;
        elog(ERROR,"CROSS VALIDATION NÃO IMPLEMENTADO");
    }

    //Mount Parameter Struct
    param->svm_type = PG_GETARG_INT32(2);
	param->kernel_type= PG_GETARG_INT32(3);
	param->degree= PG_GETARG_INT32(4);
	param->gamma= PG_GETARG_FLOAT8(5);
	param->coef0= PG_GETARG_FLOAT8(6);
	param->cache_size= PG_GETARG_FLOAT8(7);
	param->eps= PG_GETARG_FLOAT8(8);
	param->C= PG_GETARG_FLOAT8(9);
	param->nr_weight = 0;
	param->weight_label = NULL;
	param->weight = NULL;
	param->nu= PG_GETARG_FLOAT8(10);
	param->p= PG_GETARG_FLOAT8(11);
	param->shrinking= PG_GETARG_INT32(12);
	param->probability= PG_GETARG_INT32(13);

	prob = PGM_Matriz_Double2svm_problem(matrix,vector,param);

	if (cross_validation){
        do_cross_validation(prob,param,n_fold);
        elog(ERROR,"CROSS VALIDATION NÃO IMPLEMENTADO"); // Pergunta ao Filipe sobre isso!
        PG_RETURN_VOID();
	}else{

        MemoryContext contextoAnterior = MemoryContextSwitchTo( CurTransactionContext );
        struct svm_model *model = svm_train(prob,param);
        MemoryContextSwitchTo( contextoAnterior );
        PG_RETURN_POINTER(model);
	}
}

PG_FUNCTION_INFO_V1(pgm_svm_predict);
Datum pgm_svm_predict(PG_FUNCTION_ARGS);

Datum pgm_svm_predict(PG_FUNCTION_ARGS){
    PGM_Matriz_Double *matrix = (PGM_Matriz_Double*) PG_GETARG_POINTER(0);
    PGM_Vetor_Double *vector_label = (PGM_Vetor_Double*) PG_GETARG_POINTER(1);
    struct svm_model *model = (struct svm_model*) PG_GETARG_POINTER(2);
    int predict_probability = PG_GETARG_INT32(3);
    MemoryContext contextoAnterior = MemoryContextSwitchTo( CurTransactionContext );
    PGM_Vetor_Double *predict_vector = pgm_create_PGM_Vetor_Double(vector_label->n_elems);
    MemoryContextSwitchTo( contextoAnterior );
    if(matrix->n_linhas != vector_label->n_elems)
        elog(ERROR,"Numero de labels nao correspondem ao numero de linhas da matrix");

    if(predict_probability){
		if(svm_check_probability_model(model)==0)
			elog(ERROR,"Model does not support probabiliy estimates\n");
	}else
		if(svm_check_probability_model(model)!=0)
			elog(INFO,"Model supports probability estimates, but disabled in prediction.\n");

    predict(matrix,vector_label,model,predict_probability,predict_vector);

    PG_RETURN_POINTER(predict_vector);
}

PG_FUNCTION_INFO_V1(writeMatrix);
Datum writeMatrix(PG_FUNCTION_ARGS);

Datum writeMatrix(PG_FUNCTION_ARGS){
    PGM_Matriz_Double *matrix = (PGM_Matriz_Double*) PG_GETARG_POINTER(0);
    PGM_Vetor_Double *vector = (PGM_Vetor_Double*) PG_GETARG_POINTER(1);
    int i;
    char line[4096];
    char buffer[32];
    for(i = 0; i < vector->n_elems; i++){
        int j;
        memset(line,0,sizeof(char)*4096);
        memset(buffer,0,sizeof(char)*32);
        sprintf(buffer,"%d,",(int)vector->valor[i]);
        strcat(line,buffer);
        for(j = 0; j < matrix->n_colunas-1; j++){
            sprintf(buffer, "%lf,",PGM_ELEM_MATRIZ(matrix,i,j));
            strcat(line,buffer);
        }
        sprintf(buffer, "%lf",PGM_ELEM_MATRIZ(matrix,i,j));
        strcat(line,buffer);
        elog(INFO,"%s",line);
    }
    PG_RETURN_CSTRING("ok");
}

PG_FUNCTION_INFO_V1(pgm_svm_model2charptr);
Datum pgm_svm_model2charptr(PG_FUNCTION_ARGS);

Datum pgm_svm_model2charptr(PG_FUNCTION_ARGS){
    struct svm_model *model = (struct svm_model*) PG_GETARG_POINTER(0);
    MemoryContext contextoAnterior = MemoryContextSwitchTo( CurTransactionContext );
    char *str = svm_model2charptr(model);
    MemoryContextSwitchTo( contextoAnterior );
    PG_RETURN_CSTRING(str);
}

PG_FUNCTION_INFO_V1(pgm_charptr2svm_model);
Datum pgm_charptr2svm_model(PG_FUNCTION_ARGS);

Datum pgm_charptr2svm_model(PG_FUNCTION_ARGS){
    char *ptr = PG_GETARG_CSTRING(0);
    MemoryContext contextoAnterior = MemoryContextSwitchTo( CurTransactionContext );
    struct svm_model *model = charptr2svm_model(ptr);
    MemoryContextSwitchTo( contextoAnterior );
    PG_RETURN_POINTER(model);
}
