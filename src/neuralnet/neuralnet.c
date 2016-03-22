#include "postgres.h"       //standard
#include "fmgr.h"           //standard
#include "utils/memutils.h"//standard
#include "utils/geo_decls.h"//standard
#include "utils/array.h"    //arrays
#include "utils/errcodes.h" //warning

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../Util/pgm_matriz.h"
#include "../Util/pgm_malloc.h"

#include "neuralnet.h"

//Essa função deve ser adaptada ao novo modelo da RedeNeural
NeuralNet* nn_NeuralNetCreate(unsigned short n_layers,unsigned short function_activation, double bhLambda,double bhT1, double bhT2,
                           unsigned int neurons[],double input_min,double input_max, double output_min, double output_max,
                           double steepness, double bias, double weigths[]){

	unsigned int n_weights, size_byte_nn,i, TotalNeurons;
	MemoryContext contextoAnterior;
	NeuralNet *nn;
	if(n_layers < 2 || n_layers > MAX_LAYERS)
		nn_NeuralNetError("Numero de camadas deve ser entre 2 e 5");

	n_weights = nn_GetNumberWeight( n_layers, neurons );
	TotalNeurons = nn_CalculateTotalNeurons(n_layers,neurons);

	size_byte_nn = sizeof( NeuralNet) + (n_weights+TotalNeurons)*sizeof(double);
    contextoAnterior = MemoryContextSwitchTo( CurTransactionContext );
	nn = (NeuralNet*) pgm_malloc( size_byte_nn );
	MemoryContextSwitchTo( contextoAnterior);

	nn->size = size_byte_nn;

	//SET_VARSIZE(nn , size_byte_nn);

	nn->NLayers = n_layers;
	nn->FunctionActivation = function_activation;
	nn->InputMax = input_max;
	nn->InputMin = input_min;
	nn->OutputMax = output_max;
	nn->OutputMin = output_min;
	nn->Steepness = steepness;
	nn->TotalNeurons = TotalNeurons;
	nn->Bias = bias;
	nn->TotalWeights = n_weights;
	nn->MSE = 0;
	memcpy(nn->NNeurons,neurons,sizeof(int)*n_layers);
	memcpy(nn->Weights,weigths,sizeof(double)*n_weights);
	nn->WeightsPos[0] = 0;
	for ( i=1; i < nn->NLayers-1; i++)
		nn->WeightsPos[i] = nn->WeightsPos[i-1] + (nn->NNeurons[i-1]*nn->NNeurons[i]) + nn->NNeurons[i];
	return nn;
}

void nn_NeuralNetDestroy(NeuralNet *NN){
	pgm_free(NN->Weights);
	pgm_free(NN->WeightsPos);
	pgm_free(NN->NNeurons);
	pgm_free(NN);
}

unsigned int nn_CalculateTotalNeurons(unsigned short Layers, unsigned int Neurons[]){
	unsigned int total = 0,i;
	for (i = 0; i < Layers; i++)
		total += Neurons[i];
	return total;
}

unsigned int nn_MaxNeuronLayer(NeuralNet *NN){
	unsigned int max = NN->NNeurons[0], i;
	for(i = 1; i < NN->NLayers; i++)
		if(max < NN->NNeurons[i])
			max = NN->NNeurons[i];
	return max;
}

unsigned int nn_GetNumberWeight(unsigned short n_layers, unsigned int neurons[]){
	unsigned int i,
	             total = 0;
	for(i = 0; i < n_layers - 1; i++) // A ultima camada não precisa ser contada
		total += (neurons[i]+1)/*o +1 eh referente ao Bias*/ * neurons[i+1];
	return total;
}

unsigned int nn_MaxLayer(NeuralNet *NN){
    int i;
    unsigned int max = NN->NNeurons[0];
    for(i = 1; i < NN->NLayers; i++)
        if(NN->NNeurons[i] > max)
            max = NN->NNeurons[i];
    return max;
}

void nn_NeuralNetError(char *msg){
	elog(ERROR,"%s\n", msg);
}

void nn_NeuralNetWarning(char *msg){
	printf("INFO: %s\n", msg);
}


