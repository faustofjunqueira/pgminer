#ifndef NEURALNET_PARSE_H
#define NEURALNET_PARSE_H

/*
Função que inicia o modulo de erro da NeuralNet
ela deve ser invocada antes da NeuralNetError
*/
void nn_parse_NeuralNetInitParseError(char *st);
/*
Função que mostra o erro do parse, mostra aonde foi o erro e aborta o programa
Ver NeuralNetInitParseError
*/
void nn_parse_NeuralNetParseError(char *msg, char *pos);

/*
Função que retorna o numero de camadas
que deve ser 2,3,4 ou 5
caso contrario retorna o 0xFFFF

Ver Cstring2Neuralnet
*/
unsigned short int nn_parse_ReadNumberLayers(char **st);
/*
Função que retorna as o numero de neurorios por camada
Ver Cstring2Neuralnet
*/
unsigned int *nn_parse_ReadNeurons(char **st, unsigned short n_layers);

/*
Função que retorna qual sera a função de ativação
*/
unsigned short nn_parse_ReadFunctionActivation(char **st, double *bhLambida, double *bhT1, double *bhT2);

/*
Função para ler as escalas da rede
se for encontrado na string
...# # ....
não sera alterado as escalas
*/
void nn_parse_ReadScaling(char **st, double *input_min, double *input_max, double *output_min, double *output_max);

/*
Função para ler o steepness
Se nao passar nenhum valor ele atribui o valor default 2.0
*/
double nn_parse_ReadSteepness(char **st,unsigned short function_activation);

/*
Função para ler os pesos
*/
double* nn_parse_ReadWeight(char **st, unsigned int NWeights);

/*
	Função que le o valor do Bias
	Se nao passar nenhum valor ele atribui o valor default 1.0
*/
unsigned int nn_parse_ReadBias(char **st);

#endif
