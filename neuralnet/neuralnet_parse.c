#include "postgres.h"       //standard
#include "fmgr.h"           //standard
#include "utils/memutils.h"//standard
#include "utils/geo_decls.h"//standard
#include "utils/array.h"    //arrays
#include "utils/errcodes.h" //warning

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "../Util/pgm_malloc.h"
#include "neuralnet.h"
#include "functions.h"
#include "neuralnet_parse.h"

char *StringError;

void nn_parse_NeuralNetInitParseError(char *st){
	StringError = st;
}

void nn_parse_NeuralNetParseError(char *msg, char *pos){

	unsigned int len_0_at_pos = pos - StringError, i;
	char *str = (char*) pgm_malloc (sizeof(char) * (strlen(msg)+strlen(StringError)+250+2*len_0_at_pos));
	sprintf(str,"%s\n",msg);
	sprintf(str,"%s%s\n",str, StringError);
	for(i = 0; i < len_0_at_pos; i++)
		sprintf(str,"%s ",str);
	sprintf(str,"%s^\n",str);
	sprintf(str,"%sFormato da String\n",str);
	sprintf(str,"%s\t#NumeroDeCamadas#NeuroniosPorCamada#FunçãoDeAtivação#InputMin InputMax OutputMin OutputMax#Stepness#Bias#Pesos\n",str);
	elog(ERROR,"\n%s",str);
}

unsigned short nn_parse_ReadNumberLayers(char **st){
	int n_layers = 0;
	while( **st == ' ' || **st =='\t' || **st =='\r' || **st =='\n') (*st)++;
	if(**st == '#') (*st)++;
	while( **st == ' ' || **st =='\t' || **st =='\r' || **st =='\n') (*st)++;

	if(sscanf(*st,"%d",&n_layers) == 1)
		if( n_layers <= MAX_LAYERS)
			return (unsigned short) n_layers;

	nn_parse_NeuralNetParseError("Numero de camadas invalida maximo de 5",*st);
	return 0xFFFF;
}

unsigned int *nn_parse_ReadNeurons(char **st, unsigned short n_layers){
	int i;
	unsigned int*neurons;

	neurons = (unsigned int*) pgm_malloc(sizeof(unsigned int)*MAX_LAYERS);

	while( **st != '#') (*st)++;
	(*st)++;

	for(i = 0; **st != '#' ; i++){
		while( **st == ' ' || **st =='\t' || **st =='\r' || **st =='\n') (*st)++;

		if((**st >= '0' && **st <= '9') || **st == '+')
			sscanf(*st,"%d",neurons+i);
		else if(**st == '-')
			nn_parse_NeuralNetParseError("Numero negativo na camada",*st);
		else
			nn_parse_NeuralNetParseError("String mal formatada",*st);

		while(**st >= '0' && **st <= '9') (*st)++;
		while( **st == ' ' || **st =='\t' || **st =='\r' || **st =='\n') (*st)++;

	}
	if(i != n_layers)
		nn_parse_NeuralNetParseError("Numero de neuronios incorretos",*st);

	return neurons;
}

unsigned short nn_parse_ReadFunctionActivation(char **st, double *bhLambida, double *bhT1, double *bhT2){
	unsigned short function_activation = FUNCTION_ACTIVATION_DEFAULT;
	while( **st != '#') (*st)++;
	(*st)++;
	while( **st == ' ' || **st =='\t' || **st =='\r' || **st =='\n') (*st)++;
	if(**st == '#')
		return function_activation;
	else{
		int i = -1;
		char *pt, *word;
		unsigned int len_word;
		pt = *st;

		*st += 2;// adiante o # e espaço

		while( **st != ' ' && **st !='\t' && **st !='\r' && **st !='\n' && **st !='#') (*st)++; // passa a palavra toda

		len_word = *st - pt;
		word = (char*) pgm_malloc (sizeof(char)*(len_word+1));
		word[len_word] = '\0';
		strncpy(word,pt,len_word);

		while(word[++i]) word[i] = toupper(word[i]);

		if((function_activation = nn_FindFunctionActivation(word)) == 0xFFFF)
			nn_parse_NeuralNetParseError("Não foi encontrada essa função de ativação",pt);

        if(bhLambida != NULL && function_activation == NN_BIHIPERBOLIC){
            while( **st == ' ' || **st =='\t' || **st =='\r' || **st =='\n') (*st)++;
            if( **st == '#'){
                *bhLambida = NN_BI_HIPERBOLIC_LAMBDA_DEFAULT;
                *bhT1 = NN_BI_HIPERBOLIC_T1_DEFAULT;
                *bhT2 = NN_BI_HIPERBOLIC_T2_DEFAULT;
            }else if(sscanf(*st,"%lf",bhLambida) != 1)
                nn_parse_NeuralNetParseError("Não foi possivel ler o valor de lambda",pt);

            while( **st != ' ' && **st !='\t' && **st !='\r' && **st !='\n' && **st !='#') (*st)++;
            while( **st == ' ' || **st =='\t' || **st =='\r' || **st =='\n') (*st)++;
            if( **st == '#'){
                *bhT1 = NN_BI_HIPERBOLIC_T1_DEFAULT;
                *bhT2 = NN_BI_HIPERBOLIC_T2_DEFAULT;
            }else if(sscanf(*st,"%lf",bhT1) != 1)
                nn_parse_NeuralNetParseError("Não foi possivel ler o valor de lambda",pt);

            while( **st != ' ' && **st !='\t' && **st !='\r' && **st !='\n' && **st !='#') (*st)++;
            while( **st == ' ' || **st =='\t' || **st =='\r' || **st =='\n') (*st)++;
            if( **st == '#'){
                *bhT2 = *bhT1;
            }else if(sscanf(*st,"%lf",bhT2) != 1)
                nn_parse_NeuralNetParseError("Não foi possivel ler o valor de lambda",pt);
        }

        elog(INFO,"\n+-----------------------------\n+ %lf %lf %lf\n+-----------------------------\n",*bhLambida,*bhT1,*bhT2);
		return function_activation;
	}
}

