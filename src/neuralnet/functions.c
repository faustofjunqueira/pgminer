

#include <math.h>
#include <string.h>

#include "functions.h"

 double NN_BI_HIPERBOLIC_LAMBDA = NN_BI_HIPERBOLIC_LAMBDA_DEFAULT;
 double NN_BI_HIPERBOLIC_T1 = NN_BI_HIPERBOLIC_T1_DEFAULT;
 double NN_BI_HIPERBOLIC_T2 = NN_BI_HIPERBOLIC_T2_DEFAULT;

double nn_BiHiperbolic(double v){
	//função aplicada no wolframalpha.com
	//sqrt((0.5^2)*((0.23+1/(4*0.5))^2)+(0.25)^2)-sqrt((0.5^2)*((0.23-1/(4*0.5))^2)+(0.25)^2)+0.5
	//Visualizar grafico em:
	//f(x)=sqrt((0.5^2)*((x+1/(4*0.5))^2)+(0.25)^2)-sqrt((0.5^2)*((x-1/(4*0.5))^2)+(0.25)^2)+0.5
	//f(x)=2*(sqrt((0.5^2)*((x+1/(4*0.5))^2)+(0.25)^2)-sqrt((0.5^2)*((x-1/(4*0.5))^2)+(0.25)^2)+0.5) -1


	double y = ((sqrt((sqr(NN_BI_HIPERBOLIC_LAMBDA))*(sqr((v + (1.0/(4.0*NN_BI_HIPERBOLIC_LAMBDA))))) + sqr(NN_BI_HIPERBOLIC_T1)))-
                (sqrt((sqr(NN_BI_HIPERBOLIC_LAMBDA))*(sqr((v - (1.0/(4.0*NN_BI_HIPERBOLIC_LAMBDA))))) + sqr(NN_BI_HIPERBOLIC_T2)))+0.5);
	return 2*y-1.0;
}

double nn_DerivateBiHiperbolic(double v){
	//f(x) = (((0.5^2)*(x + (1/(4*0.5))))/sqrt((0.5^2)*(x + (1/(4*0.5)))^2 +(0.25)^2))-(((0.5^2)*(x - (1/(4*0.5))))/sqrt((0.5^2)*(x - (1/(4*0.5)))^2 +(0.25)^2))
	double y = ((sqr(NN_BI_HIPERBOLIC_LAMBDA)*(v+(1.0/(4.0*NN_BI_HIPERBOLIC_LAMBDA))))/sqrt((sqr(NN_BI_HIPERBOLIC_LAMBDA)*sqr((v+(1.0/(4.0*NN_BI_HIPERBOLIC_LAMBDA)))))+sqr(NN_BI_HIPERBOLIC_T1)))-
	           ((sqr(NN_BI_HIPERBOLIC_LAMBDA)*(v-(1.0/(4.0*NN_BI_HIPERBOLIC_LAMBDA))))/sqrt((sqr(NN_BI_HIPERBOLIC_LAMBDA)*sqr((v-(1.0/(4.0*NN_BI_HIPERBOLIC_LAMBDA)))))+sqr(NN_BI_HIPERBOLIC_T2)));
	return 2*y;
}

unsigned short nn_FindFunctionActivation(char *word){
	int i;
	for( i = 0; i < NN_FUNCTION_ACTIVATION; i++)
		if(!strcmp(NN_ACTIVATIONFUNC_NAMES[i],word))
			return i;
	return 0xFFFF;
}

void nn_setBiHiperbolicDefaultParameter(){
    NN_BI_HIPERBOLIC_LAMBDA = NN_BI_HIPERBOLIC_LAMBDA_DEFAULT;
    NN_BI_HIPERBOLIC_T1 = NN_BI_HIPERBOLIC_T1_DEFAULT;
    NN_BI_HIPERBOLIC_T2 = NN_BI_HIPERBOLIC_T2_DEFAULT;
}

void nn_setBiHiperbolicParameter(double lam, double t1, double t2){
     NN_BI_HIPERBOLIC_LAMBDA = lam;
     NN_BI_HIPERBOLIC_T1 = t1;
     NN_BI_HIPERBOLIC_T2 = t2;
}
