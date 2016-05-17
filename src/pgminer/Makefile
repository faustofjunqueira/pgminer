MODULES = pgminer
PG_CONFIG = pg_config
PGXS = $(shell $(PG_CONFIG) --pgxs)
INCLUDEDIR = $(shell $(PG_CONFIG) --includedir-server)
SRC_FANN = ../fann/src
INC_FANN = ../fann/src/include
OBJ_PATH = ../bin/obj
NEURALNET = $(OBJ_PATH)/neuralnet.o $(OBJ_PATH)/neuralnet_util.o $(OBJ_PATH)/neuralnet_parse.o $(OBJ_PATH)/functions.o $(OBJ_PATH)/doublefann.o $(OBJ_PATH)/neuralnet_fann.o $(OBJ_PATH)/neuralnet_fann_parse.o $(OBJ_PATH)/neuralnet_fann_train.o $(OBJ_PATH)/pgm_neuralnet.o $(OBJ_PATH)/neuralnet_train_parameter.o
#SOM
SOM = $(OBJ_PATH)/pgm_som_util.o $(OBJ_PATH)/pgm_som.o
OBJ = $(OBJ_PATH)/pgminer.o $(OBJ_PATH)/pure_c_kmeans.o $(OBJ_PATH)/pgm_kmeans.o $(OBJ_PATH)/pgm_matriz_util.o $(OBJ_PATH)/pgm_matriz.o $(OBJ_PATH)/pgm_malloc.o $(OBJ_PATH)/pgm_charptrbuf.o $(OBJ_PATH)/pacoteutilidadesql.o $(OBJ_PATH)/pgm_math_util.o $(OBJ_PATH)/pgm_math.o $(OBJ_PATH)/pgm_string.o $(OBJ_PATH)/las2.o $(OBJ_PATH)/pgm_svd.o $(OBJ_PATH)/pure_c_svd.o $(OBJ_PATH)/svdlib.o $(OBJ_PATH)/svdutil.o $(OBJ_PATH)/pgm_lapack_util.o $(OBJ_PATH)/pgm_lapack.o $(OBJ_PATH)/pgm_tsvector.o $(OBJ_PATH)/pgm_tsvector_util.o $(NEURALNET) $(SOM)
LAPACK_INC = /usr/include/lapacke
LAPACK_LIB = -llapack
MORECFLAGS = -O3 -funroll-loops -fprefetch-loop-arrays -march=native -msse4
#C45
C45 = pgm_c45.o pure_c_c4.5.o besttree.o build.o info.o discr.o contin.o subset.o prune.o stats.o st-thresh.o classify.o confmat.o sort.o getnames.o getdata.o trees.o getopt.o header.o
#DFG_FC
#DFG_FC = dfg_fc.o dfg_fc_wrappers.o
#    CUDA
#CUDA_PATH = /usr/local/cuda-5.0
#CUDA_LIB_PATH = $(CUDA_PATH)/lib64
#CUDA_LIB = -L$(CUDA_LIB_PATH) -lcudart -lcublas
#CUDA_INC_PATH = $(CUDA_PATH)/include
#CUDA_INC_FLAG = -I$(CUDA_INC_PATH) -I/usr/local/cuda-5.0/samples/common/inc/
#CUDA_CFLAGS = -g -O2 -fPIC -DLINUX_OOM_ADJ=0 -fPIC -DLINUX_OOM_ADJ=0 -Wall -Wpointer-arith -Wendif-labels -Wformat-security -fno-strict-aliasing -fwrapv -g -fpic

include $(PGXS)

all: pgminer.so

Clear:
	rm -rf ../bin

deploy: all
	rm -f /usr/lib/postgresql/$(VERSIONPG)/lib/pgminer.so
	cp ../bin/pgminer.so /usr/lib/postgresql/$(VERSIONPG)/lib/

createBinFolder:
	mkdir -p ../bin/obj

pgminer.so: createBinFolder $(OBJ)
	gcc -shared -o ../bin/pgminer.so $(OBJ) $(LAPACK_LIB) -lm -g

