/**
 * Criado 05/12/2013
 * Fausto Ferreira Junqueira
 */

#include "pgm_neuralnet.h"

PG_FUNCTION_INFO_V1(pgm_ArrayType2NN_Fann_Train_Parameter);
Datum pgm_ArrayType2NN_Fann_Train_Parameter(PG_FUNCTION_ARGS);

Datum pgm_ArrayType2NN_Fann_Train_Parameter(PG_FUNCTION_ARGS){

    PG_RETURN_POINTER(ArrayType2NN_Fann_Train_Parameter(PG_GETARG_ARRAYTYPE_P(0)));
}

PG_FUNCTION_INFO_V1(pgm_ArrayType2NN_Fann_Create_Parameter);
Datum pgm_ArrayType2NN_Fann_Create_Parameter(PG_FUNCTION_ARGS);

Datum pgm_ArrayType2NN_Fann_Create_Parameter(PG_FUNCTION_ARGS){

    PG_RETURN_POINTER(ArrayType2NN_Fann_Create_Parameter(PG_GETARG_ARRAYTYPE_P(0)));
}

PG_FUNCTION_INFO_V1(pgm_NeuralNetIn);
Datum pgm_NeuralNetIn(PG_FUNCTION_ARGS);

Datum pgm_NeuralNetIn(PG_FUNCTION_ARGS){
    PG_RETURN_POINTER( nn_Cstring2NeuralNet(PG_GETARG_CSTRING(0)) );
}

PG_FUNCTION_INFO_V1(pgm_NeuralNetOut);
Datum pgm_NeuralNetOut(PG_FUNCTION_ARGS);

Datum pgm_NeuralNetOut(PG_FUNCTION_ARGS){
    NeuralNet *NN;
    PGM_GETARG_POINTER(NN,NeuralNet,0);
    PG_RETURN_CSTRING( nn_NeuralNet2Cstring(NN) );
}

PG_FUNCTION_INFO_V1(pgm_nn_evaluate);
Datum pgm_nn_evaluate(PG_FUNCTION_ARGS);

Datum pgm_nn_evaluate(PG_FUNCTION_ARGS){
	NeuralNet *NN;
	ArrayType *Array = PG_GETARG_ARRAYTYPE_P(1);
	PGM_Vetor_Double Out;
	double *In, *Work;
    unsigned int ArrayElem,
                 NeuronsMaxLayer;

    PGM_GETARG_POINTER(NN,NeuralNet,0);
    NeuronsMaxLayer = nn_MaxLayer(NN);

	if(ARR_ELEMTYPE(PG_GETARG_ARRAYTYPE_P(1)) != FLOAT8OID)
			elog(ERROR, "O array deve ser contituido por float8 (double precision)");

	ArrayElem = ArrayGetNItems( ARR_NDIM(Array) ,ARR_DIMS(Array) );

	if (NN->NNeurons[0] != ArrayElem)
			elog(ERROR, "Numero do array de float é %d e nao corresponde com a primeira camada da neuralnet que é de %d", ArrayElem, NN->NNeurons[0]);

	In = (double*) pgm_malloc (sizeof(double)*(NeuronsMaxLayer+1));
	Work = (double*) pgm_malloc (sizeof(double)*(NeuronsMaxLayer+1));

	memcpy(In,ARR_DATA_PTR (Array), sizeof(double)*ArrayElem);

    nn_NeuralNetRun(NN, &Out,In,Work);

	PG_RETURN_ARRAYTYPE_P(pgm_PGM_Vetor_Double2ArrayType(&Out));

}

PG_FUNCTION_INFO_V1(pgm_nn_inputsize);
Datum pgm_nn_inputsize(PG_FUNCTION_ARGS);

Datum pgm_nn_inputsize(PG_FUNCTION_ARGS){
    NeuralNet *NN;

    PGM_GETARG_POINTER(NN,NeuralNet,0);

    PG_RETURN_INT32(NN->NNeurons[0]);
}

