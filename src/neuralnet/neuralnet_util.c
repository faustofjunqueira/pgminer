#include <stdio.h>
#include <stdlib.h>

#include <math.h>

#include "../Util/pgm_matriz.h"
#include "../Util/pgm_matriz_util.h"
#include "../Util/pgm_malloc.h"

#include "neuralnet.h"
#include "neuralnet_util.h"
#include "neuralnet_parse.h"

NeuralNet *nn_Cstring2NeuralNet(char* st){

  NeuralNet *nn;
	unsigned short n_layers = 0, function_activation;

	unsigned int *neurons,
	             n_weights = 0,
	             bias,
	             TotalNeurons = 0;
	//Scaling Escalas Default
	double input_min = INPUT_MIN_DEFAULT_VALUE,
		input_max = INPUT_MAX_DEFAULT_VALUE,
		output_min = OUTPUT_MIN_DEFAULT_VALUE,
		output_max = OUTPUT_MAX_DEFAULT_VALUE,
		steepness = STEEPNESS_DEFAULT_VALUE,
		bhLambda,bhT1,bhT2,
		*weights;

	nn_parse_NeuralNetInitParseError(st);

	n_layers = nn_parse_ReadNumberLayers(&st);

	if (n_layers == 0xFFFF)
		nn_parse_NeuralNetParseError("Numero de camadas invalida",st);

	neurons = nn_parse_ReadNeurons( &st, n_layers);

	function_activation = nn_parse_ReadFunctionActivation(&st,&bhLambda,&bhT1,&bhT2);

	nn_parse_ReadScaling(&st, &input_min, &input_max, &output_min, &output_max);

	steepness = nn_parse_ReadSteepness(&st, function_activation);

	bias = nn_parse_ReadBias(&st);

	n_weights = nn_GetNumberWeight( n_layers, neurons );

	TotalNeurons = nn_CalculateTotalNeurons(n_layers,neurons);

	weights = nn_parse_ReadWeight(&st,n_weights);

  nn = nn_NeuralNetCreate(n_layers,function_activation,bhLambda,bhT1,bhT2,neurons,input_min,input_max,output_min,output_max,steepness,bias,weights);

	return nn;
}

//Essa função deve ser adaptada ao novo modelo da RedeNeural
char *nn_NeuralNet2Cstring(NeuralNet *nn){
	unsigned int i;
	char *out, *st;

	out = (char*) pgm_malloc ( 80 + (nn->TotalWeights + nn->TotalNeurons) * 30);
	st = out;

	st += sprintf(st,"%u#",nn->NLayers);
	for(i = 0; i < nn->NLayers; i++)
		st += sprintf(st,"%u ",nn->NNeurons[i]);
    if(nn->FunctionActivation == NN_BIHIPERBOLIC)
        st += sprintf(st,"#%s %lf %lf %lf#",NN_ACTIVATIONFUNC_NAMES[nn->FunctionActivation],nn->BihiperbolicLambda,nn->BihiperbolicT1,nn->BihiperbolicT2);
    else
        st += sprintf(st,"#%s#",NN_ACTIVATIONFUNC_NAMES[nn->FunctionActivation]);
	st += sprintf(st,"%lf %lf %lf %lf#%lf#%u#",nn->InputMin,nn->InputMax,nn->OutputMin,nn->OutputMax,nn->Steepness,nn->Bias);

	for (i = 0; i < nn->TotalWeights; i++)
		st += sprintf(st,"%lf ",nn->Weights[i]);
	return out;
}

//Essa função deve ser adaptada ao novo modelo da RedeNeural
void nn_NeuralNetInfo(NeuralNet *nn){
	char *st = (char*) pgm_malloc(sizeof(char)*(nn->TotalNeurons*nn_GetNumberWeight(nn->NLayers,nn->NNeurons) + 1024)),
	     *pt;
	int i;
	pt = st;
	st += sprintf(st,"====================================\n");
	st += sprintf(st,"nlayers %u\nn_neurons: ", nn->NLayers);
	for(i = 0; i < nn->NLayers; i++)
		st += sprintf(st,"<%u> ", nn->NNeurons[i]);
	st += sprintf(st,"\ninput_min: %lf\n", nn->InputMin);
	st += sprintf(st,"input_max: %lf\n", nn->InputMax);
	st += sprintf(st,"output_min: %lf\n", nn->OutputMin);
	st += sprintf(st,"output_max: %lf\n", nn->OutputMax);
	st += sprintf(st,"function activation: %s (%u)\n", NN_ACTIVATIONFUNC_NAMES[nn->FunctionActivation], nn->FunctionActivation);

    if(nn->FunctionActivation == NN_BIHIPERBOLIC)
        st += sprintf(st,"\t bhParameter:\n\t\tlambda: %lf\n\t\tt1: %lf\n\t\tt2: %lf", nn->BihiperbolicLambda,nn->BihiperbolicT1,nn->BihiperbolicT2);

	st += sprintf(st,"Bias %u\n", nn->Bias);
	st += sprintf(st,"Steepness %lf\n", nn->Steepness);
	st += sprintf(st,"Total Neurons %u\n",nn->TotalNeurons);
	st += sprintf(st,"Total Weights %u\n",nn->TotalWeights);
	st += sprintf(st,"WeightsPos\n\t");
	for (i = 0; i < nn->NLayers-1; ++i)
		st += sprintf(st,"<%u> ",nn->WeightsPos[i]);
	st += sprintf(st,"\nWeights\n");
	for(i = 0; i < nn->TotalWeights;i++)
		st += sprintf(st,"\t%lf\n", nn->Weights[i]);

	st += sprintf(st,"String:\n");
	st += sprintf(st,"%s\n",nn_NeuralNet2Cstring(nn));

	st += sprintf(st,"====================================\n");
	elog(INFO,"%s\n", pt);
}

