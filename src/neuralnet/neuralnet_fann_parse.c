#include "postgres.h"       //standard
#include "fmgr.h"           //standard
#include "utils/memutils.h"//standard
#include "utils/geo_decls.h"//standard
#include "utils/errcodes.h" //warning
#include "utils/array.h"    //arrays

#include "stdio.h"

#include "../Util/pgm_malloc.h"
#include "neuralnet_fann_parse.h"

void nn_fann_parse_get_connection( int n_layers, int *n_neurons, struct fann_connection *connections_nn){
	int i,j,k,l=0;
//Matrix de posicionamente das conexoes
	int **neurons_layers = (int**) pgm_malloc (sizeof(int*) * n_layers);
	int count = 0;
	for(i=0; i < n_layers; i++){
		neurons_layers[i] = (int*) pgm_malloc(sizeof(int)*(n_neurons[i]+1));
		for(j = 0; j < n_neurons[i] + 1; j++){
			neurons_layers[i][j] = count;
			count++;
		}
	}
//Identificando as conexoes
	for(i=0; i < n_layers - 1; i++){
		for(k=0; k < n_neurons[i] + 1; k++){
			for(j = 0; j < n_neurons[i+1]; j++){
				connections_nn[l].to_neuron = neurons_layers[i+1][j];
				connections_nn[l].from_neuron = neurons_layers[i][k];
				l++;
			}
		}
	}
}

void nn_fann_parse_adjust_get_weightByFANN(int totalConnection,struct fann_connection *connections_ann, struct fann_connection *connections_nn, fann_type *weight){
	int i,k;
	//Compara as conexoes por forcao bruta (NAIBE)
	for ( i = 0; i < totalConnection; i++){
		for ( k = 0; k < totalConnection; k++){
			if(connections_ann[k].from_neuron == connections_nn[i].from_neuron && connections_ann[k].to_neuron == connections_nn[i].to_neuron){
				weight[i] = connections_ann[k].weight;
				break;
			}
		}
	}
}

double *nn_fann_parse_get_weights(struct fann *ANN, unsigned int NLayers, unsigned int *NNeurons){
	double *Weights= (double*) pgm_malloc(sizeof(double)*fann_get_total_connections(ANN));
	struct 	fann_connection 	*ConnectionsANN = (struct fann_connection*) pgm_malloc(sizeof(struct fann_connection) * ANN->total_connections);
	struct 	fann_connection 	*ConnectionsNN =  (struct fann_connection*) pgm_malloc(sizeof(struct fann_connection) * ANN->total_connections);

	fann_get_connection_array(ANN, ConnectionsANN);
	nn_fann_parse_get_connection(NLayers,(int*)NNeurons, ConnectionsNN);
	nn_fann_parse_adjust_get_weightByFANN(fann_get_total_connections(ANN), ConnectionsANN, ConnectionsNN, (fann_type*)Weights);

	return Weights;
}

void nn_fann_parse_fann_set_scaling_default(struct fann* ann){
	ann->input_min = INPUT_MIN_DEFAULT_VALUE;
	ann->input_max = INPUT_MAX_DEFAULT_VALUE;
	ann->output_min = OUTPUT_MIN_DEFAULT_VALUE;
	ann->output_max = OUTPUT_MAX_DEFAULT_VALUE;
}

void nn_fann_parse_nn_get_connection2( int n_layers, int *n_neurons, struct fann_connection *connections_nn){
	int i,j,k,l=0;
//Matrix de posicionamente das conexoes
	int **neurons_layers = (int**) pgm_malloc (sizeof(int*) * n_layers);
	int count = 0;
	for(i=0; i < n_layers; i++){
		neurons_layers[i] = (int*) pgm_malloc(sizeof(int)*(n_neurons[i]+1));
		for(j = 0; j < n_neurons[i] + 1; j++){
			neurons_layers[i][j] = count;
			count++;
		}
	}
//Identificando as conexoes
	for(i=0; i < n_layers - 1; i++){
		for(j = 0; j < n_neurons[i+1]; j++){
			for(k=0; k < n_neurons[i] + 1; k++){
				connections_nn[l].to_neuron = neurons_layers[i+1][j];
				connections_nn[l].from_neuron = neurons_layers[i][k];
				l++;
			}
		}
	}
}

double nn_fann_parse_get_weight_by_connection(NeuralNet *NN, struct fann_connection *connection){
    int in = 0, out = 0, layer = 0, sumNeuron = 0;
    do{
        if(sumNeuron <= connection->from_neuron && sumNeuron+NN->NNeurons[layer+1]+1 >= connection->from_neuron)
            break;
        else{
            sumNeuron += NN->NNeurons[layer]+1;
            layer++;
        }
    }while(layer < NN->NLayers-1);

    in = connection->from_neuron - sumNeuron;
    sumNeuron = NN->NNeurons[layer]+1;

    out = connection->to_neuron - sumNeuron;

	return nn_GetWeight(NN,layer,in,out);
}

void nn_fann_parse_adjust_get_weightByNeuralNet(NeuralNet *nn, struct fann *ann, struct fann_connection *connections_ann, struct fann_connection *connections_nn){
	int i,k;
	//Compara as conexoes por forcao bruta (NAIBE)
	for ( i = 0; i < ann->total_connections; i++){
		for ( k = 0; k < ann->total_connections; k++){
			if(connections_ann[i].from_neuron == connections_nn[k].from_neuron && connections_ann[i].to_neuron == connections_nn[k].to_neuron){
				fann_set_weight(ann,connections_ann[i].from_neuron,connections_ann[i].to_neuron,nn_fann_parse_get_weight_by_connection(nn,&connections_ann[i]));
				break;
			}
		}
	}

}

void nn_fann_parse_fann_set_scaling (struct fann* ann, double input_min, double input_max, double output_min, double output_max){
	ann->input_min = input_min;
	ann->input_max = input_max;
	ann->output_min = output_min;
	ann->output_max = output_max;
}

void nn_fann_parse_setBihiperbolicParam(struct fann *ann, double bhLambda,double bhT1, double bhT2){
    ann->BiperbolicLambda = bhLambda;
    ann->BiperbolicT1 = bhT1;
    ann->BiperbolicT2 = bhT2;
}
