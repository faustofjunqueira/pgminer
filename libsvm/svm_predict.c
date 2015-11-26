/* SVM - Predict
 *
 * Autor: GZS, FFJ
 * GCCBD - COOPE - UFRJ
 */

#include "postgres.h"       //standard
#include "fmgr.h"           //standard
#include "utils/memutils.h"//standard
#include "utils/geo_decls.h"//standard
#include "utils/errcodes.h" //warning

#include "../Util/pgm_matriz.h"
#include "../Util/pgm_matriz_util.h"
#include "../Util/pgm_malloc.h"
#include "svm.h"
#include "svm_predict.h"


void predict(PGM_Matriz_Double *matrix,PGM_Vetor_Double *vector_label,struct svm_model *model,int predict_probability, PGM_Vetor_Double* predict_vector){

	int total = 0, correct = 0;

	double sump = 0, sumt = 0, sumpp = 0, sumtt = 0, sumpt = 0, error = 0;
    struct svm_node *x = (struct svm_node *) pgm_malloc((matrix->n_colunas+1)*sizeof(struct svm_node));
	int svm_type=svm_get_svm_type(model);
	int nr_class=svm_get_nr_class(model);
	double *prob_estimates=NULL;
	int j, linha = 0, count_vector = 0;

	if(predict_probability){
		if (svm_type==NU_SVR || svm_type==EPSILON_SVR)
			elog(INFO,"Prob. model for test data: target value = predicted value + z,\nz: Laplace distribution e^(-|z|/sigma)/(2sigma),sigma=%g\n",svm_get_svr_probability(model));
		else{
			int *labels=(int *) pgm_malloc(nr_class*sizeof(int));
			svm_get_labels(model,labels);
			prob_estimates = (double *) pgm_malloc(nr_class*sizeof(double));
			elog(INFO,"labels");
			for(j=0;j<nr_class;j++)
				elog(INFO," %d",labels[j]);
			pgm_free(labels);
		}
	}

	for(linha = 0; matrix->n_linhas > linha; linha++){
        int i = 0;
		double target_label, predict_label;

		target_label = PGM_ELEM_VETOR(vector_label,linha);

		for(j = 0; j < matrix->n_colunas; j++){
			if(PGM_ELEM_MATRIZ(matrix,linha,j) == 0){
				continue;
			}else{
				x[i].index = j+1;
				x[i].value = PGM_ELEM_MATRIZ(matrix,linha,j);
				i++;
			}
		}

		x[i].index = -1;
		i++;

		if (predict_probability && (svm_type==C_SVC || svm_type==NU_SVC)){
			predict_label = svm_predict_probability(model,x,prob_estimates);
			PGM_ELEM_VETOR(predict_vector,count_vector) = predict_label;
			count_vector++;
			for(j=0;j<nr_class;j++){
                PGM_ELEM_VETOR(predict_vector,count_vector) = prob_estimates[j];
                count_vector++;
			}
		}
		else{

			predict_label = svm_predict(model,x);
			PGM_ELEM_VETOR(predict_vector,count_vector) = predict_label;
			count_vector++;
		}

		if(predict_label == target_label)
			++correct;
		error += (predict_label-target_label)*(predict_label-target_label);
		sump += predict_label;
		sumt += target_label;
		sumpp += predict_label*predict_label;
		sumtt += target_label*target_label;
		sumpt += predict_label*target_label;
		++total;
	}

	if (svm_type==NU_SVR || svm_type==EPSILON_SVR){
		elog(INFO,"Mean squared error = %g (regression)\n",error/total);
		elog(INFO,"Squared correlation coefficient = %g (regression)\n",
			((total*sumpt-sump*sumt)*(total*sumpt-sump*sumt))/
			((total*sumpp-sump*sump)*(total*sumtt-sumt*sumt))
			);
	}
	else
		elog(INFO,"Accuracy = %g%% (%d/%d) (classification)\n",
			(double)correct/total*100,correct,total);

	if(predict_probability)
		pgm_free(prob_estimates);

}