void nn_NeuralNetRun( NeuralNet *NN,  PGM_Vetor_Double *Out, double *In, double *Work){

	int layer, neuron_it,neuron_next;

	// Normalizando a entrada
	if(NN->InputMin != 0 || NN->InputMax != 1){
		for(neuron_it = 0; neuron_it < NN->NNeurons[0]; neuron_it++){
			In[neuron_it] = nn_norm(NN->InputMin,NN->InputMax,In[neuron_it]);
		}
	}
	
	for(layer = 0; layer < NN->NLayers-1; layer++){
		elog(INFO, "Camanda %d", layer);
		memset(Work,0,sizeof(double)*NN->NNeurons[layer+1]); // Limpando o vetor
		
		for(neuron_next = 0; neuron_next < NN->NNeurons[layer+1]; neuron_next++){
			for(neuron_it = 0; neuron_it < NN->NNeurons[layer]; neuron_it++){
				elog(INFO, "Peso: %d %d %d %lf",layer,neuron_it,neuron_next,nn_GetWeight(NN,layer,neuron_it,neuron_next));
				Work[neuron_next] += In[neuron_it] * nn_GetWeight(NN,layer,neuron_it,neuron_next);
			}
			Work[neuron_next] += NN->Bias * nn_GetBiasWeight(NN,layer,neuron_next);
			nn_activation_switch(NN->FunctionActivation, Work[neuron_next],Work[neuron_next]);
		}
		elog(INFO, "-------------");

		//trocando os ponteiros
		In=(double*)((long)In^(long)Work);
		Work=(double*)((long)In^(long)Work);
		In=(double*)((long)In^(long)Work);
	}

	Out->n_elems = NN->NNeurons[NN->NLayers-1];
	Out->valor = In; // se analisar bem vera que o valor final terminou no In

	if(NN->OutputMin != 0 || NN->OutputMax != 1){
		for(neuron_it = 0; neuron_it < NN->NNeurons[NN->NLayers-1]; neuron_it++){
			Out->valor[neuron_it] = nn_denorm(NN->OutputMin,NN->OutputMax,Out->valor[neuron_it]);
		}
	}

}

NeuralNet **nn_NeuralnetSplit(NeuralNet *NN, unsigned int SplitLayer){
	NeuralNet **Out = (NeuralNet**) pgm_malloc(sizeof(NeuralNet*) * 2);

	if(NN->NLayers < 3)
		nn_NeuralNetError("Não é possivel fazer split nessa rede, numero minimo de camadas são 3.");

	if(SplitLayer == 0 || SplitLayer == NN->NLayers-1)
		nn_NeuralNetError("Não é possivel fazer split, so é possivel fazer split nas camadas intermediarias");

	if(SplitLayer >= NN->NLayers)
		nn_NeuralNetError("Não é possivel fazer split, camada indisponivel");

	Out[0] = nn_NeuralNetCreate(1+SplitLayer,
	                            NN->FunctionActivation, NN->BihiperbolicLambda,NN->BihiperbolicT1,NN->BihiperbolicT2,
	                            NN->NNeurons,
	                            NN->InputMin,NN->InputMax,0.0,1.0,
	                            NN->Steepness,NN->Bias,
	                            NN->Weights
	                            );

	Out[1] = nn_NeuralNetCreate(NN->NLayers-SplitLayer,
	                            NN->FunctionActivation,NN->BihiperbolicLambda,NN->BihiperbolicT1,NN->BihiperbolicT2,
	                            NN->NNeurons+SplitLayer,
	                            0.0,1.0,NN->OutputMin,NN->OutputMax,
	                            NN->Steepness,NN->Bias,
	                            NN->Weights+NN->WeightsPos[SplitLayer]
	                            );

	return Out;
}