$(OBJ_PATH)/pgm_matriz.o: createBinFolder ../Util/pgm_matriz.c
	cc -o $@ -c ../Util/pgm_matriz.c $(MORECFLAGS) $(CFLAGS) -I$(INCLUDEDIR)

$(OBJ_PATH)/pgm_matriz_util.o: createBinFolder ../Util/pgm_matriz_util.c
	cc -o $@ -c ../Util/pgm_matriz_util.c $(CFLAGS) -I$(INCLUDEDIR)
$(OBJ_PATH)/pgm_malloc.o: createBinFolder ../Util/pgm_malloc.c
	cc -o $@ -c ../Util/pgm_malloc.c $(CFLAGS) -I$(INCLUDEDIR) -fPIC

$(OBJ_PATH)/pacoteutilidadesql.o: createBinFolder ../Util/pacutil/pacoteutilidadesql.c
	cc -o $@ -c ../Util/pacutil/pacoteutilidadesql.c $(CFLAGS) -I$(INCLUDEDIR)

$(OBJ_PATH)/pure_c_kmeans.o: createBinFolder ../pgm_kmeans/pure_c_kmeans.c
	cc -o $@ -c ../pgm_kmeans/pure_c_kmeans.c $(CFLAGS) -I$(INCLUDEDIR)

$(OBJ_PATH)/pgm_kmeans.o: createBinFolder ../pgm_kmeans/pgm_kmeans.c
	cc -o $@ -c ../pgm_kmeans/pgm_kmeans.c $(CFLAGS) -I$(INCLUDEDIR)

$(OBJ_PATH)/las2.o: createBinFolder ../pgm_SVD/las2.c
	cc -o $@ -c ../pgm_SVD/las2.c $(CFLAGS) -I$(INCLUDEDIR)

$(OBJ_PATH)/pgm_svd.o: createBinFolder ../pgm_SVD/pgm_svd.c
	cc -o $@ -c ../pgm_SVD/pgm_svd.c $(CFLAGS) -I$(INCLUDEDIR)

$(OBJ_PATH)/pure_c_svd.o: createBinFolder ../pgm_SVD/pure_c_svd.c
	cc -o $@ -c ../pgm_SVD/pure_c_svd.c $(CFLAGS) -I$(INCLUDEDIR)

$(OBJ_PATH)/svdlib.o: createBinFolder ../pgm_SVD/svdlib.c
	cc -o $@ -c ../pgm_SVD/svdlib.c $(CFLAGS) -I$(INCLUDEDIR)

$(OBJ_PATH)/svdutil.o: createBinFolder ../pgm_SVD/svdutil.c
	cc -o $@ -c ../pgm_SVD/svdutil.c $(CFLAGS) -I$(INCLUDEDIR)

$(OBJ_PATH)/pgminer.o: createBinFolder pgminer.c
	cc -o $@ -c pgminer.c $(CFLAGS) -I$(INCLUDEDIR)

$(OBJ_PATH)/pgm_lapack_util.o: createBinFolder ../pgm_lapack/pgm_lapack_util.c
	cc -o $@ -c ../pgm_lapack/pgm_lapack_util.c $(CFLAGS) -I$(INCLUDEDIR) -I$(LAPACK_INC)

$(OBJ_PATH)/pgm_lapack.o: createBinFolder ../pgm_lapack/pgm_lapack.c
	cc -o $@ -c ../pgm_lapack/pgm_lapack.c $(CFLAGS) -I$(INCLUDEDIR)

$(OBJ_PATH)/pgm_math_util.o: createBinFolder ../pgm_math/pgm_math_util.c
	cc -o $@ -c ../pgm_math/pgm_math_util.c $(CFLAGS) -I$(INCLUDEDIR)

$(OBJ_PATH)/pgm_math.o: createBinFolder ../pgm_math/pgm_math.c
	cc -o $@ -c ../pgm_math/pgm_math.c $(CFLAGS) -I$(INCLUDEDIR)

