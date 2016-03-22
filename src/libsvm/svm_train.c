#include "postgres.h"       //standard
#include "fmgr.h"           //standard
#include "utils/memutils.h"//standard
#include "utils/geo_decls.h"//standard
#include "utils/errcodes.h" //warning

#include "../Util/pgm_matriz.h"
#include "../Util/pgm_matriz_util.h"
#include "../Util/pgm_malloc.h"
#include "../Util/pgm_charptrbuf.h"

#include "svm.h"
#include "svm_train.h"
#define Malloc(type,n) (type *) pgm_malloc ((n)*sizeof(type))
#define Free(n) pgm_free(n)

char *svm_type_table[6] =
{
	"c_svc","nu_svc","one_class","epsilon_svr","nu_svr",NULL
};

char *kernel_type_table[6]=
{
	"linear","polynomial","rbf","sigmoid","precomputed",NULL
};

void exit_with_help(void){
	elog(ERROR,
	"Usage: svm-train [options] training_set_file [model_file]\n"
	"options:\n"
	"-s svm_type : set type of SVM (default 0)\n"
	"	0 -- C-SVC		(multi-class classification)\n"
	"	1 -- nu-SVC		(multi-class classification)\n"
	"	2 -- one-class SVM\n"
	"	3 -- epsilon-SVR	(regression)\n"
	"	4 -- nu-SVR		(regression)\n"
	"-t kernel_type : set type of kernel function (default 2)\n"
	"	0 -- linear: u'*v\n"
	"	1 -- polynomial: (gamma*u'*v + coef0)^degree\n"
	"	2 -- radial basis function: exp(-gamma*|u-v|^2)\n"
	"	3 -- sigmoid: tanh(gamma*u'*v + coef0)\n"
	"	4 -- precomputed kernel (kernel values in training_set_file)\n"
	"-d degree : set degree in kernel function (default 3)\n"
	"-g gamma : set gamma in kernel function (default 1/num_features)\n"
	"-r coef0 : set coef0 in kernel function (default 0)\n"
	"-c cost : set the parameter C of C-SVC, epsilon-SVR, and nu-SVR (default 1)\n"
	"-n nu : set the parameter nu of nu-SVC, one-class SVM, and nu-SVR (default 0.5)\n"
	"-p epsilon : set the epsilon in loss function of epsilon-SVR (default 0.1)\n"
	"-m cachesize : set cache memory size in MB (default 100)\n"
	"-e epsilon : set tolerance of termination criterion (default 0.001)\n"
	"-h shrinking : whether to use the shrinking heuristics, 0 or 1 (default 1)\n"
	"-b probability_estimates : whether to train a SVC or SVR model for probability estimates, 0 or 1 (default 0)\n"
	"-wi weight : set the parameter C of class i to weight*C, for C-SVC (default 1)\n"
	"-v n: n-fold cross validation mode(n > 2)\n"
	);
}

void do_cross_validation(struct svm_problem *prob, struct svm_parameter *param,int nr_fold){
	int i;
	int total_correct = 0;
	double total_error = 0;
	double sumv = 0, sumy = 0, sumvv = 0, sumyy = 0, sumvy = 0;
	double *target = Malloc(double,prob->l);

	svm_cross_validation(prob,param,nr_fold,target);
	if(param->svm_type == EPSILON_SVR ||
	   param->svm_type == NU_SVR)
	{
		for(i=0;i<prob->l;i++)
		{
			double y = prob->y[i];
			double v = target[i];
			total_error += (v-y)*(v-y);
			sumv += v;
			sumy += y;
			sumvv += v*v;
			sumyy += y*y;
			sumvy += v*y;
		}
		elog(INFO,"Cross Validation Mean squared error = %g\n",total_error/prob->l);
		elog(INFO,"Cross Validation Squared correlation coefficient = %g\n",
			((prob->l*sumvy-sumv*sumy)*(prob->l*sumvy-sumv*sumy))/
			((prob->l*sumvv-sumv*sumv)*(prob->l*sumyy-sumy*sumy))
			);
	}
	else
	{
		for(i=0;i<prob->l;i++)
			if(target[i] == prob->y[i])
				++total_correct;
		elog(INFO,"Cross Validation Accuracy = %g%%\n",100.0*total_correct/prob->l);
	}
	pgm_free(target);
}

