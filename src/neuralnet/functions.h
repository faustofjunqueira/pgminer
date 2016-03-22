#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include <math.h>

//Nesse Arquivo estão descritas funções de ativação para Redes neurais

#define sqr(x) ((x)*(x))

#define nn_function_abs(x) ((x) < 0 ? -(x) : (x))

#define NN_BI_HIPERBOLIC_LAMBDA_DEFAULT -4
#define NN_BI_HIPERBOLIC_T1_DEFAULT 2
#define NN_BI_HIPERBOLIC_T2_DEFAULT 2

extern double NN_BI_HIPERBOLIC_LAMBDA;
extern double NN_BI_HIPERBOLIC_T1;
extern double NN_BI_HIPERBOLIC_T2;

double nn_BiHiperbolic(double v);

double nn_DerivateBiHiperbolic(double v);

void nn_setBiHiperbolicDefaultParameter();

void nn_setBiHiperbolicParameter(double lam, double t1, double t2);

/* stepwise linear functions used for some of the activation functions */

/* defines used for the stepwise linear functions */

#define nn_linear_func(v1, r1, v2, r2, sum) (((((r2)-(r1)) * ((sum)-(v1)))/((v2)-(v1))) + (r1))
#define nn_stepwise(v1, v2, v3, v4, v5, v6, r1, r2, r3, r4, r5, r6, min, max, sum) (sum < v5 ? (sum < v3 ? (sum < v2 ? (sum < v1 ? min : nn_linear_func(v1, r1, v2, r2, sum)) : nn_linear_func(v2, r2, v3, r3, sum)) : (sum < v4 ? nn_linear_func(v3, r3, v4, r4, sum) : nn_linear_func(v4, r4, v5, r5, sum))) : (sum < v6 ? nn_linear_func(v5, r5, v6, r6, sum) : max))

/* NN_LINEAR */
/* #define NN_LINEAR(steepness, sum) fann_mult(steepness, sum) */
#define nn_linear_derive(steepness, value) (steepness)

/* NN_SIGMOID */
#define nn_sigmoid(steepness, sum) (1.0f/(1.0f + exp(-2.0f * steepness * sum)))
//#define NN_SIGMOID_real(sum) (1.0f/(1.0f + exp(-2.0f * sum)) - 1.0f)
#define nn_sigmoid_derive(steepness, value) (2.0f * steepness * value * (1.0f - value))

/* NN_SIGMOID_SYMMETRIC */
#define nn_sigmoid_symmetric(steepness, sum) (2.0f/(1.0f + exp(-2.0f * steepness * sum)) - 1.0f)
//#define NN_SIGMOID_symmetric_real(sum) (2.0f/(1.0f + exp(-2.0f * sum)) - 1.0f)
#define nn_sigmoid_symmetric_derive(steepness, value) steepness * (1.0f - (value*value))

/* NN_GAUSSIAN */
/* #define NN_GAUSSIAN(steepness, sum) (exp(-sum * steepness * sum * steepness)) */
#define nn_gaussian_real(sum) (exp(-sum * sum))
#define nn_gaussian_derive(steepness, value, sum) (-2.0f * sum * value * steepness * steepness)

/* NN_GAUSSIAN_SYMMETRIC */
/* #define NN_GAUSSIAN_symmetric(steepness, sum) ((exp(-sum * steepness * sum * steepness)*2.0)-1.0) */
#define nn_gaussian_symmetric_real(sum) ((exp(-sum * sum)*2.0f)-1.0f)
#define nn_gaussian_symmetric_derive(steepness, value, sum) (-2.0f * sum * (value+1.0f) * steepness * steepness)

/* NN_ELLIOT */
/* #define NN_ELLIOT(steepness, sum) (((sum * steepness) / 2.0f) / (1.0f + nn_function_abs(sum * steepness)) + 0.5f) */
#define nn_elliot_real(sum) (((sum) / 2.0f) / (1.0f + nn_function_abs(sum)) + 0.5f)
#define nn_elliot_derive(steepness, value, sum) (steepness * 1.0f / (2.0f * (1.0f + nn_function_abs(sum)) * (1.0f + nn_function_abs(sum))))

/* NN_ELLIOT_SYMMETRIC */
/* #define NN_ELLIOT_symmetric(steepness, sum) ((sum * steepness) / (1.0f + nn_function_abs(sum * steepness)))*/
#define nn_elliot_symmetric_real(sum) ((sum) / (1.0f + nn_function_abs(sum)))
#define nn_elliot_symmetric_derive(steepness, value, sum) (steepness * 1.0f / ((1.0f + nn_function_abs(sum)) * (1.0f + nn_function_abs(sum))))

/* NN_SIN_SYMMETRIC */
#define nn_sin_symmetric_real(sum) (sin(sum))
#define nn_sin_symmetric_derive(steepness, sum) (steepness*cos(steepness*sum))

/* NN_COS_SYMMETRIC */
#define nn_cos_symmetric_real(sum) (cos(sum))
#define nn_cos_symmetric_derive(steepness, sum) (steepness*-sin(steepness*sum))

/* NN_SIN */
#define nn_sin_real(sum) (sin(sum)/2.0f+0.5f)
#define nn_sin_derive(steepness, sum) (steepness*cos(steepness*sum)/2.0f)

