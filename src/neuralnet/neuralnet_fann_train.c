
#include "neuralnet_fann_train.h"


void nn_fann_train_destroi_fann_train_data( struct fann_train_data* data ) {
  pgm_free( data->input[0] ); /* Pequeno leak de memoria... */
  pgm_free( data->input );
  pgm_free( data->output[0] );
  pgm_free( data->output );
  pgm_free( data );
}

PGM_Vetor_Int *nn_fann_train_build_neuron_array(PGM_Vetor_Int *hidden, int in, int out){
  PGM_Vetor_Int* result;
  if ( hidden == NULL){
    result = pgm_create_PGM_Vetor_Int(2);
    result->valor[0] = in;
    result->valor[1] = out;
    return result;
  }else{
    int i = 0;
    result = pgm_create_PGM_Vetor_Int(hidden->n_elems+2);
    result->valor[0] = in;
    for(; i < hidden->n_elems; i++)
      result->valor[i+1] = hidden->valor[i];
    result->valor[i+1] = out;

    return result;
  }

}

struct fann *nn_fann_train_create_standard_array(PGM_Vetor_Int *layers, double steepness, int functionActivation){
  struct fann *ANN = fann_create_standard_array(layers->n_elems, (unsigned int*)layers->valor);

  nn_fann_parse_fann_set_scaling_default(ANN);

  fann_set_training_algorithm( ANN, FANN_TRAIN_RPROP );

  fann_set_activation_function_hidden(ANN,functionActivation);
  fann_set_activation_function_output(ANN,functionActivation);

  fann_set_activation_steepness_hidden(ANN,steepness);
  fann_set_activation_steepness_output(ANN,steepness);

  return ANN;
}
//=================================== FIM FUNÇÕES INTERNAS ==============================================

//void nn_fann_train_start_parameter(ANN, <LISTA DE PARAMETROS!>)

struct fann_train_data* nn_fann_train_create_fann_train_data( PGM_Matriz_Double *input, PGM_Matriz_Double *output ) {
  fann_type *data_input = (fann_type*) input->valor,
         *data_output = (fann_type*)output->valor;

  struct fann_train_data *data = (struct fann_train_data*) pgm_malloc (sizeof(struct fann_train_data));
  int i;

  if( input->n_linhas != output->n_linhas ){
    elog(ERROR, "Erro no numero de linha da entrada e saida ( %d - %d)",  input->n_linhas, output->n_linhas);
  }

  data->num_data = input->n_linhas;
  data->num_input = input->n_colunas;
  data->num_output = output->n_colunas;

  data->input = (fann_type **) pgm_calloc( data->num_data, sizeof(fann_type *));
  data->output = (fann_type **) pgm_calloc( data->num_data, sizeof(fann_type *));

  for(i = 0; i != data->num_data; i++)
  {
    data->input[i] = data_input;
    data_input += data->num_input;

    data->output[i] = data_output;
    data_output += data->num_output;
  }
  return data;
}

struct fann *nn_fann_train1(struct fann_train_data *data, PGM_Vetor_Int *hidden, int functionActivation,
                          double steepness,int max_epochs,int epochs_between_reports,double desired_error){

  struct fann *ANN;
  MemoryContext contextoAnterior = MemoryContextSwitchTo( CurTransactionContext );

  if (hidden->n_elems > 3){
    elog (ERROR, "O numero maximo de camadas escondidas é 3 (%d)", hidden->n_elems);
  }

  hidden = nn_fann_train_build_neuron_array(hidden, data->num_input, data->num_output);

  ANN = nn_fann_train_create_standard_array(hidden, steepness,functionActivation);

  fann_init_weights( ANN, data );
  fann_train_on_data(ANN, data, max_epochs, epochs_between_reports, desired_error);
  nn_fann_parse_fann_set_scaling_default(ANN);

  MemoryContextSwitchTo( contextoAnterior );

  return ANN;
}

struct fann *nn_fann_train_bihiperbolic(struct fann_train_data *data, PGM_Vetor_Int *hidden,
                          double lam, double t1, double t2,
                          double steepness,int max_epochs,int epochs_between_reports,double desired_error){

  MemoryContext contextoAnterior = MemoryContextSwitchTo( CurTransactionContext );
  struct fann *ANN;

  if (hidden->n_elems > 3){

    elog (ERROR, "O numero maximo de camadas escondidas é 3 (%d)", hidden->n_elems);
  }

  hidden = nn_fann_train_build_neuron_array(hidden, data->num_input, data->num_output);

  ANN = nn_fann_train_create_standard_array(hidden, steepness,NN_BIHIPERBOLIC);

  nn_setBiHiperbolicParameter(lam,t1,t2);

  fann_init_weights( ANN, data );
  fann_train_on_data(ANN, data, max_epochs, epochs_between_reports, desired_error);
  nn_fann_parse_fann_set_scaling_default(ANN);

  MemoryContextSwitchTo( contextoAnterior );

  return ANN;
}

struct fann *nn_fann_train2(struct fann_train_data *data, double steepness, int functionActivation,
                            int max_epochs, int epochs_between_reports,double desired_error){

  struct fann *ann;


  MemoryContext contextoAnterior = MemoryContextSwitchTo( CurTransactionContext );


  PGM_Vetor_Int *hidden = nn_fann_train_build_neuron_array(NULL, data->num_input, data->num_output);

  ann = nn_fann_train_create_standard_array(hidden, steepness,functionActivation);
  fann_init_weights( ann, data );

  fann_train_on_data(ann, data, max_epochs, epochs_between_reports, desired_error);
  nn_fann_parse_fann_set_scaling_default(ann);

  MemoryContextSwitchTo( contextoAnterior );


  return ann;
}

double nn_fann_train3(struct fann_train_data *data,struct fann *ann,
                         int max_epochs,int epochs_between_reports,double desired_error){


  MemoryContext contextoAnterior = MemoryContextSwitchTo( CurTransactionContext );


  fann_train_on_data(ann, data, max_epochs, epochs_between_reports, desired_error);


  MemoryContextSwitchTo( contextoAnterior );


  return fann_get_MSE(ann);
}

struct fann *nn_fann_train4(struct fann_train_data *data, PGM_Vetor_Int *hidden, NN_Fann_Create_Parameter *create_param, NN_Fann_Train_Parameter *train_param){

  MemoryContext contextoAnterior = MemoryContextSwitchTo( CurTransactionContext );
  struct fann *ANN;

  if (hidden->n_elems > 3){

    elog (ERROR, "O numero maximo de camadas escondidas é 3 (%d)", hidden->n_elems);
  }

  hidden = nn_fann_train_build_neuron_array(hidden, data->num_input, data->num_output);

  ANN = fann_create_standard_array(hidden->n_elems, (unsigned int*)hidden->valor);

  runCreateParameter(ANN,create_param);

  nn_fann_parse_fann_set_scaling_default(ANN);

  fann_init_weights( ANN, data );

  fann_train_on_data(ANN, data, train_param->MaxEpochs, train_param->EpochsBetweenReports, train_param->DesiredError);
  nn_fann_parse_fann_set_scaling_default(ANN);

  MemoryContextSwitchTo( contextoAnterior );

  return ANN;
}
