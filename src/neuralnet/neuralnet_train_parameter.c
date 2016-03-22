
#include "neuralnet_train_parameter.h"

int readTrainParameterKey(char *st){

    if(strcmp(st, "algorithm") == 0)
        return 0;
    else if(strcmp(st, "rate") == 0)
        return 1;
    else if(strcmp(st, "momentum") == 0)
        return 2;
    else if(strcmp(st, "bitfaillimit") == 0)
        return 3;
    else if(strcmp(st, "epochs") == 0)
        return 4;
    else if(strcmp(st, "report") == 0)
        return 5;
    else if(strcmp(st, "desired_error") == 0)
        return 6;
    else
        return -1;
}

char *readTrainParameterValue(char *st, char *buffer,NN_Fann_Train_Parameter *FannParam){
    int paramCode = readTrainParameterKey(st);

    while(*st) st++;
    st++;

    memset(buffer, 0, sizeof(char)*BUFFERPARAMLEN);
    st += sprintf(buffer,"%s",st);
    st++;

    switch(paramCode){
        case 0: FannParam->TrainingAlgorithm = readAlgorithm(buffer);break;
        case 1: if( !(FannParam->LearningRate = atof(buffer)) ) elog(ERROR,"Valor invalido para rate") ;break;
        case 2: if( !(FannParam->LearningMomentum = atof(buffer)) ) elog(ERROR,"Valor invalido para momentum") ;break;
        case 3: if( !(FannParam->BitFailLimit = atof(buffer)) ) elog(ERROR,"Valor invalido para bit fail limite") ;break;
        case 4: if( !(FannParam->MaxEpochs = atoi(buffer)) ) elog(ERROR,"Valor invalido para epochs") ;break;
        case 5: if( !(FannParam->EpochsBetweenReports= atoi(buffer)) ) elog(ERROR,"Valor invalido para report") ;break;
        case 6: if( !(FannParam->DesiredError= atof(buffer)) ) elog(ERROR,"Valor invalido para desired_error") ;break;
        default: elog(INFO,"Use: algorithm,rate,momentum,bifaillimit,epochs,report,desired_error.");elog(ERROR,"Key parameter invalid");
    }
    return st;
}

NN_Fann_Train_Parameter *ArrayType2NN_Fann_Train_Parameter(ArrayType *array){
    NN_Fann_Train_Parameter *param = NULL;
    if(ARR_NDIM(array) != 2)
        elog(ERROR,"O array deve ser bidimencional");
    else{
        char *text =(char*) ARR_DATA_PTR(array);
        char buffer[BUFFERPARAMLEN]={0};
        int i;
        param = (NN_Fann_Train_Parameter*) pgm_malloc(sizeof(NN_Fann_Train_Parameter));
        setDefaultNNFannTrainParameter(param);

        for(i = 0; i < ARR_DIMS(array)[0]; i++){
            text = readTrainParameterValue(text,buffer,param);
        }
    }
    return param;
}

void setDefaultNNFannTrainParameter(NN_Fann_Train_Parameter *param){
    //Valores usados como default pela FANN
    param->TrainingAlgorithm = FANN_TRAIN_RPROP;
    param->LearningRate = 0.7;
    param->LearningMomentum = 0;
    param->BitFailLimit = 0.35;
    param->MaxEpochs = 1000;
    param->DesiredError = 0.01;
    param->EpochsBetweenReports = 100;
}

int readAlgorithm(char *st){
    int i;
    for( i = 0; i < FANN_NALGORITHMTYPE; i++){
        if(!strcmp(st,FANN_TRAIN_NAMES[i]))
            return i;
    }
    elog(INFO,"Use:");
    for( i = 0; i < FANN_NALGORITHMTYPE; i++){
        elog(INFO,"%s",FANN_TRAIN_NAMES[i]);
    }
    elog(ERROR,"Nome do algoritmo invalido");
    return -1;
}
//==================================================================================================