$(OBJ_PATH)/pgm_cublas.o: createBinFolder ../cublas/pgm_cublas.c
	cc -o $@ -c ../cublas/pgm_cublas.c  $(INCLUDES) -I$(INCLUDEDIR) $(CFLAGS)

$(OBJ_PATH)/pure_c_cublas.o: createBinFolder ../cublas/pure_c_cublas.c
	g++ -o $@ -c ../cublas/pure_c_cublas.c  $(CUDA_INC_FLAG) $(CUDA_CFLAGS)

$(OBJ_PATH)/pure_c_cublas_v2.o: createBinFolder ../cublas/pure_c_cublas_v2.c
	g++ -o $@ -c ../cublas/pure_c_cublas_v2.c  $(CUDA_INC_FLAG) $(CUDA_CFLAGS)

$(OBJ_PATH)/svm_train.o: createBinFolder ../libsvm/svm_train.c
	cc -o $@ -c ../libsvm/svm_train.c $(INCLUDES) -I$(INCLUDEDIR) $(CFLAGS)

$(OBJ_PATH)/svm_predict.o: createBinFolder ../libsvm/svm_predict.c
	cc -o $@ -c ../libsvm/svm_predict.c $(INCLUDES) -I$(INCLUDEDIR) $(CFLAGS)

$(OBJ_PATH)/svm.o: createBinFolder ../libsvm/svm.cpp
	g++ -o $@ -c ../libsvm/svm.cpp $(INCLUDES) -I$(INCLUDEDIR) -Wall -Wconversion -O3 -fPIC

$(OBJ_PATH)/pgm_svm.o: createBinFolder ../libsvm/pgm_svm.c
	cc -o $@ -c ../libsvm/pgm_svm.c $(INCLUDES) -I$(INCLUDEDIR) $(CFLAGS)

$(OBJ_PATH)/pgm_tsvector.o: createBinFolder ../pgm_tsvector/pgm_tsvector.c
	cc -o $@ -c ../pgm_tsvector/pgm_tsvector.c $(INCLUDES) -I$(INCLUDEDIR) $(CFLAGS)

$(OBJ_PATH)/pgm_tsvector_util.o: createBinFolder ../pgm_tsvector/pgm_tsvector_util.c
	cc -o $@ -c ../pgm_tsvector/pgm_tsvector_util.c $(INCLUDES) -I$(INCLUDEDIR) $(CFLAGS)

$(OBJ_PATH)/pgm_string.o: createBinFolder ../Util/pgm_string.c
	cc -o $@ -c ../Util/pgm_string.c $(INCLUDES) -I$(INCLUDEDIR) $(CFLAGS)

$(OBJ_PATH)/pgm_charptrbuf.o: createBinFolder ../Util/pgm_charptrbuf.c
	cc -o $@ -c ../Util/pgm_charptrbuf.c $(INCLUDES) -I$(INCLUDEDIR) $(CFLAGS)

$(OBJ_PATH)/pure_c_hash.o: createBinFolder ../hash/pure_c_hash.c
	cc -o $@ -c ../hash/pure_c_hash.c $(INCLUDES) -I$(INCLUDEDIR) $(CFLAGS)

$(OBJ_PATH)/pgm_hash.o: createBinFolder ../hash/pgm_hash.c
	cc -o $@ -c ../hash/pgm_hash.c $(INCLUDES) -I$(INCLUDEDIR) $(CFLAGS)

#INICIO NEURALNET

$(OBJ_PATH)/neuralnet.o: createBinFolder ../neuralnet/neuralnet.c
	gcc -o $(OBJ_PATH)/neuralnet.o -c ../neuralnet/neuralnet.c $(INCLUDES) -I$(INCLUDEDIR) $(CFLAGS)

$(OBJ_PATH)/neuralnet_util.o: createBinFolder ../neuralnet/neuralnet_util.c
	gcc -o $(OBJ_PATH)/neuralnet_util.o -c ../neuralnet/neuralnet_util.c $(INCLUDES) -I$(INCLUDEDIR) $(CFLAGS) -lm

