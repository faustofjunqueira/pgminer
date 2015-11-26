#ifndef NEURALNET_FANN_H
#define NEURALNET_FANN_H

#include "fann/include/doublefann.h"

/*
Função que converte uma struct fann para NeuralNet
*/
NeuralNet *nn_fann_Fann2NeuralNet(struct fann *ANN);

/*
Função que converte uma NeuralNet em struct fann
*/
struct fann *nn_fann_NeuralNet2Fann(NeuralNet *NN);

#endif
