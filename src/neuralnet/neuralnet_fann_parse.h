#ifndef NEURALNET_FANN_PARSE_H
#define NEURALNET_FANN_PARSE_H

#include "neuralnet.h"
#include "fann/include/doublefann.h"
/*
Pega o vertor de conexões da NeuralNet
*/
void nn_fann_parse_get_connection( int n_layers, int *n_neurons, struct fann_connection *connections_nn);

/*
Ajusta os pesos do layout da FANN para o layout da NeuralNet
*/
void nn_fann_parse_adjust_get_weightByFANN(int totalConnection,struct fann_connection *connections_ann, struct fann_connection *connections_nn, fann_type *weight);

/*
Função que retorna os pesos da FANN para NeuralNet
*/
double *nn_fann_parse_get_weights(struct fann *ANN, unsigned int NLayers, unsigned int *NNeurons);

/*
Ajusta os pesos do layout da NeuralNet para o layout da FANN
*/
void nn_fann_parse_adjust_get_weightByNeuralNet(NeuralNet *nn, struct fann *ann, struct fann_connection *connections_ann, struct fann_connection *connections_nn);

/*
Set as escalas da fann para default
*/
void nn_fann_parse_fann_set_scaling_default(struct fann* ann);

/*
pega as conexões da NN
*/
void nn_fann_parse_nn_get_connection2( int n_layers, int *n_neurons, struct fann_connection *connections_nn);

/*
Função que seta as escalas da FANN
*/
void nn_fann_parse_fann_set_scaling (struct fann* ann, double input_min, double input_max, double output_min, double output_max);

/*
Função que geta os pesos para  as conexões
*/
double nn_fann_parse_get_weight_by_connection(NeuralNet *NN, struct fann_connection *connection);

/*
Função que seta os parametro da função bihiperbolica
*/
void nn_fann_parse_setBihiperbolicParam(struct fann *ann, double bhLambda,double bhT1, double bhT2);
#endif
