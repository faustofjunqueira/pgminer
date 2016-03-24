#include <string.h>
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
#include "access/htup_details.h" //Add heap_form_tuple

#include "../Util/pgm_matriz.h"
#include "../Util/pgm_matriz_util.h"
#include "../Util/pgm_string.h"
#include "../Util/pgm_malloc.h"

#include "types.i"


PG_FUNCTION_INFO_V1(pgm_c45);
Datum pgm_c45(PG_FUNCTION_ARGS);

void trainingC45(char* , char* , char* , char*);
void runC45(int *,char *,char *, bool);
void charMatrix2charVector(char **, int, char* );
int maxNElem(PGM_Vetor_String* , int );

Datum pgm_c45(PG_FUNCTION_ARGS){
    char *vector_atributos;
    PGM_Vetor_String *data = pgm_ArrayType2PGM_Vetor_String(PG_GETARG_ARRAYTYPE_P(0));
    int data_qtd_linha = PG_GETARG_INT32(1);
    int data_qtd_coluna = PG_GETARG_INT32(2);

    PGM_Vetor_String *atributos = pgm_ArrayType2PGM_Vetor_String(PG_GETARG_ARRAYTYPE_P(3));
    int atributos_qtd_linha = PG_GETARG_INT32(4);
    int atributos_qtd_coluna = PG_GETARG_INT32(5);


    PGM_Vetor_String *rotulo = pgm_ArrayType2PGM_Vetor_String(PG_GETARG_ARRAYTYPE_P(6));
    int rotulo_qtd_linha = PG_GETARG_INT32(7);
    int rotulo_qtd_coluna = PG_GETARG_INT32(8);

    char **matrix_data = pgm_vetor_string2charptr(data);
    char **matrix_atributos = pgm_vetor_string2charptr(atributos);
    char **matrix_rotulo = pgm_vetor_string2charptr(rotulo);
    char *vector_data;
    vector_data = (char*) pgm_malloc(data_qtd_linha*data_qtd_coluna*sizeof(char));
    charMatrix2charVector(matrix_data,data_qtd_linha, vector_data);


    vector_atributos = (char*) pgm_malloc(atributos_qtd_linha*atributos_qtd_coluna*sizeof(char));
    charMatrix2charVector(matrix_atributos,atributos_qtd_linha, vector_atributos);

    char *vector_rotulo;
    vector_rotulo = (char*) pgm_malloc(rotulo_qtd_linha*rotulo_qtd_coluna*sizeof(char));
    charMatrix2charVector(matrix_rotulo,rotulo_qtd_linha, vector_rotulo);

    char *vetor_atributos_rotulo = (char*) pgm_malloc (((rotulo_qtd_linha*rotulo_qtd_coluna) + (atributos_qtd_linha * atributos_qtd_coluna)) *sizeof(char));


    trainingC45(vector_data,vector_atributos,vector_rotulo,vetor_atributos_rotulo);

    pgm_free(vector_data);
    pgm_free(vetor_atributos_rotulo);

    char *hexAttRot = (char*) pgm_malloc((strlen(vetor_atributos_rotulo)+strlen(vetor_atributos_rotulo)+1)*sizeof(char));
    strcpy(hexAttRot,vetor_atributos_rotulo);
    strcat(hexAttRot,"@");
    strcat(hexAttRot,TRsHEX);


    pgm_free(matrix_data);
    pgm_free(matrix_atributos);
    pgm_free(matrix_rotulo);

    pgm_free(TRs);
    pgm_free(PrintedTRs);
    pgm_free(TRsHEX);

    PG_RETURN_CSTRING(hexAttRot);
}



PG_FUNCTION_INFO_V1(pgm_c45_test);
Datum pgm_c45_test(PG_FUNCTION_ARGS);

Datum pgm_c45_test(PG_FUNCTION_ARGS){


    PGM_Vetor_String *test = pgm_ArrayType2PGM_Vetor_String(PG_GETARG_ARRAYTYPE_P(0));
    int test_qtd_linha = PG_GETARG_INT32(1);
    int test_qtd_coluna = PG_GETARG_INT32(2);
    char *tree = PG_GETARG_POINTER(3);

    bool comRotulos = PG_GETARG_INT32(4); //PG_GET_ARG_POINTER(4);

    char **matrix_test = pgm_vetor_string2charptr(test);
    char *vector_test;
    vector_test = (char*) pgm_malloc(test_qtd_linha*test_qtd_coluna*sizeof(char));
    charMatrix2charVector(matrix_test,test_qtd_linha, vector_test);


    PGM_Vetor_Int *result = pgm_malloc(sizeof(PGM_Vetor_Int));
    int *rotulos = (int*) pgm_malloc(sizeof(int) * test_qtd_linha);
    elog(INFO,"%s",tree);

    char *vetor_att_rot = strtok(tree, "@");
    TRsHEX = strtok(NULL, "@");
    TRs = pgm_malloc(sizeof(unsigned char)*(strlen(TRsHEX)/2)+1);

    runC45(rotulos,vetor_att_rot,vector_test,comRotulos);

    result->n_elems=test_qtd_linha;
    result->valor=rotulos;

    //pgm_free(TRs);
    //pgm_free(TRsHEX);
    //pgm_free(tree);
    //pgm_free(vetor_att_rot);
    PG_RETURN_POINTER(result);
}

void sPrintTree(Tree t);
void GetNames(char *v);
void sGetTree();

PG_FUNCTION_INFO_V1(pgm_c45_print_tree);
Datum pgm_c45_print_tree(PG_FUNCTION_ARGS);

Datum pgm_c45_print_tree(PG_FUNCTION_ARGS){

    char *tree = PG_GETARG_POINTER(0);
    char *vetor_att_rot = strtok(tree, "@");
    TRsHEX = strtok(NULL, "@");
    TRs = pgm_malloc(sizeof(unsigned char)*(strlen(TRsHEX)/2)+1);

    GetNames(vetor_att_rot);
    sGetTree();
    sPrintTree(myTree);
    elog(INFO,"%s",PrintedTRs);
    PG_RETURN_CSTRING(PrintedTRs);
}


void charMatrix2charVector(char ** cmatrix, int qtdLinhas, char *vector_aux){
    int i;
    strcpy(vector_aux,cmatrix[0]);
    for(i=1 ; i < qtdLinhas; i++){
        strcat(vector_aux,cmatrix[i]);
    }
}