void nn_parse_ReadScaling(char **st, double *input_min, double *input_max, double *output_min, double *output_max){
	while( **st != '#') (*st)++;
	(*st)++;
	while( **st == ' ' || **st =='\t' || **st =='\r' || **st =='\n') (*st)++;
	if(**st== '#'){
		return;
	}
	if(sscanf(*st, "%lf", input_min) != 1)
		nn_parse_NeuralNetParseError("String Mal formatada",*st);
	while( **st != ' ' && **st !='\t' && **st !='\r' && **st !='\n') (*st)++;
	while( **st == ' ' || **st =='\t' || **st =='\r' || **st =='\n') (*st)++;
	if(sscanf(*st, "%lf", input_max) != 1)
		nn_parse_NeuralNetParseError("String Mal formatada",*st);
	while( **st != ' ' && **st !='\t' && **st !='\r' && **st !='\n') (*st)++;
	while( **st == ' ' || **st =='\t' || **st =='\r' || **st =='\n') (*st)++;
	if(sscanf(*st, "%lf", output_min) != 1)
		nn_parse_NeuralNetParseError("String Mal formatada",*st);
	while( **st != ' ' && **st !='\t' && **st !='\r' && **st !='\n') (*st)++;
	while( **st == ' ' || **st =='\t' || **st =='\r' || **st =='\n') (*st)++;
	if(sscanf(*st, "%lf", output_max) != 1)
		nn_parse_NeuralNetParseError("String Mal formatada",*st);
}

double nn_parse_ReadSteepness(char **st,unsigned short function_activation){
	double steepness;

	while( **st != '#') (*st)++;
	(*st)++;
	while( **st == ' ' || **st =='\t' || **st =='\r' || **st =='\n') (*st)++;

	if(**st== '#'){
		if(function_activation == nn_FindFunctionActivation("NN_BIHIPERBOLIC"))
			return STEEPNESS_BIHIPERBOLIC_DEFAULT;
		else
			return STEEPNESS_SIGMOID_DEFAULT;
	}
	if(sscanf(*st, "%lf", &steepness) != 1)
		nn_parse_NeuralNetParseError("String Mal formatada",*st);

	return steepness;
}

unsigned int nn_parse_ReadBias(char **st){
	while( **st != '#') (*st)++;
	(*st)++;
	while( **st == ' ' || **st =='\t' || **st =='\r' || **st =='\n') (*st)++;
	if(**st== '#')
		return BIAS_DEFAULT_VALUE;
	else{
		unsigned int bias;
		if(sscanf(*st,"%u",&bias) != 1)
			nn_parse_NeuralNetParseError("Não foi possivel ler o valor do Bias",*st);
		while ((**st >= '0' && **st <='9') || **st =='.' || **st =='e' || **st =='E' || **st =='-' || **st =='+' ) (*st)++;
		return bias;
	}
}

double* nn_parse_ReadWeight(char **st, unsigned int NWeights){
	unsigned int i;
	double *weight = (double*) pgm_malloc (sizeof(double)*(NWeights));

	while( **st != '#')(*st)++;
	(*st)++;

	for(i = 0; i < NWeights; i++){
		while( **st == ' ' || **st =='\t' || **st =='\r' || **st =='\n') (*st)++;
		if(!(**st))
			nn_parse_NeuralNetParseError("Numero de pesos insuficiente",*st);
		if(sscanf(*st,"%lf",weight+i) != 1)
			nn_parse_NeuralNetParseError("Não foi possivel ler o valor do peso",*st);
		while ((**st >= '0' && **st <='9') || **st =='.' || **st =='e' || **st =='E' || **st =='-' || **st =='+' ) (*st)++;
	}
	while( **st == ' ' || **st =='\t' || **st =='\r' || **st =='\n') (*st)++;

	if(**st) nn_parse_NeuralNetParseError("String mal formatada",*st);

	return weight;
}
