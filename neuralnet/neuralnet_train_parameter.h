#ifndef NEURALNET_TRAIN_PARAMETER_H
#define NEURALNET_TRAIN_PARAMETER_H

#include "postgres.h"       //standard
#include "fmgr.h"           //standard
#include "utils/memutils.h"//standard
#include "utils/geo_decls.h"//standard
#include "utils/array.h"    //arrays
#include "utils/errcodes.h" //warning
#include "catalog/pg_type.h"// OID
#include "utils/lsyscache.h"// construi array type
#include "funcapi.h"        // trabalhar com row
#include "access/heapam.h"  //Construção de Tupla

#include "../Util/pgm_malloc.h"
#include "functions.h"
#include "fann/include/fann.h"

#define BUFFERPARAMLEN 32

typedef struct{
    int    TrainingAlgorithm;
    float  LearningRate;
    float  LearningMomentum;
    double BitFailLimit;
    int MaxEpochs;
    int EpochsBetweenReports;
    double DesiredError;

}NN_Fann_Train_Parameter;

typedef struct{
    int ActivationFunction;
    double Steepness;
    double Bihiperbolic_T1;
    double Bihiperbolic_T2;
}NN_Fann_Create_Parameter;

int readTrainParameterKey  (char *st);
char *readTrainParameterValue(char *st, char *buffer,NN_Fann_Train_Parameter *FannParam);
void setDefaultNNFannTrainParameter(NN_Fann_Train_Parameter *param);
NN_Fann_Train_Parameter *ArrayType2NN_Fann_Train_Parameter(ArrayType *array);
int readAlgorithm(char *st);
NN_Fann_Create_Parameter *ArrayType2NN_Fann_Create_Parameter(ArrayType *array);
void setDefaultNNFannCreateParameter(NN_Fann_Create_Parameter *param);
int readCreateParameterKey(char *st);
char *readCreateParameterValue(char *st, char *buffer,NN_Fann_Create_Parameter*FannParam);
int readActivationFunction(char *st);
void runCreateParameter(struct fann *ANN,NN_Fann_Create_Parameter *create_param);

#endif
