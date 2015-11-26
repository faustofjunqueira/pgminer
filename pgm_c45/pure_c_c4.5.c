/*************************************************************************/
/*	Main routine, c4.5						                             */
/*************************************************************************/

#include "defns.i"
#include "types.i"
#include "extern.i"
#include "postgres.h"

#include "../Util/pgm_malloc.h"

/*
** External data, described in extern.i
*/
short		MaxAtt, MaxClass, MaxDiscrVal = 2;

ItemNo		MaxItem;

Description	*Item;

DiscrValue	*MaxAttVal;

char		*SpecialStatus;

String		 *ClassName,
		       *AttName,
		   **AttValName,
		FileName = "DF";

short		VERBOSITY = 0,
		    TRIALS    = 10;

Boolean		GAINRATIO  = true,
		    SUBSET     = false,
		    BATCH      = true,
		    UNSEENS    = true,
		    PROBTHRESH = false;

ItemNo		MINOBJS   = 2,
		    WINDOW    = 0,
		    INCREMENT = 0;

float		CF = 0.25;

Tree		*Pruned;

Boolean		AllKnown = true;

trainingC45(char *data, char *att, char *rot, char *atrot){
    /*
    ** Variaveis Utilizadas
    */
    short Best, BestTree();
    int o;
    extern char *optarg;
    extern int optind;
    Boolean FirstTime=true;

    /*
    ** Acerta o lista de atributos para poder ser consumida pelo C45
    */
    int index;
    char *found = strstr( att, ",\"" );

    while (found != NULL){
     index = found - att;
     att[index] = ':';
     att[index+1] = ' ';
     found = strstr( att,  ",\"" );
    }

     /*
     ** Acerta o lista de rotulos para poder ser consumida pelo C45
     */
     found = strstr( rot,  "(" );
     index = found - rot;
     rot[index] = ' ';

     found = strstr(rot, ")(");
     while(found != NULL){
     	index = found - rot;
        rot[index] = ',';
        rot[index + 1] =  ' ';
        found = strstr(rot, ")(");
     }

    /*
    ** Juntando os rotulos com os atributos
    */
    strcpy(atrot, rot);
    strcat(atrot, att);

    /*
    ** Função que pega os atributos e rotulos
    */
    GetNames(atrot);
    /*
    ** Função que pega os valores para o treinamento
    */
    GetData(data, true);

    /*
    ** Constuindo a arvore de decisao.
    */

    if ( BATCH ){
        TRIALS = 1;
        OneTree();
        Best = 0;
    }
    else{
        Best = BestTree();
    }


    /*
    ** Soften thresholds in best tree
    */

    if ( PROBTHRESH )
    {
        SoftenThresh(Pruned[Best]);
    }

    /*
    ** Save best tree
    */

    if ( BATCH || TRIALS == 1 )
    {
        printf("\nTree saved\n");
    }
    else
    {
        printf("\nBest tree from trial %d saved\n", Best);
    }

    /*
    ** Realizando a avaliacao do modelo
    */
    char *resultado = (char*) pgm_malloc(1024 * 10 * sizeof(char));
    char label_treinamento[100] = "\t\t ..:: TREINAMENTO ::.. \n ";
    strcat(resultado, label_treinamento);
    Evaluate(false, Best, resultado);
    elog(INFO,"%s",resultado);

    /****************************************************
    /* Exemplo de ida e volta para o postgres          */
    /****************************************************

    /* IDA: gerar a tree e enviar para o banco de dados */
   /* IDA: gerar a tree e enviar para o banco de dados */

   sSaveTree(Pruned[Best]);  // unsigned char *TRs      salva Pruned[Best] na variável TRs.
   sPrintTree(Pruned[Best]);

}

void runC45(int *rotulos,char* attrot,char *test, bool comRotulos)
{
    TRIALS = 1;
    GetNames(attrot);
    sGetTree(); // myTree recebe TRs
    GetData(test, comRotulos);
    GetRotulos(rotulos);
    // daqui para frente fazer a avaliação usando myTree e o conjunto de dados a ser rotulado a ser avaliado com o Evaluate() e GetRotulos()

}