PG_FUNCTION_INFO_V1(pgm_nn_outputsize);
Datum pgm_nn_outputsize(PG_FUNCTION_ARGS);

Datum pgm_nn_outputsize(PG_FUNCTION_ARGS){
    NeuralNet *NN;
    PGM_GETARG_POINTER(NN,NeuralNet,0);
    PG_RETURN_INT32(NN->NNeurons[NN->NLayers - 1]);
}

PG_FUNCTION_INFO_V1(pgm_nn_nlayers);
Datum pgm_nn_nlayers(PG_FUNCTION_ARGS);

Datum pgm_nn_nlayers(PG_FUNCTION_ARGS){
    NeuralNet *NN;
    PGM_Vetor_Int *Result;
    PGM_GETARG_POINTER(NN,NeuralNet,0);
    Result = pgm_Array2PGM_Vetor_Int( NN->NLayers,(int*) NN->NNeurons );
    PG_RETURN_ARRAYTYPE_P(pgm_PGM_Vetor_Int2ArrayType(Result));
}

PG_FUNCTION_INFO_V1(pgm_nn_set_scaling);
Datum pgm_nn_set_scaling(PG_FUNCTION_ARGS);

Datum pgm_nn_set_scaling(PG_FUNCTION_ARGS){
	NeuralNet *NN;
	double input_min = PG_GETARG_FLOAT8(1);
	double input_max = PG_GETARG_FLOAT8(2);
	double output_min = PG_GETARG_FLOAT8(3);
	double output_max = PG_GETARG_FLOAT8(4);
	PGM_GETARG_POINTER(NN,NeuralNet,0);

	NN->InputMin = input_min;
	NN->InputMax = input_max;
	NN->OutputMin = output_min;
	NN->OutputMax = output_max;
	PG_RETURN_POINTER(NN);
}

PG_FUNCTION_INFO_V1(pgm_nn_get_scaling);
Datum pgm_nn_get_scaling(PG_FUNCTION_ARGS);

Datum pgm_nn_get_scaling(PG_FUNCTION_ARGS){
	NeuralNet *NN;
    PGM_GETARG_POINTER(NN,NeuralNet,0);
	{
    Datum *valor = palloc ( 4 * sizeof(Datum));

    int4 tuplen;
    bool *isnull;
    TupleDesc tupdesc;
    HeapTuple tupla;

		valor[0] = Float8GetDatumFast(NN->InputMin);
		valor[1] = Float8GetDatum(NN->InputMax);
		valor[2] = Float8GetDatum(NN->OutputMin);
		valor[3] = Float8GetDatum(NN->OutputMax);

    if (get_call_result_type (fcinfo , NULL , &tupdesc) != TYPEFUNC_COMPOSITE)
            elog(ERROR , "Não foi possivel concluir a construção da tupla! reveja os data types");

    BlessTupleDesc (tupdesc);
    tuplen = tupdesc->natts;

    isnull = palloc0 ( tuplen * sizeof(bool));

    tupla = heap_form_tuple ( tupdesc ,  valor , isnull );

    PG_RETURN_DATUM (HeapTupleGetDatum ( tupla ));
	}
}

PG_FUNCTION_INFO_V1(pgm_nn_split);
Datum pgm_nn_split(PG_FUNCTION_ARGS);

Datum pgm_nn_split(PG_FUNCTION_ARGS){
	NeuralNet *nn,**out;
	int n_split = PG_GETARG_INT32(1);
	MemoryContext contextoAnterior;

	PGM_GETARG_POINTER(nn,NeuralNet,0);
	contextoAnterior = MemoryContextSwitchTo( CurTransactionContext );
	out= nn_NeuralnetSplit(nn, n_split);
	MemoryContextSwitchTo( contextoAnterior );


	{
		Datum *result = palloc ( 2 * sizeof(Datum));

    int4 tuplen;
    bool *isnull;
    TupleDesc tupdesc;
    HeapTuple tuple;

		result[0] = PointerGetDatum(out[0]);
		result[1] = PointerGetDatum(out[1]);

    if (get_call_result_type (fcinfo , NULL , &tupdesc) != TYPEFUNC_COMPOSITE)
            elog(ERROR , "Não foi possivel concluir a construção da tupla! reveja os data types");

    BlessTupleDesc (tupdesc);

    tuplen = tupdesc->natts;

    isnull = palloc0 ( tuplen * sizeof(bool));

    tuple = heap_form_tuple ( tupdesc ,  result , isnull );

    PG_RETURN_DATUM (HeapTupleGetDatum ( tuple ));
	}
}