/* NN_COS */
#define nn_cos_real(sum) (cos(sum)/2.0f+0.5f)
#define nn_cos_derive(steepness, sum) (steepness*-sin(steepness*sum)/2.0f)

#define nn_activation_switch(activation_function, value, result) \
switch(activation_function) \
{ \
	case NN_LINEAR: \
		result = (double)value; \
        break; \
	case NN_LINEAR_PIECE: \
		result = (double)((value < 0) ? 0 : (value > 1) ? 1 : value); \
        break; \
	case NN_LINEAR_PIECE_SYMMETRIC: \
		result = (double)((value < -1) ? -1 : (value > 1) ? 1 : value); \
        break; \
	case NN_SIGMOID: \
		result = (double)nn_sigmoid(1.0, value); \
        break; \
	case NN_SIGMOID_SYMMETRIC: \
		result = (double)nn_sigmoid_symmetric(1.0, value); \
        break; \
	case NN_SIGMOID_SYMMETRIC_STEPWISE: \
		result = (double)nn_stepwise(-2.64665293693542480469e+00, -1.47221934795379638672e+00, -5.49306154251098632812e-01, 5.49306154251098632812e-01, 1.47221934795379638672e+00, 2.64665293693542480469e+00, -9.90000009536743164062e-01, -8.99999976158142089844e-01, -5.00000000000000000000e-01, 5.00000000000000000000e-01, 8.99999976158142089844e-01, 9.90000009536743164062e-01, -1, 1, value); \
        break; \
	case NN_SIGMOID_STEPWISE: \
		result = (double)nn_stepwise(-2.64665246009826660156e+00, -1.47221946716308593750e+00, -5.49306154251098632812e-01, 5.49306154251098632812e-01, 1.47221934795379638672e+00, 2.64665293693542480469e+00, 4.99999988824129104614e-03, 5.00000007450580596924e-02, 2.50000000000000000000e-01, 7.50000000000000000000e-01, 9.49999988079071044922e-01, 9.95000004768371582031e-01, 0, 1, value); \
        break; \
	case NN_THRESHOLD: \
		result = (double)((value < 0) ? 0 : 1); \
        break; \
	case NN_THRESHOLD_SYMMETRIC: \
		result = (double)((value < 0) ? -1 : 1); \
        break; \
	case NN_GAUSSIAN: \
		result = (double)nn_gaussian_real(value); \
        break; \
	case NN_GAUSSIAN_SYMMETRIC: \
		result = (double)nn_gaussian_symmetric_real(value); \
        break; \
	case NN_ELLIOT: \
		result = (double)nn_elliot_real(value); \
	    break; \
	case NN_ELLIOT_SYMMETRIC: \
		result = (double)nn_elliot_symmetric_real(value); \
        break; \
	case NN_SIN_SYMMETRIC: \
		result = (double)nn_sin_symmetric_real(value); \
        break; \
	case NN_COS_SYMMETRIC: \
		result = (double)nn_cos_symmetric_real(value); \
        break; \
	case NN_SIN: \
		result = (double)nn_sin_real(value); \
        break; \
	case NN_COS: \
		result = (double)nn_cos_real(value); \
        break; \
	case NN_GAUSSIAN_STEPWISE: \
        result = 0; \
        break; \
    case NN_BIHIPERBOLIC: \
        result = (double) nn_BiHiperbolic(value); \
        break; \
}

enum nn_ActivationFuncEnum
{
	NN_LINEAR = 0,
	NN_THRESHOLD,
	NN_THRESHOLD_SYMMETRIC,
	NN_SIGMOID,
	NN_SIGMOID_STEPWISE,
	NN_SIGMOID_SYMMETRIC,
	NN_SIGMOID_SYMMETRIC_STEPWISE,
	NN_GAUSSIAN,
	NN_GAUSSIAN_SYMMETRIC,
	NN_GAUSSIAN_STEPWISE,
	NN_ELLIOT,
	NN_ELLIOT_SYMMETRIC,
	NN_LINEAR_PIECE,
	NN_LINEAR_PIECE_SYMMETRIC,
	NN_SIN_SYMMETRIC,
	NN_COS_SYMMETRIC,
	NN_SIN,
	NN_COS,
	NN_BIHIPERBOLIC
};

#define NN_FUNCTION_ACTIVATION 19

static char const *const NN_ACTIVATIONFUNC_NAMES[] = {
	"NN_LINEAR",
	"NN_THRESHOLD",
	"NN_THRESHOLD_SYMMETRIC",
	"NN_SIGMOID",
	"NN_SIGMOID_STEPWISE",
	"NN_SIGMOID_SYMMETRIC",
	"NN_SIGMOID_SYMMETRIC_STEPWISE",
	"NN_GAUSSIAN",
	"NN_GAUSSIAN_SYMMETRIC",
	"NN_GAUSSIAN_STEPWISE",
	"NN_ELLIOT",
	"NN_ELLIOT_SYMMETRIC",
	"NN_LINEAR_PIECE",
	"NN_LINEAR_PIECE_SYMMETRIC",
	"NN_SIN_SYMMETRIC",
	"NN_COS_SYMMETRIC",
	"NN_SIN",
	"NN_COS",
	"NN_BIHIPERBOLIC"
};


/*
Função que retorna a função de ativação a partir de uma string
Caso de erro ela retorna 0xFFFF
*/
unsigned short nn_FindFunctionActivation(char *word);

#endif