struct svm_problem* PGM_Matriz_Double2svm_problem(PGM_Matriz_Double *m, PGM_Vetor_Double* y, struct svm_parameter *param){
    struct svm_problem *prob = Malloc(struct svm_problem, 1);
	int elements,i, j, count = 0, max_index = 0 , max_index_cur = 0;
    struct svm_node *x_space;

	elements = prob->l = m->n_linhas;

    for ( i = 0; i < m->n_linhas*m->n_colunas; i++)
        if (m->valor[i] != 0) elements++;

	prob->y = Malloc(double,prob->l);
	prob->x = Malloc(struct svm_node *,prob->l);
	x_space = Malloc(struct svm_node,elements);

	for( i = 0; i < m->n_linhas; i++){
	    prob->x[i] = &x_space[count];
        for( j = 0; j < m->n_colunas; j++){
            double element_cur = PGM_ELEM_MATRIZ(m,i,j);
            if(element_cur != 0){
                x_space[count].index = j+1;
                x_space[count].value = element_cur;
                count++;
                max_index_cur = j+1;
                if(max_index < max_index_cur) max_index = max_index_cur;
            }

        }
        x_space[count].index = -1;
        x_space[count].value = 0.0;
        count++;
        prob->y[i] = y->valor[i];
	}

    if(param->gamma == 0 && max_index > 0)
		param->gamma = 1.0/max_index;

	if(param->kernel_type == PRECOMPUTED)
		for(i=0;i<prob->l;i++){
			if (prob->x[i][0].index != 0){
				elog(ERROR,"Wrong input format: first column must be 0:sample_serial_number\n");
			}
			if ((int)prob->x[i][0].value <= 0 || (int)prob->x[i][0].value > max_index){
				elog(ERROR,"Wrong input format: sample_serial_number out of range\n");
			}
		}
    return prob;
}

void printProblem(struct svm_problem *pt){
    int i,j;

    elog(INFO,"\nL: %d\n\nElementos de *y = ", pt->l);
    for ( i = 0; i < pt->l; i++){
        elog(INFO," %lf", pt->y[i]);
    }
    elog(INFO,"\n\nElementos de x\n");
    for (i = 0; i < pt->l; i++){
        for( j = 0; j < 3; j++)
            elog(INFO,"(%d - %lf){%d} ",pt->x[i][j].index,pt->x[i][j].value,j);
        elog(INFO,"\n");
    }
}

void printParam(struct svm_parameter *param){

    elog(INFO,"svm_type 	%d	\n	",param->svm_type);
    elog(INFO,"kernel_type 	%d	\n	",param->kernel_type);
    elog(INFO,"degree 	%d	\n	",param->degree);
    elog(INFO,"gamma 	%lf	\n	",param->gamma);
    elog(INFO,"coef0 	%lf	\n	",param->coef0);
    elog(INFO,"cache_size 	%lf	\n	",param->cache_size);
    elog(INFO,"eps 	%lf	\n	",param->eps);
    elog(INFO,"C 	%lf	\n	",param->C);
    elog(INFO,"nu 	%lf	\n	",param->nu);
    elog(INFO,"p 	%lf	\n	",param->p);
    elog(INFO,"shrinking 	%d	\n	",param->shrinking);
    elog(INFO,"probability 	%d	\n	",param->probability);

}