$(OBJ_PATH)/neuralnet_parse.o: createBinFolder ../neuralnet/neuralnet_parse.c
	gcc -o $(OBJ_PATH)/neuralnet_parse.o -c ../neuralnet/neuralnet_parse.c $(INCLUDES) -I$(INCLUDEDIR) $(CFLAGS)

$(OBJ_PATH)/functions.o: createBinFolder ../neuralnet/functions.c
	gcc -o $(OBJ_PATH)/functions.o -c ../neuralnet/functions.c $(INCLUDES) -I$(INCLUDEDIR) $(CFLAGS) -lm

$(OBJ_PATH)/doublefann.o: createBinFolder ../neuralnet/fann/doublefann.c
	gcc -o $(OBJ_PATH)/doublefann.o -c ../neuralnet/fann/doublefann.c $(CFLAGS) -I$(INCLUDEDIR) -I../neuralnet/fann/ -I../neuralnet/fann/include

$(OBJ_PATH)/neuralnet_fann.o: createBinFolder ../neuralnet/neuralnet_fann.c
	gcc -o $(OBJ_PATH)/neuralnet_fann.o -c ../neuralnet/neuralnet_fann.c $(INCLUDES) -I$(INCLUDEDIR) $(CFLAGS)

$(OBJ_PATH)/neuralnet_fann_parse.o: createBinFolder ../neuralnet/neuralnet_fann_parse.c
	gcc -o $(OBJ_PATH)/neuralnet_fann_parse.o -c ../neuralnet/neuralnet_fann_parse.c $(INCLUDES) -I$(INCLUDEDIR) $(CFLAGS)

$(OBJ_PATH)/neuralnet_train_parameter.o: createBinFolder ../neuralnet/neuralnet_train_parameter.c
	gcc -o $(OBJ_PATH)/neuralnet_train_parameter.o -c ../neuralnet/neuralnet_train_parameter.c $(INCLUDES) -I$(INCLUDEDIR) $(CFLAGS)

$(OBJ_PATH)/neuralnet_fann_train.o: createBinFolder ../neuralnet/neuralnet_fann_train.c
	gcc -o $(OBJ_PATH)/neuralnet_fann_train.o -c ../neuralnet/neuralnet_fann_train.c $(INCLUDES) -I$(INCLUDEDIR) $(CFLAGS)

$(OBJ_PATH)/pgm_neuralnet.o: createBinFolder ../neuralnet/pgm_neuralnet.c
	gcc -o $(OBJ_PATH)/pgm_neuralnet.o -c ../neuralnet/pgm_neuralnet.c $(INCLUDES) -I$(INCLUDEDIR) $(CFLAGS)

#FIM NEURALNET

#INICIO C45

pgm_c45.o: createBinFolder ../pgm_c45/pgm_c45.c
	cc -o pgm_c45.o -c ../pgm_c45/pgm_c45.c $(INCLUDES) -I$(INCLUDEDIR) $(CFLAGS)
pure_c_c4.5.o: createBinFolder ../pgm_c45/pure_c_c4.5.c
	cc -o pure_c_c4.5.o -c ../pgm_c45/pure_c_c4.5.c $(INCLUDES) -I$(INCLUDEDIR) $(CFLAGS)
besttree.o: createBinFolder ../pgm_c45/besttree.c
	cc -o besttree.o -c ../pgm_c45/besttree.c $(INCLUDES) -I$(INCLUDEDIR) $(CFLAGS)
build.o: createBinFolder ../pgm_c45/build.c
	cc -o build.o -c ../pgm_c45/build.c $(INCLUDES) -I$(INCLUDEDIR) $(CFLAGS)
info.o: createBinFolder ../pgm_c45/info.c
	cc -o info.o -c ../pgm_c45/info.c $(INCLUDES) -I$(INCLUDEDIR) $(CFLAGS)
discr.o: createBinFolder ../pgm_c45/discr.c
	cc -o discr.o -c ../pgm_c45/discr.c $(INCLUDES) -I$(INCLUDEDIR) $(CFLAGS)
