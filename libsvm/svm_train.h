#ifndef SVM_TRAIN_H_INCLUDED
#define SVM_TRAIN_H_INCLUDED

#include "svm.h"

void exit_with_help(void);
void do_cross_validation(struct svm_problem *prob, struct svm_parameter *param,int nr_fold);
struct svm_problem* PGM_Matriz_Double2svm_problem(PGM_Matriz_Double *m, PGM_Vetor_Double* y, struct svm_parameter *param);
void printProblem(struct svm_problem *pt);
void printParam(struct svm_parameter *param);
void printModel(struct svm_model* model);

struct svm_model *string2svm_model(char *input);
char *next_delimeter(char *ptr);
struct svm_model* charptr2svm_model( char* str);
char *svm_model2charptr(struct svm_model *model);
#endif // SVM-TRAIN_H_INCLUDED