void printModel(struct svm_model* model){
    int i,n;
    printParam(&model->param);
    elog(INFO,"nr_class %d",model->nr_class);
    elog(INFO,"l %d",model->l);
    n = model->nr_class * (model->nr_class-1)/2;
    if(model->rho != NULL)
        for(i = 0; i < n; i++)
            elog(INFO,"rho[%d] %lf",i,model->rho[i]);
    if(model->probA != NULL)
        for ( i = -0; i < n; i++)
            elog(INFO,"probA[%d] %lf",i,model->probA[i]);
    if(model->probB != NULL)
        for ( i = -0; i < n; i++)
            elog(INFO,"probB[%d] %lf",i,model->probB[i]);
    if(model->sv_indices != NULL)
        for ( i = 0; i < model->nr_class; i++)
            elog(INFO,"sv_indices[%d] %d",i,model->sv_indices[i]);
    if(model->label != NULL)
        for ( i = 0; i < model->nr_class; i++)
            elog(INFO,"sv_indices[%d] %d",i,model->label[i]);
    if(model->nSV != NULL)
        for ( i = 0; i < model->nr_class; i++)
            elog(INFO,"nSV[%d] %d",i,model->nSV[i]);
    elog(INFO,"free_sv %d",model->free_sv);
}

char *svm_model2charptr(struct svm_model *model){
	int i,j;
	unsigned int n_nodes;
	CharptrBuf *buf_iter = CharptrBuf_init(1024);
	double **sv_coef = model->sv_coef; 
	struct svm_node **SV = model->SV;
	int cap_out = 128*1024;
	CharptrBuf *result = CharptrBuf_init(cap_out);
	char *buffer = (char*) malloc (sizeof(char*)*64);
	struct svm_parameter param = model->param;

	//Write Parameter
	sprintf(buffer,"#%d#%d#%d#%lf#%lf",param.svm_type,param.kernel_type,param.degree,param.gamma,param.coef0);
	CharptrBuf_append(result,buffer);

	sprintf(buffer,"#%d#%d#",model->nr_class,model->l);
	CharptrBuf_append(result,buffer);
	
	for(i = 0; i < (model->nr_class*(model->nr_class-1)/2);i++){
		sprintf(buffer,"%lf#",model->rho[i]);
		CharptrBuf_append(result,buffer);
	}

	for(i=0;i<model->nr_class;i++){
		sprintf(buffer,"%d#",model->label[i]);
		CharptrBuf_append(result,buffer);
	}


	if(model->probA){
		CharptrBuf_append(result,"1#");
		for(i=0;i<(model->nr_class*(model->nr_class-1)/2);i++){
			sprintf(buffer,"%lf#",model->probA[i]);
			CharptrBuf_append(result,buffer);
		}
	}
	if(model->probB){
		CharptrBuf_append(result,"2#");
		for(i=0;i<(model->nr_class*(model->nr_class-1)/2);i++){
			sprintf(buffer,"%lf#",model->probB[i]);
			CharptrBuf_append(result,buffer);
		}
	}

	if(model->nSV)
	{
		CharptrBuf_append(result,"3#");
		for(i=0;i<model->nr_class;i++){
			sprintf(buffer,"%d#",model->nSV[i]);
			CharptrBuf_append(result,buffer);
		}
	}
	
	CharptrBuf_append(result,"0#");
//	Quando aparecer no vetor 1 quer dizer que tem probA quando aparecer 2 tem probB quando aparecer 3 tem nSV, quando aparecer o 0 nao tem nem A, nem B,nem nSV para serem lido	

	for(i=0;i<model->l;i++)
	{	
		struct svm_node *p = SV[i];
		n_nodes = 0;		
		for(j=0;j<model->nr_class-1;j++){
			sprintf(buffer,"%lf#",sv_coef[j][i]);
			CharptrBuf_append(result,buffer);
		}		
		if(param.kernel_type == PRECOMPUTED){
			sprintf(buffer,"0#%d#",(int)(p->value));
			CharptrBuf_append(buf_iter,buffer);
			n_nodes++;
		}else
			while(p->index != -1){				
				sprintf(buffer,"%d#%lf#",p->index,p->value);
				CharptrBuf_append(buf_iter,buffer);
				p++;
				n_nodes++;
			}
		sprintf(buffer,"%d#",n_nodes);
		CharptrBuf_append(result,buffer);
		CharptrBuf_append(result,buf_iter->buffer);
		CharptrBuf_clear(buf_iter);
	}
	CharptrBuf_destroy(&buf_iter);
	CharptrBuf_append(result,"#");
	return PGM_CHARPTRBUF_GET_DATA(result); 
}