NeuralNet *nn_NeuralNetMerge(NeuralNet *NeuralA, NeuralNet *NeuralB){
	unsigned int MergeNNeurons[MAX_LAYERS];
	double *MergeWeights;

	if(NeuralA->OutputMin != 0.0 || NeuralA->OutputMax != 1.0 || NeuralB->InputMin != 0.0 || NeuralB->InputMax != 1.0)
		nn_NeuralNetError("Escala nao compativel!");

	if( NeuralA->NLayers + NeuralB->NLayers - 1 > 5)
		nn_NeuralNetError("O numero de camada execede o maximo permitido.");

	if( NeuralA->NNeurons[NeuralA->NLayers-1] != NeuralB->NNeurons[0])
		nn_NeuralNetError("O numero de neuronios da ultima camada de A nao corresponde com a primeira camada de B");

	if(NeuralA->FunctionActivation != NeuralB->FunctionActivation)
		nn_NeuralNetError("Funções de Ativação não compativeis.");
    else if(NeuralA->FunctionActivation == NN_BIHIPERBOLIC)
            if(NeuralA->BihiperbolicLambda != NeuralB->BihiperbolicLambda || NeuralA->BihiperbolicT1 != NeuralB->BihiperbolicT1 ||NeuralA->BihiperbolicT2 != NeuralB->BihiperbolicT2)
                nn_NeuralNetError("Parametros da função bihiperbolic são incompativeis.");

	if(NeuralA->Bias != NeuralB->Bias)
		nn_NeuralNetError("Valores de Bias não compativeis.");

	if(NeuralA->Steepness != NeuralB->Steepness)
		nn_NeuralNetError("Valores de Steepness não compativeis.");


	memcpy(MergeNNeurons,NeuralA->NNeurons,sizeof(unsigned int)*NeuralA->NLayers);

	memcpy(MergeNNeurons+NeuralA->NLayers,NeuralB->NNeurons + 1,sizeof(unsigned int)*NeuralB->NLayers - 1);


	MergeWeights = (double*) pgm_malloc (sizeof(double)*nn_GetNumberWeight(NeuralA->NLayers+NeuralB->NLayers-1, MergeNNeurons));

	memcpy(MergeWeights,NeuralA->Weights,sizeof(double)*NeuralA->TotalWeights);

	memcpy(MergeWeights+NeuralA->TotalWeights,NeuralB->Weights,sizeof(double)*NeuralB->TotalWeights);

	return nn_NeuralNetCreate(NeuralA->NLayers + NeuralB->NLayers - 1,
	                          NeuralA->FunctionActivation,NeuralA->BihiperbolicLambda,NeuralA->BihiperbolicT1,NeuralA->BihiperbolicT2,
	                          MergeNNeurons,
	                          NeuralA->InputMin,NeuralA->InputMax,NeuralB->OutputMin,NeuralB->OutputMax,
	                          NeuralA->Steepness,NeuralA->Bias,
	                          MergeWeights);
}

char nn_NeuralNetEqual(NeuralNet *NeuralA,NeuralNet *NeuralB){
	int i;
	if(NeuralA->NLayers != NeuralB->NLayers)
		return 0;

	if(NeuralA->FunctionActivation != NeuralB->FunctionActivation)
		return 0;
    else if(NeuralA->FunctionActivation == NN_BIHIPERBOLIC)
            if(NeuralA->BihiperbolicLambda != NeuralB->BihiperbolicLambda || NeuralA->BihiperbolicT1 != NeuralB->BihiperbolicT1 ||NeuralA->BihiperbolicT2 != NeuralB->BihiperbolicT2)
                nn_NeuralNetError("Parametros da função bihiperbolic são incompativeis.");


	if(NeuralA->Bias != NeuralB->Bias)
		return 0;

	if(NeuralA->Steepness != NeuralB->Steepness)
		return 0;

	if(NeuralA->InputMin != NeuralB->InputMin ||
	   NeuralA->InputMax != NeuralB->InputMax ||
	   NeuralA->OutputMin != NeuralB->OutputMin ||
	   NeuralA->OutputMax != NeuralB->OutputMax)
		return 0;

	for(i = 0; i < NeuralA->NLayers; i++)
		if(NeuralA->NNeurons[i] != NeuralB->NNeurons[i])
			return 0;

	for(i = 0; i < NeuralA->TotalWeights;i++)
		if(NeuralA->Weights[i] != NeuralB->Weights[i])
			return 0;

	return 1;
}

double nn_GetDistance(NeuralNet *nnA, NeuralNet *nnB){
	double result = 0.0;
	int i, n_weights = 0;

	if(nnA->NLayers != nnB->NLayers)
		nn_NeuralNetError("Numero de camada são diferentes.");

	for(i = 0; i < nnA->NLayers; i++){
		if(nnA->NNeurons[i] != nnB->NNeurons[i])
			nn_NeuralNetError("Numeros de neuronios da camada são diferentes.");
	}

	n_weights = nn_GetNumberWeight(nnA->NLayers, nnA->NNeurons);
	for(i = 0; i < n_weights; i++){
		result += (nnA->Weights[i] - nnB->Weights[i]) * (nnA->Weights[i] - nnB->Weights[i]);
	}
	return sqrt(result);
}

short nn_GetFunctionActivationByName(char *st){
	if(st){
      short i;
      for(i = 0; i < NN_FUNCTION_ACTIVATION; i++){
      	if(!strcmp(st,NN_ACTIVATIONFUNC_NAMES[i])){
      		return i;
      	}
      }
	}
	return -1;
}