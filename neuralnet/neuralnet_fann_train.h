#ifndef NEURALNET_FANN_TRAIN_H
#define NEURALNET_FANN_TRAIN_H

#include "postgres.h"       //standard
#include "fmgr.h"           //standard
#include "utils/memutils.h"//standard
#include "utils/geo_decls.h"//standard
#include "utils/array.h"    //arrays
#include "utils/errcodes.h" //warning


#include "neuralnet.h"
#include "neuralnet_fann_train.h"
#include "neuralnet_fann_parse.h"
#include "neuralnet_train_parameter.h"

#include "../Util/pgm_malloc.h"
#include "../Util/pgm_matriz.h"
#include "../Util/pgm_matriz_util.h"

#include "fann/include/doublefann.h"

struct fann_train_data* nn_fann_train_create_fann_train_data( PGM_Matriz_Double *input, PGM_Matriz_Double *output );

struct fann *nn_fann_train1(struct fann_train_data *data, PGM_Vetor_Int *hidden, int functionActivation,
                          double steepness,int max_epochs,int epochs_between_reports,double desired_error);

struct fann *nn_fann_train2(struct fann_train_data *data, double steepness, int functionActivation,
                            int max_epochs, int epochs_between_reports,double desired_error);

double nn_fann_train3(struct fann_train_data *data,struct fann *ann,
                         int max_epochs,int epochs_between_reports,double desired_error);

struct fann *nn_fann_train4(struct fann_train_data *data, PGM_Vetor_Int *hidden,
                            NN_Fann_Create_Parameter *create_param, NN_Fann_Train_Parameter *train_param);

struct fann *nn_fann_train_bihiperbolic(struct fann_train_data *data, PGM_Vetor_Int *hidden,
                          double lam, double t1, double t2,
                          double steepness,int max_epochs,int epochs_between_reports,double desired_error);

// ------------ FUNÇÂO INTERNA

struct fann *nn_fann_train_create_standard_array(PGM_Vetor_Int *layers, double steepness, int functionActivation);
PGM_Vetor_Int *nn_fann_train_build_neuron_array(PGM_Vetor_Int *hidden, int in, int out);
void nn_fann_train_destroi_fann_train_data( struct fann_train_data* data );
#endif
