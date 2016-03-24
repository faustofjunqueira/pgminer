#ifndef PGM_NEURALNET_H
#define PGM_NEURALNET_H

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

#include "../Util/pgm_malloc.h"
#include "../Util/pgm_matriz.h"
#include "../Util/pgm_matriz_util.h"
#include "../Util/pgm_types.h"

#include "neuralnet.h"
#include "neuralnet_fann.h"
#include "neuralnet_fann_train.h"
#include "neuralnet_train_parameter.h"
#include "neuralnet_util.h"
#include "functions.h"

#define PGM_GETARG_POINTER(var,type_pointer,num_parameter) if(!PointerIsValid((var)=(type_pointer*)PG_GETARG_POINTER((num_parameter)))) elog(ERROR, "Ponteiro invalido")



#endif
