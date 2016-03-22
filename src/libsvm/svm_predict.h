
#ifndef SVM_PREDICT_H_INCLUDED
#define SVM_PREDICT_H_INCLUDED

void predict(PGM_Matriz_Double *matrix,PGM_Vetor_Double *vector_label,struct svm_model *model,int predict_probability, PGM_Vetor_Double* predict_vector);

#endif // SVM_PREDICT_H_INCLUDED
