#ifndef NEURALNET_UTIL_H
#define NEURALNET_UTIL_H

#include "../Util/pgm_matriz.h"
#include "neuralnet.h"
#include "functions.h"

#define nn_norm(min,max,x) (((x)-(min))/((max)-(min)))

#define nn_denorm(min,max,x) ((x)*((max)-(min)) + (min))

/*
Função que converte uma string em Rede Neural
tal string deve ser formatada da seguinte maneira:

#NumeroDeCamadas#NeuroniosPorCamada#FunçãoDeAtivação#InputMin InputMax OutputMin OutputMax#Stepness#Bias#Pesos
Se nao preencher o campo função de ativação, entao sera atribuida a default : NN_SIGMOID_SYMETRIC
Se nao preencher os campos de input output scale sera atribuido os valores default: 0 1 0 1 repectivamente
*/
NeuralNet *nn_Cstring2NeuralNet(char* text);

/*
Função que converte uma NeuralNet em Cstring
*/
char *nn_NeuralNet2Cstring(NeuralNet *nn);

/*
Imprimir a neuralnet no console
*/
void nn_NeuralNetInfo(NeuralNet *nn);

/*
Função que valida a rede para uma determinada entrada.
Os parametros in e work devem ser vetores com tamanho do numero de neuronios da maior camada;
O PGM_Vetor_Double é o ponteiro que será apontado para a saida, Aloque somente a estrutura, nao aloque o vetor
*/
void nn_NeuralNetRun( NeuralNet *NN,  PGM_Vetor_Double *Out, double *In, double *Work);

/*
Função que quebra a rede em 2 partes
*/
NeuralNet **nn_NeuralnetSplit(NeuralNet *NN, unsigned int SplitLayer);

/*
Função que mescla duas Redes.
	Onde:
		NeuralA->OutputMin == 0.0
		NeuralA->OutputMax == 1.0
		NeuralB->InputMin == 0.0
		NeuralB->InputMax == 1.0

		NeuralA->NLayers + NeuralB->NLayers - 1 >= 5

		NeuralA->NNeurons[NeuralA->NLayers-1] == NeuralB->NNeurons[0]

		NeuralA->FunctionActivation == NeuralB->FunctionActivation

		NeuralA->Bias == NeuralB->Bias

		NeuralA->Steepness == NeuralB->Steepness

*/
NeuralNet *nn_NeuralNetMerge(NeuralNet *NeuralA, NeuralNet *NeuralB);

/*
Função que compara duas redes
*/
char nn_NeuralNetEqual(NeuralNet *NeuralA,NeuralNet *NeuralB);


/*
Função que pega a distancia entre duas redes
*/
double nn_GetDistance(NeuralNet *nnA, NeuralNet *nnB);
#endif
