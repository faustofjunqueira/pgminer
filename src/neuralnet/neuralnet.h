#ifndef NEURALNET_H
#define NEURALNET_H

#include "../Util/pgm_matriz.h"

/* 	Struct: NeuralNet
	Principal estrutura da rede neural.

	Use a função create_neuralnet para cria-la.

 */
#define MAX_LAYERS  5
//Escalas
#define INPUT_MIN_DEFAULT_VALUE 0
#define INPUT_MAX_DEFAULT_VALUE 1.0
#define OUTPUT_MIN_DEFAULT_VALUE 0
#define OUTPUT_MAX_DEFAULT_VALUE 1.0
//Bias
#define BIAS_DEFAULT_VALUE 1.0
//Steepness
#define STEEPNESS_DEFAULT_VALUE 2.0
//Bit Fail
#define BIT_FAIL_DEFAULT_VALUE 0
//Function Activation
#define FUNCTION_ACTIVATION_DEFAULT NN_SIGMOID_SYMMETRIC;
#define STEEPNESS_SIGMOID_DEFAULT 2
#define STEEPNESS_BIHIPERBOLIC_DEFAULT 0.5

typedef struct {
  // tamanho de byte da struct
  int size; 

  // numero de camadas
  unsigned short NLayers;
  
  // codigo da função de ativação
  unsigned short FunctionActivation; 

  // numero de neuronios por camada
  unsigned int NNeurons[MAX_LAYERS];
  
  // numero total de neurônios presentes na ANN
  unsigned int TotalNeurons;

  // Número total de pesos da rede neural
  unsigned int TotalWeights;

  //Utilizado na conversão para FANN
  unsigned int Bias;
  double Steepness;
  double MSE;

  // Quando escolhido a função bihperbólica como função de ativação, esses parametros precisam estar preenchido
  double BihiperbolicLambda, BihiperbolicT1,BihiperbolicT2;

  // Input (Max e Min), valores maximos e mínimos da entrada para efeitos de normalização
  double InputMin, InputMax;
  // Output (Max e Min), valores maximos e mínimos da saída para efeitos de normalização
  double OutputMin, OutputMax;

  //Vetor com o indice que inicia a sequencia de pesos da camada no vetor Weights
  unsigned int WeightsPos[MAX_LAYERS - 1];

  // Vetor que guarda os valores dos neuronios e os pesos. Para acessar os pesos basta da um GetWeght(layer)
  double Weights[0];
  /*
  Pesos:
    primeira camada
      - Bias[layer][i] - Neuron[layer+1][i]
      - Neuron[layer][j] - Neuron[layer+1][i]
      - Neuron[layer][j+1] - Neuron[layer+1][i]
      assim sucessivamente
      Lembrando que a ultima camada nao tem Bias
  */
}NeuralNet;

/*
Função que cria uma rede Neural
*/
NeuralNet* nn_NeuralNetCreate(unsigned short n_layers,unsigned short function_activation, double bhLambda,double bhT1, double bhT2,
                           unsigned int neurons[],double input_min,double input_max, double output_min, double output_max,
                           double steepness, double bias, double weigths[]);

/*
Macro que retorna o peso de uma conexão de neuronios.
Lembrando que o Bias eh o NeuronIn = 0, porem ele será mascarado. Logo se quizer pegar Bias usa o nn_GetBiasWeight
Na expressão a parte que diz : ... (NeuronPrev+1) ... o +1 se da à mascara referida. Para que a quando passe 0 - 0 nao seja o Bias e sim o primeiro neuronPrev com o primeiro neuronNext
*/
#define nn_GetWeight(NN,Layer,NeuronPrev,NeuronNext) \
	NN->Weights[(NN)->WeightsPos[Layer] + (NN->NNeurons[Layer]+1)*(NeuronNext) + (NeuronPrev)]

/*
Macro que retorna o peso do Bias, vide nn_GetWeight
*/
#define nn_GetBiasWeight(NN, Layer,NeuronNext) \
	NN->Weights[(NN)->WeightsPos[Layer] + (NN->NNeurons[Layer]+1)*(NeuronNext)+(NN->NNeurons[Layer])]

/*
Função que retorna o numero de neuronios da maior camada
*/
unsigned int nn_MaxLayer(NeuralNet *NN);

/*
Calcula o Total de Neuronios
*/
unsigned int nn_CalculateTotalNeurons(unsigned short Layers, unsigned int Neurons[]);

/*
Função que desaloca a neuralnet
*/
void nn_NeuralNetDestroy(NeuralNet *NN);

/*
Função que retorna a maior camada da rede
*/
unsigned int nn_MaxNeuronLayer(NeuralNet *NN);

/*
Função que conta a quantidade de pesos.
*/
unsigned int nn_GetNumberWeight(unsigned short n_layers, unsigned int neurons[]);

/*
Função que mostra o erro e aborta o programa
Ver NeuralNetInitError
*/
void nn_NeuralNetError(char *msg);

/*
Função que mostra um alerta no console
*/
void nn_NeuralNetWarning(char *msg);

#endif