contin.o: createBinFolder ../pgm_c45/contin.c
	cc -o contin.o -c ../pgm_c45/contin.c $(INCLUDES) -I$(INCLUDEDIR) $(CFLAGS)
subset.o: createBinFolder ../pgm_c45/subset.c
	cc -o subset.o -c ../pgm_c45/subset.c $(INCLUDES) -I$(INCLUDEDIR) $(CFLAGS)
prune.o: createBinFolder ../pgm_c45/prune.c
	cc -o prune.o -c ../pgm_c45/prune.c $(INCLUDES) -I$(INCLUDEDIR) $(CFLAGS)
stats.o: createBinFolder ../pgm_c45/stats.c
	cc -o stats.o -c ../pgm_c45/stats.c $(INCLUDES) -I$(INCLUDEDIR) $(CFLAGS)
st-thresh.o: createBinFolder ../pgm_c45/st-thresh.c
	cc -o st-thresh.o -c ../pgm_c45/st-thresh.c $(INCLUDES) -I$(INCLUDEDIR) $(CFLAGS)
classify.o: createBinFolder ../pgm_c45/classify.c
	cc -o classify.o -c ../pgm_c45/classify.c $(INCLUDES) -I$(INCLUDEDIR) $(CFLAGS)
confmat.o: createBinFolder ../pgm_c45/confmat.c
	cc -o confmat.o -c ../pgm_c45/confmat.c $(INCLUDES) -I$(INCLUDEDIR) $(CFLAGS)
sort.o: createBinFolder ../pgm_c45/sort.c
	cc -o sort.o -c ../pgm_c45/sort.c $(INCLUDES) -I$(INCLUDEDIR) $(CFLAGS)
getnames.o: createBinFolder ../pgm_c45/getnames.c
	cc -o getnames.o -c ../pgm_c45/getnames.c $(INCLUDES) -I$(INCLUDEDIR) $(CFLAGS)
getdata.o: createBinFolder ../pgm_c45/getdata.c
	cc -o getdata.o -c ../pgm_c45/getdata.c $(INCLUDES) -I$(INCLUDEDIR) $(CFLAGS)
trees.o: createBinFolder ../pgm_c45/trees.c
	cc -o trees.o -c ../pgm_c45/trees.c $(INCLUDES) -I$(INCLUDEDIR) $(CFLAGS)
getopt.o: createBinFolder ../pgm_c45/getopt.c
	cc -o getopt.o -c ../pgm_c45/getopt.c $(INCLUDES) -I$(INCLUDEDIR) $(CFLAGS)
header.o: createBinFolder ../pgm_c45/header.c
	cc -o header.o -c ../pgm_c45/header.c $(INCLUDES) -I$(INCLUDEDIR) $(CFLAGS)

#FIM C45

#INICIO SOM

$(OBJ_PATH)/pgm_som_util.o: createBinFolder ../pgm_som/pgm_som_util.c
	cc -o $(OBJ_PATH)/pgm_som_util.o -c ../pgm_som/pgm_som_util.c $(INCLUDES) -I$(INCLUDEDIR) $(CFLAGS)

$(OBJ_PATH)/pgm_som.o: createBinFolder
	cc -o $(OBJ_PATH)/pgm_som.o -c ../pgm_som/pgm_som.c $(INCLUDES) -I$(INCLUDEDIR) $(CFLAGS)

#FIM SOM

#INICIO DFG_FC

dfg_fc.o: createBinFolder ../dfg_fc/dfg_fc.cpp
	g++ -o dfg_fc.o -c ../dfg_fc/dfg_fc.cpp $(INCLUDES) -I$(INCLUDEDIR) -Wall -fPIC -g

dfg_fc_wrappers.o: createBinFolder ../dfg_fc/dfg_fc_wrappers.c
	cc -o dfg_fc_wrappers.o -c ../dfg_fc/dfg_fc_wrappers.c $(INCLUDES) -I$(INCLUDEDIR) $(CFLAGS) -g

#FIM DFG_FC