PG_FUNCTION_INFO_V1(pgm_nn_merge);
Datum pgm_nn_merge(PG_FUNCTION_ARGS);

Datum pgm_nn_merge(PG_FUNCTION_ARGS){
	NeuralNet *nnA = (NeuralNet*) PG_GETARG_POINTER(0),
						*nnB = (NeuralNet*) PG_GETARG_POINTER(1),
						*result;
    MemoryContext contextoAnterior;

    PGM_GETARG_POINTER(nnA,NeuralNet,0);
    PGM_GETARG_POINTER(nnB,NeuralNet,1);

	contextoAnterior = MemoryContextSwitchTo( CurTransactionContext );
	result = nn_NeuralNetMerge(nnA,nnB);
	MemoryContextSwitchTo( contextoAnterior );

	PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(pgm_nn_get_MSE);
Datum pgm_nn_get_MSE(PG_FUNCTION_ARGS);

Datum pgm_nn_get_MSE(PG_FUNCTION_ARGS){
	NeuralNet *NN;

	PGM_GETARG_POINTER(NN,NeuralNet,0);

	PG_RETURN_FLOAT8(NN->MSE);
}

PG_FUNCTION_INFO_V1(pgm_nn_set_MSE);
Datum pgm_nn_set_MSE(PG_FUNCTION_ARGS);

Datum pgm_nn_set_MSE(PG_FUNCTION_ARGS){
	NeuralNet *NN;
	float8 new_mse = PG_GETARG_FLOAT8(1);

    PGM_GETARG_POINTER(NN,NeuralNet,0);

	NN->MSE = new_mse;

	PG_RETURN_FLOAT8(NN->MSE);
}

PG_FUNCTION_INFO_V1(pgm_nn_equal);
Datum pgm_nn_equal(PG_FUNCTION_ARGS);

Datum pgm_nn_equal(PG_FUNCTION_ARGS){
    NeuralNet *nnA, *nnB;
    PGM_GETARG_POINTER(nnA,NeuralNet,0);
    PGM_GETARG_POINTER(nnB,NeuralNet,1);

	PG_RETURN_BOOL(nn_NeuralNetEqual(nnA,nnB));
}

PG_FUNCTION_INFO_V1(pgm_nn_get_weight_array);
Datum pgm_nn_get_weight_array(PG_FUNCTION_ARGS);

Datum pgm_nn_get_weight_array(PG_FUNCTION_ARGS){
	NeuralNet *nn;
	int layer = PG_GETARG_INT32(1);
	int i, n_weight;
	MemoryContext contextoAnterior;
	PGM_Matriz_Double *matrix;

	PGM_GETARG_POINTER(nn,NeuralNet,0);

    n_weight = (nn->NNeurons[layer] + 1/*bias*/)*nn->NNeurons[layer+1];

	if(nn->NLayers == layer+1 || layer > 5 || layer < 0) elog(ERROR, "Camada indisponivel");

	contextoAnterior = MemoryContextSwitchTo( CurTransactionContext );
	matrix = pgm_create_matrix_double(nn->NNeurons[layer] + 1, nn->NNeurons[layer+1]);
	MemoryContextSwitchTo( contextoAnterior );

	for(i=0; i < n_weight; i++){
		matrix->valor[i] = *(nn->Weights+nn->WeightsPos[layer]+i);
	}

	PG_RETURN_POINTER(matrix);
}

PG_FUNCTION_INFO_V1(pgm_nn_set_weight_array);
Datum pgm_nn_set_weight_array(PG_FUNCTION_ARGS);

Datum pgm_nn_set_weight_array(PG_FUNCTION_ARGS){
	NeuralNet *nn;
	PGM_Matriz_Double *matrix =  (PGM_Matriz_Double*) PG_GETARG_POINTER(1);
	int layer = PG_GETARG_INT32(2), i , n_weights;

    PGM_GETARG_POINTER(nn,NeuralNet,0);

	if(nn->NLayers == layer+1 || layer > 5 || layer < 0) elog(ERROR, "Camada indisponivel");

	if(nn->NNeurons[layer]+1 != matrix->n_linhas || nn->NNeurons[layer+1] != matrix->n_colunas) elog(ERROR, "Estrutura da matriz nao é compativel com a estrutura da Neuralnet");

    n_weights  = (nn->NNeurons[layer] + 1/*bias*/)*nn->NNeurons[layer+1];

	for(i = 0; i < n_weights; i++){
		*(nn->Weights+nn->WeightsPos[layer]+i) = matrix->valor[i];
	}

	PG_RETURN_POINTER(nn);
}

PG_FUNCTION_INFO_V1(pgm_nn_get_distance);
Datum pgm_nn_get_distance(PG_FUNCTION_ARGS);

Datum pgm_nn_get_distance(PG_FUNCTION_ARGS){
    NeuralNet *nnA,*nnB;
    PGM_GETARG_POINTER(nnA,NeuralNet,0);
    PGM_GETARG_POINTER(nnB,NeuralNet,1);
	PG_RETURN_FLOAT8(nn_GetDistance(nnA, nnB));
}

PG_FUNCTION_INFO_V1(pgm_bi_hiperbolic);
Datum pgm_bi_hiperbolic(PG_FUNCTION_ARGS);

Datum pgm_bi_hiperbolic(PG_FUNCTION_ARGS){
    double value = PG_GETARG_FLOAT8(0),
           lambda = PG_GETARG_FLOAT8(1),
           t1 = PG_GETARG_FLOAT8(2),
           t2 = PG_GETARG_FLOAT8(3);

    Datum valor[2];
    bool *tup_isnull;
    TupleDesc tupdesc;
    HeapTuple tupla;

    if (get_call_result_type (fcinfo , NULL , &tupdesc) != TYPEFUNC_COMPOSITE)
        elog(ERROR , "Não foi possivel concluir a construção da tupla! reveja os data types");

    nn_setBiHiperbolicParameter(lambda,t1,t2);

    valor[0] = Float8GetDatum( nn_BiHiperbolic(value) );
    valor[1] = Float8GetDatum( nn_DerivateBiHiperbolic(value) );

    BlessTupleDesc (tupdesc);
    tup_isnull = palloc0 ( tupdesc->natts * sizeof(bool));

    tupla = heap_form_tuple ( tupdesc ,  valor , tup_isnull );

    PG_RETURN_DATUM (HeapTupleGetDatum ( tupla ));

}


PG_FUNCTION_INFO_V1(pgm_create_train_data);
Datum pgm_create_train_data(PG_FUNCTION_ARGS);

Datum pgm_create_train_data(PG_FUNCTION_ARGS){
	PGM_Matriz_Double *matrix_in,*matrix_out;
	struct fann_train_data *data;
	MemoryContext contextoAnterior;

	PGM_GETARG_POINTER(matrix_in,PGM_Matriz_Double,0);
	PGM_GETARG_POINTER(matrix_out,PGM_Matriz_Double,1);

    contextoAnterior = MemoryContextSwitchTo( CurTransactionContext );
    data = nn_fann_train_create_fann_train_data(matrix_in, matrix_out);

    MemoryContextSwitchTo( contextoAnterior );

	PG_RETURN_POINTER(data);
}

PG_FUNCTION_INFO_V1(pgm_free_train_data);
Datum pgm_free_train_data(PG_FUNCTION_ARGS);

Datum pgm_free_train_data(PG_FUNCTION_ARGS){
    struct fann_train_data *data;
    PGM_GETARG_POINTER(data,struct fann_train_data,0);
    nn_fann_train_destroi_fann_train_data( data );

	PG_RETURN_INT32(0);
}

PG_FUNCTION_INFO_V1(pgm_train_neuralnet1);
Datum pgm_train_neuralnet1(PG_FUNCTION_ARGS);

Datum pgm_train_neuralnet1(PG_FUNCTION_ARGS){
	struct fann_train_data *data;
	PGM_Vetor_Int     *hidden    = pgm_ArrayType2PGM_Vetor_Int(PG_GETARG_ARRAYTYPE_P(1));
	double steepness =  PG_GETARG_FLOAT8(3);
	int max_epochs =  PG_GETARG_INT64(4);
	int epochs_between_reports = PG_GETARG_INT64(5);
	double desired_error = PG_GETARG_FLOAT8(6);
    short int functionActivation = PG_GETARG_INT32(2);

    PGM_GETARG_POINTER(data,struct fann_train_data,0);

    if(functionActivation >= NN_FUNCTION_ACTIVATION ){
        int i;
        elog(INFO,"Use:");
        for(i = 0; i < NN_FUNCTION_ACTIVATION; i++)
            elog(INFO,"%d: %s",i,NN_ACTIVATIONFUNC_NAMES[i]);
        elog(ERROR,"Função inserida nao correspondente");
    }

	PG_RETURN_POINTER(
        nn_fann_train1(data, hidden, functionActivation,
                       steepness,max_epochs,epochs_between_reports,desired_error)
    );
}

PG_FUNCTION_INFO_V1(pgm_train_neuralnet11);
Datum pgm_train_neuralnet11(PG_FUNCTION_ARGS);

Datum pgm_train_neuralnet11(PG_FUNCTION_ARGS){
	struct fann_train_data *data = (struct fann_train_data *) PG_GETARG_POINTER(0);
	PGM_Vetor_Int     *hidden    = pgm_ArrayType2PGM_Vetor_Int(PG_GETARG_ARRAYTYPE_P(1));
	double steepness =  PG_GETARG_FLOAT8(3);
	int max_epochs =  PG_GETARG_INT64(4);
	int epochs_between_reports = PG_GETARG_INT64(5);
	double desired_error = PG_GETARG_FLOAT8(6);

    short int functionActivation = nn_FindFunctionActivation(PG_GETARG_CSTRING(2));

    PGM_GETARG_POINTER(data,struct fann_train_data,0);

    if(functionActivation == 0xFFFF ){
        int i;
        elog(INFO,"Use:");
        for(i = 0; i < NN_FUNCTION_ACTIVATION; i++)
            elog(INFO,"%d: %s",i,NN_ACTIVATIONFUNC_NAMES[i]);
        elog(ERROR,"Função inserida nao correspondente");
    }

	PG_RETURN_POINTER(
        nn_fann_train1(data, hidden, functionActivation,
                       steepness,max_epochs,epochs_between_reports,desired_error)
    );
}

PG_FUNCTION_INFO_V1(pgm_train_neuralnet2);
Datum pgm_train_neuralnet2(PG_FUNCTION_ARGS);

Datum pgm_train_neuralnet2(PG_FUNCTION_ARGS){
	struct fann_train_data *data;
	double steepness =  PG_GETARG_FLOAT8(2);
	int max_epochs =  PG_GETARG_INT64(3);
	int epochs_between_reports = PG_GETARG_INT64(4);
	double desired_error = PG_GETARG_FLOAT8(5);

    int functionActivation = PG_GETARG_INT32(1);

    PGM_GETARG_POINTER(data,struct fann_train_data,0);

    if(functionActivation >= NN_FUNCTION_ACTIVATION ){
        int i;
        elog(INFO,"Use:");
        for(i = 0; i < NN_FUNCTION_ACTIVATION; i++)
            elog(INFO,"%d: %s",i,NN_ACTIVATIONFUNC_NAMES[i]);
        elog(ERROR,"Função inserida nao correspondente");
    }

	PG_RETURN_POINTER(
        nn_fann_train2(data, steepness, functionActivation,
                        max_epochs, epochs_between_reports,desired_error)
    );
}

PG_FUNCTION_INFO_V1(pgm_train_neuralnet21);
Datum pgm_train_neuralnet21(PG_FUNCTION_ARGS);

Datum pgm_train_neuralnet21(PG_FUNCTION_ARGS){
	struct fann_train_data *data;
	double steepness =  PG_GETARG_FLOAT8(2);
	int max_epochs =  PG_GETARG_INT64(3);
	int epochs_between_reports = PG_GETARG_INT64(4);
	double desired_error = PG_GETARG_FLOAT8(5);

    short int functionActivation = nn_FindFunctionActivation(PG_GETARG_CSTRING(1));

    PGM_GETARG_POINTER(data,struct fann_train_data,0);

    if(functionActivation == 0xFFFF ){
        int i;
        elog(INFO,"Use:");
        for(i = 0; i < NN_FUNCTION_ACTIVATION; i++)
            elog(INFO,"%d: %s",i,NN_ACTIVATIONFUNC_NAMES[i]);
        elog(ERROR,"Função inserida nao correspondente");
    }

	PG_RETURN_POINTER(
        nn_fann_train2(data, steepness, functionActivation,
                        max_epochs, epochs_between_reports,desired_error)
    );
}


PG_FUNCTION_INFO_V1(pgm_train_neuralnet3);
Datum pgm_train_neuralnet3(PG_FUNCTION_ARGS);

Datum pgm_train_neuralnet3(PG_FUNCTION_ARGS){
	struct fann_train_data *data;
	struct fann *ann = (struct fann *) PG_GETARG_POINTER(1);
	int max_epochs =  PG_GETARG_INT64(2);
	int epochs_between_reports = PG_GETARG_INT64(3);
	double desired_error = PG_GETARG_FLOAT8(4);

    PGM_GETARG_POINTER(data,struct fann_train_data,0);

    nn_fann_train3(data,ann,max_epochs,epochs_between_reports,desired_error);

	PG_RETURN_FLOAT8(fann_get_MSE(ann));
}

PG_FUNCTION_INFO_V1(pgm_train_neuralnet4);
Datum pgm_train_neuralnet4(PG_FUNCTION_ARGS);

Datum pgm_train_neuralnet4(PG_FUNCTION_ARGS){
	struct fann_train_data *data;
	NN_Fann_Create_Parameter *create_param;
	NN_Fann_Train_Parameter *train_param;
	PGM_Vetor_Int     *hidden    = pgm_ArrayType2PGM_Vetor_Int(PG_GETARG_ARRAYTYPE_P(1));

    PGM_GETARG_POINTER(data,struct fann_train_data,0);
    PGM_GETARG_POINTER(create_param,NN_Fann_Create_Parameter,2);
    PGM_GETARG_POINTER(train_param,NN_Fann_Train_Parameter,3);

	PG_RETURN_POINTER(nn_fann_train4(data, hidden,create_param,train_param));
}

PG_FUNCTION_INFO_V1(pgm_train_neuralnet_bihiperbolic);
Datum pgm_train_neuralnet_bihiperbolic(PG_FUNCTION_ARGS);

Datum pgm_train_neuralnet_bihiperbolic(PG_FUNCTION_ARGS){
	struct fann_train_data *data;
	PGM_Vetor_Int     *hidden    = pgm_ArrayType2PGM_Vetor_Int(PG_GETARG_ARRAYTYPE_P(1));
	double steepness =  PG_GETARG_FLOAT8(3);
	int max_epochs =  PG_GETARG_INT64(5);
	int epochs_between_reports = PG_GETARG_INT64(6);
	double desired_error = PG_GETARG_FLOAT8(7);
    double lam = PG_GETARG_FLOAT8(2);
    double t1 = PG_GETARG_FLOAT8(3);
    double t2 = PG_GETARG_FLOAT8(4);

    PGM_GETARG_POINTER(data,struct fann_train_data,0);

    PG_RETURN_POINTER(
           nn_fann_train_bihiperbolic(data,hidden,lam, t1,t2,steepness,
                                      max_epochs,epochs_between_reports,desired_error)
    );
}

PG_FUNCTION_INFO_V1(pgm_fann_test);
Datum pgm_fann_test(PG_FUNCTION_ARGS);

Datum pgm_fann_test(PG_FUNCTION_ARGS){
	struct fann_train_data *data = (struct fann_train_data *) PG_GETARG_POINTER(0);
	struct fann *ann = (struct fann *) PG_GETARG_POINTER(1);
	MemoryContext contextoAnterior = MemoryContextSwitchTo( CurTransactionContext );

	fann_test_data(ann,data);

	MemoryContextSwitchTo( contextoAnterior );

	PG_RETURN_FLOAT8(fann_get_MSE(ann));
}

PG_FUNCTION_INFO_V1(pgm_fann_run);
Datum pgm_fann_run(PG_FUNCTION_ARGS);

Datum pgm_fann_run(PG_FUNCTION_ARGS){
	struct fann *ann = (struct fann *) PG_GETARG_POINTER(0);
	ArrayType *array = PG_GETARG_ARRAYTYPE_P(1);
	PGM_Vetor_Double *input = pgm_ArrayType2PGM_Vetor_Double(array);
	unsigned int *layers = (unsigned int*) pgm_malloc (sizeof(unsigned int)*MAX_LAYERS);
	PGM_Vetor_Double *out;
	int i, n_input = fann_get_num_input(ann), n_output = fann_get_num_output(ann);

	fann_get_layer_array(ann,layers);

	if(ARR_ELEMTYPE(PG_GETARG_ARRAYTYPE_P(1)) != FLOAT8OID)
		elog(ERROR, "O array deve ser contituido por float8 (double precision)");

	if (layers[0] != ArrayGetNItems( ARR_NDIM(array) ,ARR_DIMS(array)))
    elog(ERROR, "Numero do array de float é %d e nao corresponde com a primeira camada da neuralnet que é de %d", ArrayGetNItems( ARR_NDIM(array) ,ARR_DIMS(array)), layers[0]);

	if(ann->input_min != 0 || ann->input_max != 1){
		for(i = 0; i < n_input; i++) input->valor[i] = nn_norm(ann->input_min,ann->input_max,input->valor[i]);
	}

	out = pgm_Array2PGM_Vetor_Double(n_output ,(double*) fann_run(ann,(fann_type *) input->valor));

	PG_RETURN_ARRAYTYPE_P(pgm_PGM_Vetor_Double2ArrayType(out));
}

PG_FUNCTION_INFO_V1(pgm_nn_fann2neuralnet);
Datum pgm_nn_fann2neuralnet(PG_FUNCTION_ARGS);

Datum pgm_nn_fann2neuralnet(PG_FUNCTION_ARGS){
	PG_RETURN_POINTER(nn_fann_Fann2NeuralNet((struct fann*) PG_GETARG_POINTER(0)));
}

PG_FUNCTION_INFO_V1(pgm_nn_neuralnet2fann);
Datum pgm_nn_neuralnet2fann(PG_FUNCTION_ARGS);

Datum pgm_nn_neuralnet2fann(PG_FUNCTION_ARGS){
	PG_RETURN_POINTER(nn_fann_NeuralNet2Fann((NeuralNet*) PG_GETARG_POINTER(0)));
}

PG_FUNCTION_INFO_V1(pgm_nn_info);
Datum pgm_nn_info(PG_FUNCTION_ARGS);

Datum pgm_nn_info(PG_FUNCTION_ARGS){
    nn_NeuralNetInfo((NeuralNet*) PG_GETARG_POINTER(0));
    PG_RETURN_VOID();
}
