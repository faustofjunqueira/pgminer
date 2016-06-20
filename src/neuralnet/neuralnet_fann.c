#include "postgres.h"       //standard
#include "fmgr.h"           //standard
#include "utils/memutils.h"//standard
#include "utils/geo_decls.h"//standard
#include "utils/array.h"    //arrays
#include "utils/errcodes.h" //warning

#include "fann/include/doublefann.h"
#include "../Util/pgm_malloc.h"
#include "neuralnet_fann_train.h"
#include "neuralnet_fann_parse.h"
#include "neuralnet.h"
#include "neuralnet_util.h"
#include "neuralnet_fann.h"

NeuralNet *nn_fann_Fann2NeuralNet(struct fann *ANN ){
	unsigned int ANN_NLayers = fann_get_num_layers(ANN),
	             *BiasArray = (unsigned int* ) pgm_malloc (sizeof(unsigned int)*ANN_NLayers),
	             *ANN_NNeurons = (unsigned int* ) pgm_malloc (sizeof(unsigned int)*ANN_NLayers);
	double *ANN_Weights;
	NeuralNet *NN;

	fann_get_bias_array(ANN,BiasArray);

	fann_get_layer_array(ANN,ANN_NNeurons);

	ANN_Weights = nn_fann_parse_get_weights(ANN,ANN_NLayers,ANN_NNeurons);
  NN = nn_NeuralNetCreate(
    ANN_NLayers,
    // Na FANN, cada Neuronio tem uma função de ativação. A neuralnet usa somente a função de ativação do primeiro neuronio, 
    // porem a primeira camada da FANN nao tem função de ativação, logo pegamos da camada seguinte
    (ANN->first_layer+1)->first_neuron->activation_function,
    ANN->BiperbolicLambda,ANN->BiperbolicT1,ANN->BiperbolicT2,
    ANN_NNeurons,
    ANN->input_min,ANN->input_max,ANN->output_min,ANN->output_max,
    // Na FANN, cada Neuronio tem um steepness de ativação. A neuralnet usa somente o steepness de ativação do primeiro neuronio, 
    // porem a primeira camada da FANN nao tem stepness, logo pegamos da camada seguinte
    (ANN->first_layer+1)->first_neuron->activation_steepness,
    // A FANN tem um bias para cada camada da rede, na NeuralNet eh usado somente o bias da primeira camada
    BiasArray[0], 
    ANN_Weights
  );
	NN->MSE = fann_get_MSE(ANN);

	return NN;
}

struct fann *nn_fann_NeuralNet2Fann(NeuralNet *NN){

	struct 	fann_connection 	*ConnectionsANN;
	struct 	fann_connection 	*ConnectionsNN;
    PGM_Vetor_Int Neurons;
    struct fann *ANN;
	MemoryContext contextoAnterior = MemoryContextSwitchTo( CurTransactionContext );

    Neurons.n_elems = NN->NLayers;
    Neurons.valor = (int*) NN->NNeurons;

	ANN = nn_fann_train_create_standard_array(&Neurons,NN->Steepness,NN->FunctionActivation);

	nn_fann_parse_fann_set_scaling_default(ANN);

	MemoryContextSwitchTo( contextoAnterior );

	ConnectionsANN = (struct fann_connection*) pgm_malloc(sizeof(struct fann_connection) * ANN->total_connections);
	ConnectionsNN  = (struct fann_connection*) pgm_malloc(sizeof(struct fann_connection) * ANN->total_connections);

	fann_get_connection_array(ANN,ConnectionsANN);
	nn_fann_parse_nn_get_connection2(NN->NLayers,(int*)NN->NNeurons,ConnectionsNN);

	nn_fann_parse_adjust_get_weightByNeuralNet(NN, ANN, ConnectionsANN,ConnectionsNN);

	nn_fann_parse_fann_set_scaling(ANN,NN->InputMin,NN->InputMax,NN->OutputMin,NN->OutputMax);
	nn_fann_parse_setBihiperbolicParam(ANN,NN->BihiperbolicLambda,NN->BihiperbolicT1,NN->BihiperbolicT2);

	return ANN;
}