void setDefaultNNFannCreateParameter(NN_Fann_Create_Parameter *param){
    param->ActivationFunction = NN_SIGMOID;
    param->Steepness = 0.5;
    param->Bihiperbolic_T1 = 0.25;
    param->Bihiperbolic_T2 = 0.25;
}

NN_Fann_Create_Parameter *ArrayType2NN_Fann_Create_Parameter(ArrayType *array){
    NN_Fann_Create_Parameter *param = NULL;
    if(ARR_NDIM(array) != 2)
        elog(ERROR,"O array deve ser bidimencional");
    else{
        char *text =(char*) ARR_DATA_PTR(array);
        char buffer[BUFFERPARAMLEN]={0};
        int i;
        param = (NN_Fann_Create_Parameter*) pgm_malloc(sizeof(NN_Fann_Create_Parameter));
        setDefaultNNFannCreateParameter(param);

        for(i = 0; i < ARR_DIMS(array)[0]; i++){
            text = readCreateParameterValue(text,buffer,param);
        }
    }
    return param;
}

int readCreateParameterKey(char *st){

    if(strcmp(st, "function") == 0)
        return 0;
    else if(strcmp(st, "steepness") == 0)
        return 1;
    else if(strcmp(st, "t1") == 0)
        return 2;
    else if(strcmp(st, "t2") == 0)
        return 3;
    else
        return -1;
}

char *readCreateParameterValue(char *st, char *buffer,NN_Fann_Create_Parameter*FannParam){
    int paramCode = readCreateParameterKey(st);

    while(*st) st++;
    st++;

    memset(buffer, 0, sizeof(char)*BUFFERPARAMLEN);
    st += sprintf(buffer,"%s",st);
    st++;

    switch(paramCode){
        case 0: FannParam->ActivationFunction = readActivationFunction(buffer);break;
        case 1: if( !(FannParam->Steepness = atof(buffer)) ) elog(ERROR,"Valor invalido para steepness") ;break;
        case 2: if( !(FannParam->Bihiperbolic_T1 = atof(buffer)) ) elog(ERROR,"Valor invalido para t1") ;break;
        case 3: if( !(FannParam->Bihiperbolic_T2 = atof(buffer)) ) elog(ERROR,"Valor invalido para t2") ;break;
        default: elog(INFO,"Use: function,steepness,t1,t2.");elog(ERROR,"Key parameter invalid");
    }
    return st;
}

int readActivationFunction(char *st){
    int i;
    for( i = 0; i < NN_FUNCTION_ACTIVATION; i++){
        if(!strcmp(st,NN_ACTIVATIONFUNC_NAMES[i]))
            return i;
    }
    elog(INFO,"Use:");
    for( i = 0; i < NN_FUNCTION_ACTIVATION; i++){
        elog(INFO,"%s",NN_ACTIVATIONFUNC_NAMES[i]);
    }
    elog(ERROR,"Nome do algoritmo invalido");
    return -1;
}

void runCreateParameter(struct fann *ANN,NN_Fann_Create_Parameter *create_param){
    fann_set_activation_function_hidden(ANN,create_param->ActivationFunction);
    fann_set_activation_function_output(ANN,create_param->ActivationFunction);

    fann_set_activation_steepness_hidden(ANN,create_param->Steepness);
    fann_set_activation_steepness_output(ANN,create_param->Steepness);
    nn_setBiHiperbolicParameter(create_param->Steepness, create_param->Bihiperbolic_T1, create_param->Bihiperbolic_T2);
}

void runTrainParameter(struct fann *ANN, NN_Fann_Train_Parameter *train_param){
    fann_set_training_algorithm( ANN, train_param->TrainingAlgorithm );
    fann_set_learning_momentum(ANN,train_param->LearningMomentum);
    fann_set_learning_rate(ANN,train_param->LearningRate);
}