char *next_delimeter(char *ptr){
	while(*ptr != '#') ptr++; 
	while(*ptr == '#') ptr++;
	return ptr;
}

struct svm_model* charptr2svm_model( char* str){
	int i,j;
	unsigned int n_nodes;
	int tem_prob = 0;
	char *ptr = str;
	struct svm_model *model = (struct svm_model*) pgm_malloc (sizeof(struct svm_model));
	model->rho = NULL;
	model->probA = NULL;
	model->probB = NULL;
	model->sv_indices = NULL;
	model->label = NULL;
	model->nSV = NULL;

	if(sscanf(ptr,"#%d#%d#%d#%lf#%lf#%d#%d",&model->param.svm_type,&model->param.kernel_type,&model->param.degree,&model->param.gamma,&model->param.coef0,&model->nr_class,&model->l) < 7){
		pgm_free(model);
		return NULL;
	}

	for(i = 0; i < 8; i++)	ptr = next_delimeter(ptr);

	model->rho = (double*) pgm_malloc(sizeof(double)*(model->nr_class*(model->nr_class-1)/2));

	for(i = 0; i < model->nr_class*(model->nr_class-1)/2;i++){
		sscanf(ptr,"%lf",&model->rho[i]);
		ptr = next_delimeter(ptr);
	}

	model->label = (int*) pgm_malloc (sizeof(int)*model->nr_class);

	for(i=0;i<model->nr_class;i++){
		sscanf(ptr,"%d",&model->label[i]);
		ptr = next_delimeter(ptr);
	}
	
	do{
		sscanf(ptr,"%d",&tem_prob);
		ptr = next_delimeter(ptr);
		
		if(tem_prob == 1){
			model->probA = (double*) pgm_malloc (sizeof(double)*(model->nr_class*(model->nr_class-1)/2));
			for(i=0;i<(model->nr_class*(model->nr_class-1)/2);i++){
				sscanf(ptr,"%lf",&model->probA[i]);
				ptr = next_delimeter(ptr);
			}
		}else if(tem_prob == 2){
			model->probB = (double*) pgm_malloc (sizeof(double)*(model->nr_class*(model->nr_class-1)/2));
			for(i=0;i<(model->nr_class*(model->nr_class-1)/2);i++){
				sscanf(ptr,"%lf",&model->probB[i]);
				ptr = next_delimeter(ptr);
			}
		}else if(tem_prob == 3){
			model->nSV = (int*) pgm_malloc(sizeof(int)*(model->nr_class*(model->nr_class-1)/2));
			for(i=0;i<model->nr_class;i++){
				sscanf(ptr,"%d",&model->nSV[i]);
				ptr = next_delimeter(ptr);
			}
		}
	}while(tem_prob);

	model->sv_coef = (double**) pgm_malloc (sizeof(double*)*model->nr_class - 1);
	
	for(i = 0; i < model->nr_class - 1; i++) 
		model->sv_coef[i] = (double*) pgm_malloc (sizeof(double)*model->l);

	model->SV = (struct svm_node**) pgm_malloc (sizeof(struct svm_node)*model->l);
	
	
	for(i=0;i<model->l;i++){
		for(j=0;j<model->nr_class-1;j++){
			sscanf(ptr,"%lf",&(model->sv_coef[j][i]));
			ptr = next_delimeter(ptr);
		}
		sscanf(ptr,"%d",&n_nodes);
		ptr = next_delimeter(ptr);
		model->SV[i] = (struct svm_node*) pgm_malloc(sizeof(struct svm_node)*n_nodes);
		for(j = 0; j < n_nodes; j++){		
			sscanf(ptr,"%d#%lf#",&(model->SV[i][j].index),&(model->SV[i][j].value));
			ptr = next_delimeter(ptr);
			ptr = next_delimeter(ptr);
		}
	}

	return model;
}
