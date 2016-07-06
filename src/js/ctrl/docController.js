(function(app){

  app.controller('DocController', function($scope,parametroFactory, retornoFactory, tipoFactory, operadorFactory, funcaoFactory){

    function setup(lista){
      var r = angular.copy(lista);
      for(var i in lista){
        if(lista[i].tipo){
          lista[i].funcoes.forEach(function(f){
            r.push(f);
          });
        }
      }
      return r;
    }

    var listaFuncoes = [
      {
        nome: 'prepare_data_to_learn',
        descricao:'Cria tabela temporária com valores organizados para treinamento, já normalizado. OBS: a saída é sempre normalizada no intervalo (tol, 1-tol), exceto se tol >= 0.5 (nesse caso não há normalização da saída). Se min >= max, não há normalização da entrada. Veja create_nn.',
        cabecalho:'prepare_data_to_learn(sql text, nome_tabela text, k_folds integer DEFAULT 5, keep_col_ratio boolean DEFAULT true, min_e double precision DEFAULT (-1), max_e double precision DEFAULT 1, tol double precision DEFAULT 0.001) RETURNS bigint',
        parametros: [
          parametroFactory.criar('sql', 'text', "SQL eh uma consulta de um dos dois tipos a seguir, representando uma classificação ou uma regressão: 1) Classificação: 'select id, entrada, classe from tabela' Ex: 'select id, array[ petal_w, sepal_w, petal_l, sepal_l ] entrada, classe from iris' classe pode ser integer ou text, e será convertido para uma representação vetorial do tipo (0,1,0,0,0); 2) Regressão: 'select id, entrada, saida from tabela' Ex: 'select x*100+y id, array[ x, y ] entrada, array[ x*y ] saida from generate_series( 1, 10 ) x, generate_series( 1, 10 ) y'"),
          parametroFactory.criar('nome_tabela', 'text', 'nome da tabela a ser criada, contendo as seguintes colunas: id, fold, entrada, saida'),
          parametroFactory.criar('k_folds', 'integer', 'número de folds na tabela a ser criada; default 5'),
          parametroFactory.criar('keep_col_ratio', 'boolean', 'define se a normalização manterá a razão entre as colunas; default true'),
          parametroFactory.criar('min_e,max_e,tol', 'double precision', 'normalização da entrada no intervalo (min+tol, max-tol)'),
        ],
        retorno:[
          retornoFactory.criar('nlines', 'bigint', 'número de linhas criadas')
        ]
      },
      {
        nome: 'apply_inverse_sigmoid',
        descricao:'Aplica a função inversa da sigmoid sobre cada elemento x 1da matriz m. Inversa da sigmoid: -log( 2.0/(x+1.0) -1.0)/16.0',
        cabecalho:'apply_inverse_sigmoid(in m matrix, out c matrix) RETURNS matrix',
        parametros: [
          parametroFactory.criar('m', 'matrix', 'Matriz de valores')
        ],
        retorno:[
          retornoFactory.criar('c', 'matrix', 'Matriz com valores aplicados na função')
        ]
      },
      {
        nome: 'apply_sigmoid',
        descricao:'Aplica a função sigmoid sobre cada elemento x da matriz m. Sigmoid: 2.0/(1.0 + exp( -16.0*x ) ) - 1.0',
        cabecalho:'apply_sigmoid(in m matrix, out c matrix) RETURNS matrix',
        parametros: [
          parametroFactory.criar('m', 'matrix', 'Matriz de valores')
        ],
        retorno:[
          retornoFactory.criar('c', 'matrix', 'Matriz com valores aplicados na função sigmoid')
        ]
      },
      {
        nome: 'blas_multiply',
        descricao:'Função executa a multiplicação de duas matrizes. C = A * B',
        cabecalho:'blas_multiply(a bigint, b bigint) RETURNS bigint',
        parametros: [
          parametroFactory.criar('A', 'bigint', 'Ponteiro para Matriz A'),
          parametroFactory.criar('B', 'bigint', 'Ponteiro para Matriz B')
        ],
        retorno:[
          retornoFactory.criar('C', 'bigint', 'Ponteiro para matriz resultante')
        ]
      },
      {
        nome: 'classe2vector',
        descricao:'Transforma o conjunto de classes para vetores de valores 0 a 1',
        cabecalho:'classe2vector(sql_distinct_classes text, OUT classe text, OUT vector double precision[]) RETURNS SETOF record',
        parametros: [
          parametroFactory.criar('sql_distinct_classes', 'text', "EX: select * from classe2vector( 'select distinct classe from iris order by classe' )")
        ],
        retorno:[
          retornoFactory.criar('classe', 'matrix', 'Matriz com valores aplicados na função sigmoid'),
          retornoFactory.criar('vector', 'double precision[]', 'Matriz com valores aplicados na função sigmoid')
        ]
      },
      {
        nome: 'cleanup_table',
        descricao:'Zera ou cria uma tabela',
        cabecalho:'cleanup_table(table_name text, table_columns text, make_temp bool)',
        parametros: [
          parametroFactory.criar('table_name', 'text', 'Nome da tabela'),
          parametroFactory.criar('table_columns', 'text', "Colunas da tabela. Ex: '( GRUPO INTEGER PRIMARY KEY, VALOR FLOAT8[] NOT NULL )'"),
          parametroFactory.criar('make_temp', 'bool', 'Se true, a tabela será temporária, caso contrário a tabela será permanente')
        ],
        retorno:[]
      },
      {
        nome: 'cos',
        descricao:'Aplica a função cosseno nos pontos',
        cabecalho:['cos(a double precision[], b double precision[])','cos(a matrix, b matrix)'],
        parametros: [
          parametroFactory.criar('A', 'double precision[] | matrix', 'Matriz ou vetor A'),
          parametroFactory.criar('B', 'double precision[] | matrix', 'Matriz ou vetor B')
        ],
        retorno:[
          retornoFactory.criar('cosseno', 'double precision', 'Valor da função cosseno')
        ]
      },

      {
        nome: 'create_fann',
        descricao:'Função cria um RNA baseada na struct da FANN',
        cabecalho:"create_fann(table_data text, layers integer[], validation_fold text DEFAULT '1'::text, test_fold text DEFAULT '1'::text, max_epochs integer DEFAULT 500, epochs_between_report integer DEFAULT 10, OUT net fann, OUT mse_train double precision, OUT mse_validation double precision, OUT mse_test double precision, OUT misclassified_train integer, OUT misclassified_validation integer, OUT misclassified_test integer) RETURNS SETOF record",
        parametros: [
          parametroFactory.criar("table_data", 'text','Deve ser uma tabela preparada por prepare_data_to_learn, tendo as colunas id, fold, entrada, saida, com entrada e saida normalizadas'),
          parametroFactory.criar("layers", 'integer[]','array com o número de neurônios das camadas intermediárias(hdden layers)'),
          parametroFactory.criar("validation_fold", 'text', "fold de validação. default: 1"),
          parametroFactory.criar("test_fold", 'text', 'fold de teste. default: 1'),
          parametroFactory.criar("max_epochs", 'integer', "número máximo de épocas. default: 500"),
          parametroFactory.criar("epochs_between_report", 'integer', "número de epocas entre relatórios. default: 10")
        ],
        retorno:[
          retornoFactory.criar("mse_train", "double precision", "erro mínimo quadrado"),
          retornoFactory.criar("mse_validation", "double precision", "erro mínimo quadrado"),
          retornoFactory.criar("mse_test", "double precision", "erro mínimo quadrado"),
          retornoFactory.criar("misclassified_train", "integer", "acurácia"),
          retornoFactory.criar("misclassified_validation", "integer", "acurácia"),
          retornoFactory.criar("misclassified_test", "integer", "acurácia")
        ]
      },
      {
        nome: 'create_nn',
        descricao:'Função cria um RNA baseada na neuralnet',
        cabecalho:'create_nn(table_data text, hidden integer[], functionActivation integer DEFAULT 3, steepness float default 0.01, validation_fold integer default 1, test_fold integer default 1, max_epochs integer default 500, epochs_between_report integer default 10, OUT nn neuralnet, OUT mse_report double precision[]) RETURNS record',
        parametros: [
          parametroFactory.criar('table_data', 'text','Deve ser uma tabela preparada por prepare_data_to_learn, tendo as colunas id, fold, entrada, saida, com entrada e saida normalizadas.'),
          parametroFactory.criar('hidden', 'integer[]','array com o número de neurônios das camadas intermediárias(hdden layers)'),
          parametroFactory.criar('functionActivation', 'integer', 'Função de ativação. Veja todos os valores em Função de Ativação: default: 3 (NN_SIGMOID)'),
          parametroFactory.criar('steepness', 'float', 'valor do steepness. default: 0.01' ),
          parametroFactory.criar('validation_fold', 'integer', 'fold de validação. default: 1' ),
          parametroFactory.criar('test_fold', 'integer', 'fold de teste. default: 1' ),
          parametroFactory.criar('max_epochs', 'integer', 'número máximo de épocas. default: 500' ),
          parametroFactory.criar('epochs_between_report', 'integer', 'número de epocas entre relatórios. default: 10'),
        ],
        retorno:[
          retornoFactory.criar('nn', 'neuralnet', 'RNA'),
          retornoFactory.criar('mse_report', 'double precision[]', 'mínimo erro quadrado')
        ]
      },
      {
        nome: 'diag',
        descricao:'Cria e retorna uma matrix diagonal [a] do tamanho de v.',
        cabecalho:[
          'diag(in v vector, out c matrix)',
          'diag(size integer, a double precision DEFAULT 1.0, OUT c matrix)'
        ],
        parametros: [
          parametroFactory.criar('v', 'vector', 'vetor que será transpassado para diagonal'),
          parametroFactory.criar('size', 'integer', 'dimensão da matriz'),
          parametroFactory.criar('a', 'double precision', 'valores que serão inseridos na diagonal default 1.0')
        ],
        retorno:[
          retornoFactory.criar('c', 'matrix', 'matriz diagonal')
        ]
      },
      {
        nome: 'dims',
        descricao:'Consegue as dimensões da matriz',
        cabecalho:'dims(m matrix, OUT lines integer, OUT cols integer) RETURNS record',
        parametros: [
          parametroFactory.criar('m', 'matrix', 'Matriz de valores')
        ],
        retorno:[
          retornoFactory.criar('lines', 'integer', 'Número de linhas da matriz'),
          retornoFactory.criar('cols', 'integer', 'Número de colunas da matriz')
        ]
      },
      {
        nome: 'distancia',
        descricao:'Distância de duas matrizes A e B',
        cabecalho:'distancia(a matrix, b matrix, OUT valor double precision) RETURNS double precision',
        parametros: [
          parametroFactory.criar('A', 'matrix', 'Matriz de valores'),
          parametroFactory.criar('B', 'matrix', 'Matriz de valores')
        ],
        retorno:[
          retornoFactory.criar('distancia', 'double precision', 'distancia de duas matrizes')
        ]
      },
      {
        nome: 'dup',
        descricao:'Duplica uma matriz',
        cabecalho:'dup(m matrix, OUT c matrix) RETURNS matrix',
        parametros: [
          parametroFactory.criar('m', 'matrix', 'Matriz de valores')
        ],
        retorno:[
          retornoFactory.criar('c', 'matrix', 'Matriz duplicada')
        ]
      },
      {
        nome: 'fann',
        descricao:'Converte para o tipo fann',
        cabecalho:'fann(ann bigint, OUT c fann) RETURNS fann',
        parametros: [
          parametroFactory.criar('ann', 'bigint', 'ponteiro para ann')
        ],
        retorno:[
          retornoFactory.criar('ann', 'fann', 'Estrutura fann')
        ]
      },
      {
        nome: 'fann_create_train_data',
        descricao:'Criar estrutura de dados para treinamento',
        cabecalho:'fann_create_train_data(entrada matrix, saida matrix, OUT r traindata) RETURNS traindata',
        parametros: [
          parametroFactory.criar('entrada', 'matrix', 'matrix normalizada com os valores de entrada do treinamento'),
          parametroFactory.criar('saída', 'matrix', 'matrix normalizada com os valores de saída. classificados')
        ],
        retorno:[
          retornoFactory.criar('data', 'traindata', 'estrutura preparada para treinamento')
        ]
      },
      {
        nome: 'free',
        descricao:'Desaloca uma matriz',
        cabecalho:'free(m matrix) RETURNS void',
        parametros: [
          parametroFactory.criar('m', 'matrix', 'Matriz de valores')
        ],
        retorno:[]
      },
      {
        nome: 'get_col',
        descricao:'Retorna a coluna de uma matriz',
        cabecalho:'get_col(m matrix, col integer) RETURNS double precision[]',
        parametros: [
          parametroFactory.criar('m', 'matrix', 'Matriz de valores'),
          parametroFactory.criar('col', 'integer', 'índice da coluna'),
        ],
        retorno:[
          retornoFactory.criar('coluna', 'double precision[]', 'array com os valores da coluna')
        ]
      },
      {
        nome: 'get_line',
        descricao:'Retorna a linha de uma matriz',
        cabecalho:'get_line(m matrix, line integer) RETURNS double precision[]',
        parametros: [
          parametroFactory.criar('m', 'matrix', 'Matriz de valores'),
          parametroFactory.criar('line', 'integer', 'índice da linha')
        ],
        retorno:[
          retornoFactory.criar('line', 'double precision[]', 'array com os valores da linha')
        ]
      },
      {
        nome: 'inv',
        descricao:'Retorna a matriz inversa',
        cabecalho:'inv(m matrix, OUT c matrix) RETURNS matrix',
        parametros: [
          parametroFactory.criar('m', 'matrix', 'Matriz de valores')
        ],
        retorno:[
          retornoFactory.criar('c', 'matrix', 'matriz inversa de m')
        ]
      },
      {
        nome: 'inverse_sigmoid',
        descricao:'Retorna o valor da sigmoid inversa. Inversa da sigmoid: ln( x/(1.0 - x) )/2.0',
        cabecalho:[
          'inverse_sigmoid(a double precision[]) RETURNS double precision[]',
          'inverse_sigmoid(e double precision) RETURNS double precision'
        ],
        parametros: [
          parametroFactory.criar('a', 'double precision[]', 'array de valores que serão aplicados a x na função '),
          parametroFactory.criar('x', 'double precision', 'valor a ser aplicado na função')
        ],
        retorno:[
          retornoFactory.criar('inv', 'double precision[]', 'valores da inversa da sigmoid'),
          retornoFactory.criar('inv', 'double precision', 'valor da inversa da sigmoid')
        ]
      },
      {
        nome: 'make_dense_matrix',
        descricao:"Cria uma matiz densa a partir de um SQL de uma matriz esparsa",
        cabecalho:"make_dense_matrix(sql text, zero text DEFAULT ''::text, centrar boolean DEFAULT true) RETURNS bigint",
        parametros: [
          parametroFactory.criar('sql', 'text', "SQL de uma matriz esparsa do tipo 'select linha, coluna, valor from ..."),
          parametroFactory.criar('zero', 'text', "significa que zero é zero; não haverá substituição; 'mavg' significa substituir zero pela média dos valores da matriz inteira; 'lavg' significa substituir os zeros de cada linha pela média de cada linha; 'cavg' significa substituir os zeros de cada coluna pela média de cada coluna"),
          parametroFactory.criar('centrar', 'bool', "centrar a média em zero, o que equivale a subtrair de cada valor diferente de zero a média escolhida."),
        ],
        retorno:[
          retornoFactory.criar('matrix', 'bigint', "ponteiro para a matriz densa")
        ]
      },
      {
        nome: 'make_matrix',
        descricao:'Cria uma matriz a partir de um sql',
        cabecalho:'make_matrix(sql text, OUT matrix bigint, OUT keys bigint) RETURNS record',
        parametros: [
          parametroFactory.criar('sql', 'text', "'sql' deve ser uma string retornando (id,valor), onde 'id integer primary key' e valor é um array float8[] com os valores do ponto, ou uma tabela/view com as colunas id::integer, valor::float8[] EX1: 'select id, array[ x, y ] valor from tabela' EX2: 'select cast( row_number() over() as integer ) id, array[ x, y ] valor from tabela'")
        ],
        retorno:[
          retornoFactory.criar('matrix', 'bigint', "ponteiro para matrix"),
          retornoFactory.criar('keys', 'bigint', "ponteiro para vetor contendo a chave (id) de cada linha da matriz.")
        ]
      },
      {
        nome: 'make_random_matrix',
        descricao:'Criar uma matriz com valores randômicos',
        cabecalho:'make_random_matrix(n_lines integer, n_cols integer, a double precision DEFAULT 0.0, b double precision DEFAULT 1.0) RETURNS bigint',
        parametros: [
          parametroFactory.criar('n_lines', 'integer', 'número de linhas'),
          parametroFactory.criar('n_cols', 'integer', 'número de colunas'),
          parametroFactory.criar('a', 'double precision', 'valores de'),
          parametroFactory.criar('b', 'double precision', 'valores até'),
        ],
        retorno:[
          retornoFactory.criar('matrix', 'bigint', 'ponteiro para matriz')
        ]
      },
      {
        nome: 'matrix',
        descricao:'Criar uma matrix',
        cabecalho:[
          'matrix(ptr_matrix bigint, OUT c matrix) RETURNS matrix',
          'matrix(bidimensional_array double precision[], OUT c matrix) RETURNS matrix',
          'matrix(sql text, OUT c matrix) RETURNS matrix'
        ],
        parametros: [
          parametroFactory.criar('ptr_matrix', 'bigint', 'ponteiro para a matrix'),
          parametroFactory.criar('bidimensional_array','double precision[]', 'array de valores de duas dimensões'),
          parametroFactory.criar('sql','text', 'sql similar ao make_matrix')
        ],
        retorno:[
          retornoFactory.criar('c', 'matrix', 'matrix estruturada')
        ]
      },
      {
        nome: 'matrix2table',
        descricao:'Converte matriz em tabela',
        cabecalho:[
          'matrix2table(m matrix, table_name text, OUT nlines integer, OUT ncols integer) RETURNS record',
          'matrix2table(ptr_matrix bigint, table_name text, OUT nlines integer, OUT ncols integer) RETURNS record',
          'matrix2table(keys bigint, matrix bigint, table_name text, OUT nlines integer, OUT ncols integer) RETURNS record'
        ],
        parametros: [
          parametroFactory.criar('matrix', 'matrix', 'Matriz de valores'),
          parametroFactory.criar('ptr_matrix', 'bigint', 'ponteiro para matriz de valores'),
          parametroFactory.criar('keys', 'bigint', 'ponteiro para vetor com as chaves da nova tabela'),
          parametroFactory.criar('table_name', 'text', 'nome da tabela'),
        ],
        retorno:[
          retornoFactory.criar('nlines', 'integer', 'número de linhas da tabela'),
          retornoFactory.criar('ncols', 'integer', 'número de colunas da tabela'),
        ]
      },
      {
        nome: 'matrix2table_transpose',
        descricao:'Converte matriz em tabela, com a transposta',
        cabecalho:[
          'matrix2table_transpose(matrix bigint, table_name text, OUT nlines integer, OUT ncols integer) RETURNS record',
          'matrix2table_transpose(keys bigint, matrix bigint, table_name text, OUT nlines integer, OUT ncols integer) RETURNS record'
        ],
        parametros: [
          parametroFactory.criar('matrix', 'bigint', 'ponteiro para matriz de valores'),
          parametroFactory.criar('keys', 'bigint', 'ponteiro para vetor com as chaves da nova tabela'),
          parametroFactory.criar('table_name', 'text', 'nome da tabela'),
        ],
        retorno:[
          retornoFactory.criar('nlines', 'integer', 'número de linhas da tabela'),
          retornoFactory.criar('ncols', 'integer', 'número de colunas da tabela'),
        ]
      },
      {
        nome: 'max',
        descricao:'Retorna o maior valor',
        cabecalho:'max(a double precision, b double precision) RETURNS double precision',
        parametros: [
          parametroFactory.criar('a', 'double precision', 'valor a ser comparado'),
          parametroFactory.criar('b', 'double precision', 'valor a ser comparado'),
        ],
        retorno:[
          retornoFactory.criar('max', 'double precision', 'max = a > b ? a : b')
        ]
      },
      {
        nome: 'max_elem',
        descricao:'Retorna o maior valor presente no array',
        cabecalho:'max_elem(a double precision[]) RETURNS double precision',
        parametros: [
          parametroFactory.criar('a', 'double precision[]', 'array de valores')
        ],
        retorno:[
          retornoFactory.criar('max', 'double precision', 'elemento de maior valor no array')
        ]
      },
      {
        nome: 'min',
        descricao:'Retorna o menor valor',
        cabecalho:'min(a double precision, b double precision) RETURNS double precision',
        parametros: [
          parametroFactory.criar('a', 'double precision', 'valor a ser comparado'),
          parametroFactory.criar('b', 'double precision', 'valor a ser comparado'),
        ],
        retorno:[
          retornoFactory.criar('min', 'double precision', 'max = a < b ? a : b')
        ]
      },
      {
        nome: 'ncols',
        descricao:'Retorna o número de colunas de uma matrix',
        cabecalho:'ncols(m matrix, OUT cols integer) RETURNS integer',
        parametros: [
          parametroFactory.criar('m', 'matrix', 'Matriz de valores')
        ],
        retorno:[
          retornoFactory.criar('cols', 'integer', 'Número de colunas da matriz m')
        ]
      },
      {
        nome: 'nlines',
        descricao:'Retorna o número de linhas de uma matrix',
        cabecalho:'nlines(m matrix, OUT nlines integer) RETURNS integer',
        parametros: [
          parametroFactory.criar('m', 'matrix', 'Matriz de valores')
        ],
        retorno:[
          retornoFactory.criar('nlines', 'integer', 'Número de linhas da matriz m')
        ]
      },
      {
        nome: 'norm',
        descricao:'Retorna a norma do array a',
        cabecalho:['norm(a double precision[]) RETURNS double precision','norm(m matrix) RETURNS double precision'],
        parametros: [
          parametroFactory.criar('a', 'double precision[]', 'array de valores'),
          parametroFactory.criar('m', 'matrix', 'matriz de valores')
        ],
        retorno:[
          retornoFactory.criar('norm', 'double precision', 'norma do array')
        ]
      },
      
      {
        nome: 'pgm_hello_world',
        descricao:'Função de Hello World do Pgminer',
        cabecalho:'FUNCTION pgm_hello_world(entrada CSTRING) RETURNS CSTRING',
        parametros: [
          parametroFactory.criar('entrada', 'CSTRING', 'uma palavra qualquer')
        ],
        retorno:[
          retornoFactory.criar('saida', 'CSTRING', 'a mesma palavra da entrada')
        ]
      },
      {
        nome: 'pgm_kmeans',
        descricao:'Executa o algoritmo de clusterização k-means',
        cabecalho:'pgm_kmeans(pontos bigint, k integer, seed integer, fim integer, OUT centroide bigint, OUT grupo bigint, OUT sumd double precision) RETURNS record',
        parametros: [
          parametroFactory.criar('pontos', 'bigint', 'matriz de pontos'),
          parametroFactory.criar('k', 'integer', 'número de grupos desejados'),
          parametroFactory.criar('seed', 'integer', '1 - para sortear k pontos para serem os centros dos grupos; 2 - para sortear cada ponto em um grupo'),
          parametroFactory.criar('fim', 'integer', '1 - se um Grupo Secar , Elimine - o; 2 - se um Grupo Secar , cria um grupo com o elemento mais longe de seu centroide; 3 - se um Grupo Secar , Erro .'),
        ],
        retorno:[
          retornoFactory.criar('centroide', 'bigint', 'ponteiro para matriz com os centroídes'),
          retornoFactory.criar('grupos', 'bigint', 'O grupo é um ponteiro para vetor de n posições, onde n é o número total de pontos p k , que armazena na posição i, onde i é o índice na matriz correspondente ao ponto p i , o grupo que o ponto p i está alocado. Logo, gi = grupo(pi)'),
          retornoFactory.criar('sumd', 'double precision', 'soma quadrática mínima'),
        ]
      },
      {
        nome: 'pgm_cos',
        descricao:'Aplica função cosseno em duas matrizes',
        cabecalho:'pgm_cos(matrix_a bigint, matrix_b bigint) RETURNS bigint',
        parametros: [
          parametroFactory.criar('a', 'bigint', 'Matriz de valores'),
          parametroFactory.criar('b', 'bigint', 'Matriz de valores'),
        ],
        retorno:[
          retornoFactory.criar('cosseno', 'bigint', 'ponteiro para vetor')
        ]
      },

      {
        nome: 'neuralnet',
        descricao:'Módulo de operação da RNA (rede neural artificial) que é possivel serializar e inserir em tabelas. Serialização da neuralnet: #NumeroDeCamadas#NeuroniosPorCamada#FunçãoDeAtivação#InputMin InputMax OutputMin OutputMax#Stepness#Bias#Pesos',
        tipo: {
          nome: 'neuralnet',
          lang: 'C',
          campos: [
            tipoFactory.criar('size', 'int', 'tamanho de byte da struct'),
            tipoFactory.criar('NLayers', 'unsigned short', 'numero de camadas'),
            tipoFactory.criar('FunctionActivation', 'unsigned short', 'codigo da função de ativação'),
            tipoFactory.criar('NNeurons[MAX_LAYERS]', 'unsigned int', 'numero de neuronios por camada. MAX_LAYERS = 5'),
            tipoFactory.criar('TotalNeurons', 'unsigned int', 'numero total de neurônios presentes na ANN'),
            tipoFactory.criar('TotalWeights', 'unsigned int', 'Número total de pesos da rede neural'),
            tipoFactory.criar('Bias', 'unsigned int', 'Utilizado na conversão para FANN'),
            tipoFactory.criar('Steepness', 'double', 'Valor do Steepness. Parâmetro de compatibilidade com Fann'),
            tipoFactory.criar('MSE', 'double', 'Valor do Erro mínimo quadrado. Parâmetro de compatibilidade com Fann'),
            tipoFactory.criar('BihiperbolicLambda', 'double', 'Parâmetros da função de ativação bi-hiperbolica'),
            tipoFactory.criar('BihiperbolicT1', 'double', 'Parâmetros da função de ativação bi-hiperbolica'),
            tipoFactory.criar('BihiperbolicT2', 'double', 'Parâmetros da função de ativação bi-hiperbolica'),
            tipoFactory.criar('InputMin', 'double', 'Limite de input mínimo. Parâmetros de escala. Default = 0'),
            tipoFactory.criar('InputMax', 'double', 'Limite de inpute máximo. Parâmetros de escala. Default = 1.0'),
            tipoFactory.criar('OutputMin', 'double', 'Limite de output mínimo. Parâmetros de escala. Default = 0'),
            tipoFactory.criar('OutputMax', 'double', 'Limite de output máximo. Parâmetros de escala. Default = 1.0'),
            tipoFactory.criar('WeightsPos[MAX_LAYERS - 1]', 'unsigned int', 'Vetor com o indice que inicia a sequencia de pesos da camada no vetor Weights'),
            tipoFactory.criar('Weights[0]','double', 'Vetor que guarda os valores dos neuronios e os pesos. Para acessar os pesos basta da um GetWeght(layer). OBS: Pesos: primeira camada: Bias[layer][i] - Neuron[layer+1][i]; Neuron[layer][j] - Neuron[layer+1][i]; Neuron[layer][j+1] - Neuron[layer+1][i]; assim sucessivamente. Lembrando que a ultima camada nao tem Bias')
          ]
        },
        operador: [
          operadorFactory.criar('/','integer','opr_nn_split_part - Ex: neuralnet / integer'),
          operadorFactory.criar('*','double precision[]','pgm_nn_evaluate - Ex: neuralnet * double precision[] ou double precision[] * neuralnet')
        ],
        funcoes: [
          funcaoFactory.criar('pgm_neuralnetin', 'pgm_neuralnetin(st cstring) RETURNS neuralnet','De-Serialização de neuralnet', [
            parametroFactory.criar('st','cstring','String de neuralnet serializado utilizando pgm_neuralnetout'),
          ],[
            retornoFactory.criar('nn','neuralnet','RNA de-serializada'),
          ]),
          funcaoFactory.criar('pgm_neuralnetout', 'pgm_neuralnetout(neuralnet) RETURNS cstring','Serialização de neuralnet', [
            parametroFactory.criar('nn','neuralnet','neuralnet para a ser serializada'),
          ],[
            retornoFactory.criar('st','cstring','RNA serializada'),
          ]),
          funcaoFactory.criar('pgm_nn_equal', 'pgm_nn_equal(nn1 neuralnet, nn2 neuralnet) RETURNS boolean','Compara se duas neuralnets são iguais', [
            parametroFactory.criar('nn1','neuralnet','RNA a ser comparada'),
            parametroFactory.criar('nn2','neuralnet','RNA a ser comparada'),
          ],[
            retornoFactory.criar('eq','boolean','True se iguais, caso contrário, False'),
          ]),
          funcaoFactory.criar('pgm_nn_evaluate', 'pgm_nn_evaluate(nn neuralnet, vector double precision[]) RETURNS double precision[]','Função que valida a rede para uma determinada entrada.', [
            parametroFactory.criar('nn1','neuralnet','RNA'),
            parametroFactory.criar('vector','double precision[]','registro a ser classificado'),
          ],[
            retornoFactory.criar('classificacao','double precision[]','Vetor com a probabiblidade de classificacao'),
          ]),
          funcaoFactory.criar('pgm_nn_get_distance', 'pgm_nn_get_distance(nna neuralnet, nnb neuralnet) RETURNS double precision','Obtem a distância entre as redes', [
            parametroFactory.criar('nna','neuralnet','RNA'),
            parametroFactory.criar('nnb','neuralnet','RNA'),
          ],[
            retornoFactory.criar('distancia','double precision','Distância entre as RNAs'),
          ]),
          funcaoFactory.criar('pgm_nn_get_mse', 'pgm_nn_get_mse(nn neuralnet) RETURNS double precision','Obtem o erro mínimo quadrado', [
            parametroFactory.criar('nn','neuralnet','RNA'),
          ],[
            retornoFactory.criar('mse','double precision','Erro mínimo quadrado'),
          ]),
          funcaoFactory.criar('pgm_nn_get_scaling', 'pgm_nn_get_scaling(nn neuralnet, OUT input_min double precision, OUT input_max double precision, OUT output_min double precision, OUT output_max double precision) ','Obtem as escalas da RNA',  [
            parametroFactory.criar('nn1','neuralnet','RNA'),
          ],[
            retornoFactory.criar('input_min','double precision','Limite mínimo de input'),
            retornoFactory.criar('input_max','double precision','Limite máximo de input'),
            retornoFactory.criar('output_min','double precision','Limite mínimo de output'),
            retornoFactory.criar('output_max','double precision','Limite máximo de output'),
          ]),
          funcaoFactory.criar('pgm_nn_get_weight_array', 'pgm_nn_get_weight_array(nn neuralnet, layer integer) RETURNS bigint','Obtem os pesos de uma camada da RNA',  [
            parametroFactory.criar('nn','neuralnet','RNA'),
            parametroFactory.criar('layer','integer','camada desejada'),
          ],[
            retornoFactory.criar('ptr','bigint','ponteiro para pgm_vector_double com os pesos da camada'),
          ]),
          funcaoFactory.criar('pgm_nn_inputsize', 'pgm_nn_inputsize(nn neuralnet) RETURNS integer','Obtem o número de neurônio da primeira camada',  [
            parametroFactory.criar('nn','neuralnet','RNA'),
          ],[
            retornoFactory.criar('inputsize','integer','Número de neurônios'),
          ]),
          funcaoFactory.criar('pgm_nn_merge', 'pgm_nn_merge(nna neuralnet, nnb neuralnet) RETURNS neuralnet','Mescla duas RNAs. Condição: nna->OutputMin == 0.0; nna->OutputMax == 1.0; nnb->InputMin == 0.0; nnb->InputMax == 1.0; nna->NLayers + nnb->NLayers - 1 >= 5; nna->NNeurons[nna->NLayers-1] == nnb->NNeurons[0]; nna->FunctionActivation == nnb->FunctionActivation; nna->Bias == nnb->Bias; nna->Steepness == nnb->Steepness', [
            parametroFactory.criar('nna','neuralnet','RNA'),
            parametroFactory.criar('nnb','neuralnet','RNA'),
          ],[
            retornoFactory.criar('nn','neuralnet','RNAs mescladas'),
          ]),
          funcaoFactory.criar('pgm_nn_neuralnet2fann', 'pgm_nn_neuralnet2fann(nn neuralnet) RETURNS bigint','Converte RNA do tipo neuralnet em RNA do tipo fann',  [
            parametroFactory.criar('nn','neuralnet','RNA'),
          ],[
            retornoFactory.criar('fann_ptr','bigint','ponteiro para RNA fann'),
          ]),
          funcaoFactory.criar('pgm_nn_nlayers', 'pgm_nn_nlayers(neuralnet) RETURNS integer[]','Número de neurônios por camada',  [
            parametroFactory.criar('nn','neuralnet','RNA'),
          ],[
            retornoFactory.criar('nlayers','integer[]','Número de neurônios por camada'),
          ]),
          funcaoFactory.criar('pgm_nn_outputsize', 'pgm_nn_outputsize(nn neuralnet) RETURNS integer','Obtem o número de neurônios da última camada', [
            parametroFactory.criar('nn','neuralnet','RNA'),
          ],[
            retornoFactory.criar('outputsize','integer','Número de neurônios'),
          ]),
          funcaoFactory.criar('pgm_nn_set_mse', 'pgm_nn_set_mse(nn neuralnet, new_mse double precision) RETURNS double precision','Troca o MSE(erro mínimo quadrado) de um RNA', [
            parametroFactory.criar('nn','neuralnet','RNA'),
            parametroFactory.criar('new_mse','double precision','Novo valor para mse'),
          ],[
            retornoFactory.criar('mse','double precision','Novo MSE'),
          ]),
          funcaoFactory.criar('pgm_nn_set_scaling', 'pgm_nn_set_scaling(nn neuralnet, input_min double precision, input_max double precision, output_min double precision, output_max double precision) RETURNS neuralnet','descricao', [
            parametroFactory.criar('nn','neuralnet','RNA'),
            parametroFactory.criar('input_min','double precision','Novo limite mínimo de input'),
            parametroFactory.criar('input_max','double precision','Novo limite máximo de input'),
            parametroFactory.criar('output_min','double precision','Novo limite mínimo de output'),
            parametroFactory.criar('output_max','double precision','Novo limite máximo de output'),
          ],[
            retornoFactory.criar('nn','neuralnet','RNA com as escalas trocadas'),
          ]),
          funcaoFactory.criar('pgm_nn_set_weight_array', 'pgm_nn_set_weight_array(nn neuralnet, matrix bigint, layer integer) RETURNS neuralnet','Troca os pesos de uma camada', [
            parametroFactory.criar('nn','neuralnet','RNA'),
            parametroFactory.criar('matrix','bigint','Ponteiro para pgm_matrix_double com os novos pesos para camada'),
            parametroFactory.criar('layer','integer','Camada a ser trocada'),
          ],[
            retornoFactory.criar('nn','neuralnet','RNA com as escalas trocadas'),
          ]),
          funcaoFactory.criar('pgm_nn_split', 'pgm_nn_split(nn neuralnet, split integer, OUT nn1 neuralnet, OUT nn2 neuralnet) RETURNS record','Função que quebra a rede em 2 partes', [
            parametroFactory.criar('nn','neuralnet','RNA'),
            parametroFactory.criar('layer','integer','camada onde será efetuada a cisão'),
          ],[
            retornoFactory.criar('nn1','neuralnet','RNA correspondente à primeira parte da RNA anterior'),
            retornoFactory.criar('nn2','neuralnet','RNA correspondente à segunda parte da RNA anterior'),
          ]),
          funcaoFactory.criar('pgm_nn_test', 'pgm_nn_test(matrix_in bigint, matrix_out bigint, nn neuralnet) RETURNS double precision','Efetua teste na RNA com um dado conjunto de entrada',[
            parametroFactory.criar('matrix_in','bigint','ponteiro para pgm_matrix_double com o conjunto de entrada'),
            parametroFactory.criar('matrix_out','bigint','ponteiro para pgm_matrix_double com o conjunto de saída'),
            parametroFactory.criar('nn','neuralnet','RNA'),
          ],[
            retornoFactory.criar('accurate','double precision','Acurácia da RNA'),
          ]),
        ]
      },

      {
        nome: 'fann',
        descricao:'Estrutura para ponteiro da estrutura fann',
        tipo: {
          nome: 'fann',
          lang: 'pgPL/SQL',
          campos: [
            tipoFactory.criar('ann', 'bigint', 'ponteiro para struct da fann'),
          ]
        },
        operador: [
          operadorFactory.criar('*','double precision[]','opr_fann_apply - Ex: double precision[] / fann'),
        ],
        funcoes: [
          funcaoFactory.criar('pgm_nn_fann_get_mse_report','pgm_nn_fann_get_mse_report(fann bigint) RETURNS double precision','Obtem o erro mínimo quadrado',[
            parametroFactory.criar('fann','bigint','ponteiro para estrutura fann'),
          ],[
            retornoFactory.criar('mse','double precision[]','Erro mínimo quadrado'),
          ]),
          funcaoFactory.criar('pgm_nn_fann2neuralnet','pgm_nn_fann2neuralnet(fann bigint) RETURNS neuralnet','Converte ponteiro fann para neuralnet',[
            parametroFactory.criar('fann','bigint','Ponteiro para estrutura fann'),
          ],[
            retornoFactory.criar('nn','neuralnet','RNA convertida'),
          ]),
          funcaoFactory.criar('pgm_nn_fann_create_train_data','pgm_nn_fann_create_train_data(data_matrix_input bigint, data_matrix_ouput bigint) RETURNS bigint','Cria estrutura de dados para treinamento. Recomendado usar prepare_data_to_learn',[
            parametroFactory.criar('data_matrix_input','bigint','ponteiro para pgm_matrix_double com dados de entrada'),
            parametroFactory.criar('data_matrix_output','bigint','ponteiro para pgm_matrix_double com dados de saída'),
          ],[
            retornoFactory.criar('train_data','bigint','RNA de-serializada'),
          ]),
          funcaoFactory.criar('pgm_nn_fann_free_train_data','pgm_nn_fann_free_train_data(train_data bigint) RETURNS void','Desaloca train_data',[
            parametroFactory.criar('train_data','bigint','ponteiro para train_data'),
          ],[]),
          funcaoFactory.criar('pgm_nn_fann_run','pgm_nn_fann_run(ann bigint, _input double precision[]) RETURNS double precision[]','Função que valida a rede para uma determinada entrada.',[
            parametroFactory.criar('ann','bigint','ponteiro para estrutura da fann'),
            parametroFactory.criar('_input','double precision[]','registro de entrada a ser classificado'),
          ],[
            retornoFactory.criar('classificacao','double precision[]','Vetor com a probabiblidade de classificacao'),
          ]),
          funcaoFactory.criar('pgm_nn_fann_test','pgm_nn_fann_test(train_data bigint, ann bigint, OUT mse double precision) RETURNS double precision','Efetua teste na RNA com um dado conjunto de entrada',[
            parametroFactory.criar('train_data','bigint','Ponteiro para estrutura train_data'),
            parametroFactory.criar('ann','bigint','Ponteiro para estrutura fann'),
          ],[
            retornoFactory.criar('mse','double precision','Erro mínino quadrado'),
          ]),
          funcaoFactory.criar('pgm_nn_fann_train','pgm_nn_fann_train(train_data bigint, hidden_layer integer[], functionActivation integer, steepness double precision, max_epochs integer, epochs_between_reports integer, desired_error double precision) RETURNS bigint','Executa treinamento de uma RNA',[
            parametroFactory.criar('train_data','bigint','Ponteiro para estrutura train_data'),
            parametroFactory.criar('hidden_layer', 'integer[]', 'Número de neurônios nas camadas intermediária.'),
            parametroFactory.criar('functionActivation', 'integer', 'Valor da função de Ativação'),
            parametroFactory.criar('steepness', 'double precision', 'Valor de steepness'),
            parametroFactory.criar('max_epochs', 'integer', 'Número máximo de épocas para convergir'),
            parametroFactory.criar('epochs_between_reports', 'integer', 'Número de epocas entre relatórios.'),
            parametroFactory.criar('desired_error', 'double precision', 'Erro desejado'),
          ],[
            retornoFactory.criar('nn','bigint','Ponteiro para RNA criada. No formato de neuralnet'),
          ])
        ]
      },

      {
        nome: 'matrix',
        descricao:'Estrura representa matrix de valores double (pgm_matrix_double)',
        tipo: {
          nome: 'matrix',
          lang: 'pgPL/SQL',
          campos: [
            tipoFactory.criar('ptr', 'bigint', 'ponteiro para struct pgm_matrix_double'),
          ]
        },
        operador: [
          operadorFactory.criar('*', 'matrix', 'opr_matrix_multiply'),
          operadorFactory.criar('*', 'vector', 'opr_vector_matrix_multiply'),
          operadorFactory.criar('*', 'double precision[]', 'opr_array_matrix_multiply'),
          operadorFactory.criar('*', 'double precision', 'opr_matrix_double_multiply'),
          operadorFactory.criar('+', 'matrix', 'opr_matrix_add'),
          operadorFactory.criar('-', 'matrix', 'opr_matrix_subtract'),
          operadorFactory.criar('==>', 'text', 'matrix2table'),
        ],
        funcoes: [
          funcaoFactory.criar('opr_array_matrix_multiply', 'opr_array_matrix_multiply(a double precision[], b matrix, OUT c double precision[]) RETURNS double precision[]','Efetua multiplicação de matrix',[
            parametroFactory.criar('a','double precision[]','array bidimencional'),
            parametroFactory.criar('b','double precision[]','array bidimencional'),
          ],[
            retornoFactory.criar('c','double precision[]','array bidimencional'),
          ]),
          funcaoFactory.criar('opr_matrix_add', 'opr_matrix_add(a matrix, b matrix, OUT c matrix) RETURNS matrix','Soma matricial',[
            parametroFactory.criar('a','matrix','matriz de valores A'),
            parametroFactory.criar('b','matrix','matriz de valores B'),
          ],[
            retornoFactory.criar('C','matrix','matriz resultante'),
          ]),
          funcaoFactory.criar('opr_matrix_double_multiply', 'opr_matrix_double_multiply(a matrix, b double precision, OUT c matrix) RETURNS matrix','Multiplicação de matriz',[
            parametroFactory.criar('a','matrix','matriz de valores A'),
            parametroFactory.criar('b','matrix','matriz de valores B'),
          ],[
            retornoFactory.criar('C','matrix','matriz resultante'),
          ]),
          funcaoFactory.criar('opr_matrix_multiply', 'opr_matrix_multiply(a matrix, b matrix, OUT c matrix) RETURNS matrix','Multiplicação de matriz',[
            parametroFactory.criar('a','matrix','matriz de valores A'),
            parametroFactory.criar('b','matrix','matriz de valores B'),
          ],[
            retornoFactory.criar('C','matrix','matriz resultante'),
          ]),
          funcaoFactory.criar('opr_matrix_subtract', 'opr_matrix_subtract(a matrix, b matrix, OUT c matrix) RETURNS matrix','Subtração de matriz',[
            parametroFactory.criar('a','matrix','matriz de valores A'),
            parametroFactory.criar('b','matrix','matriz de valores B'),
          ],[
            retornoFactory.criar('C','matrix','matriz resultante'),
          ]),
          funcaoFactory.criar('pgm_array2matrix','pgm_array2matrix(bidimensional_array double precision[]) RETURNS bigint','Converte array bidimencional em ponteiro para pgm_matrix_double',[
            parametroFactory.criar('bidimensional_array','double precision[]','array bidimencional'),
          ],[
            retornoFactory.criar('matrix','bigint','ponteiro para pgm_matrix_double'),
          ]),
          funcaoFactory.criar('pgm_array2matrix_int','pgm_array2matrix_int(value integer[])','Converte array bidimencional em ponteiro para pgm_matrix_int',[
            parametroFactory.criar('value','integer[]','array bidimencional'),
          ],[
            retornoFactory.criar('matrix','bigint','ponteiro para pgm_matrix_int'),
          ]),
          funcaoFactory.criar('pgm_array2matrix_float','pgm_array2matrix_float(value real[]) RETURNS bigint','Converte array bidimencional para ponteiro para pgm_matrix_double',[
            parametroFactory.criar('value','real[]','array bidimencional'),
          ],[
            retornoFactory.criar('matrix','bigint','ponteiro para pgm_matrix_double'),
          ]),
          funcaoFactory.criar('pgm_matrix2array','pgm_matrix2array(matrix bigint) RETURNS double precision[]','Converte ponteiro para pgm_matrix_double para array bi-dimencional',[
            parametroFactory.criar('matrix','bigint','ponteiro para pgm_matrix_double'),
          ],[
            retornoFactory.criar('array','double precision[]','array bidimencional'),
          ]),
          funcaoFactory.criar('pgm_matrix2array','pgm_matrix2array(a matrix) RETURNS double precision[]','Converte matrix em array bi-dimencional',[,
            parametroFactory.criar('a','matrix','array bidimencional'),
          ],[
            retornoFactory.criar('c','double precision[]','array bidimencional'),
          ]),
          funcaoFactory.criar('pgm_matrix_add','pgm_matrix_add(matrix_a bigint, matrix_b bigint) RETURNS bigint','Soma de matrizes',[
            parametroFactory.criar('a','bigint','ponteiro para pgm_matrix_double A'),
            parametroFactory.criar('b','bigint','ponteiro para pgm_matrix_double B'),
          ],[
            retornoFactory.criar('c','bigint','ponteiro para pgm_matrix_double'),
          ]),
          funcaoFactory.criar('pgm_matrix_apply_inverse_sigmoid','pgm_matrix_apply_inverse_sigmoid(matrix bigint) RETURNS bigint','Aplica a função inversa da sigmoid. Inversa da sigmoid: -log( 2.0/(x+1.0) -1.0)/16.0',[
            parametroFactory.criar('matrix','bigint','ponteiro para pgm_matrix_double'),
          ],[
            retornoFactory.criar('c','double precision[]','ponteiro para pgm_matrix_double'),
          ]),
          funcaoFactory.criar('pgm_matrix_apply_scale','pgm_matrix_apply_scale(m bigint, scale double precision) RETURNS bigint','Aplica escala na matriz',[
            parametroFactory.criar('matrix','bigint','ponteiro para pgm_matrix_double'),
            parametroFactory.criar('scale','double precision','escala'),
          ],[
            retornoFactory.criar('matrix','bigint','ponteiro de pgm_matrix_double com a escala aplicado'),
          ]),
          funcaoFactory.criar('pgm_matrix_apply_sigmoid','pgm_matrix_apply_sigmoid(matrix bigint) RETURNS bigint','Aplica função sigmoid na matriz',[
            parametroFactory.criar('matrix','bigint','ponteiro para pgm_matrix_double'),
          ],[
            retornoFactory.criar('matrix','bigint','ponteiro para pgm_matrix_double resultante'),
          ]),
          funcaoFactory.criar('pgm_matrix_copy','pgm_matrix_copy(matrix bigint) RETURNS bigint','Copia um matriz',[
            parametroFactory.criar('matrix','bigint','ponteiro para pgm_matrix_double'),
          ],[
            retornoFactory.criar('matrix','bigint','ponteiro para pgm_matrix_double copiado'),
          ]),
          funcaoFactory.criar('pgm_matrix_create','pgm_matrix_create(n_lines integer, n_cols integer) RETURNS bigint','',[
            parametroFactory.criar('n_lines','integer','número de linhas'),
            parametroFactory.criar('n_cols','integer','número de colunas'),
          ],[
            retornoFactory.criar('matrix','bigint','ponteiro para pgm_matrix_double'),
          ]),
          funcaoFactory.criar('pgm_matrix_double_multiply','pgm_matrix_double_multiply(a bigint, b double precision, OUT c bigint) RETURNS bigint','Multiplicação de matrizes. C = AxB',[
            parametroFactory.criar('A','bigint','ponteiro para pgm_matrix_double'),
            parametroFactory.criar('B','bigint','ponteiro para pgm_matrix_double'),
          ],[
            retornoFactory.criar('C','bigint','ponteiro para pgm_matrix_double'),
          ]),
          funcaoFactory.criar('pgm_matrix_float2array','pgm_matrix_float2array(matrix bigint) RETURNS real[]','Converte pgm_matrix_float para array bi-dimencional',[
            parametroFactory.criar('matrix','bigint','ponteiro para pgm_matrix_double'),
          ],[
            retornoFactory.criar('array','double precision[]','array bidimencional'),
          ]),
          funcaoFactory.criar('pgm_matrix_float_create','pgm_matrix_float_create(n_lines integer, n_cols integer) RETURNS bigint','Cria um pgm_matrix_float',[
            parametroFactory.criar('n_lines','integer','número de linhas'),
            parametroFactory.criar('n_cols','integer','número de colunas'),
          ],[
            retornoFactory.criar('matrix','bigint','ponteiro para pgm_matrix_float'),
          ]),
          funcaoFactory.criar('pgm_matrix_float_set_elem','pgm_matrix_float_set_elem(matrix bigint, line integer, col integer, value double precision) RETURNS bigint','Troca o valor de um elemento da matriz',[
            parametroFactory.criar('matrix','bigint','ponteiro para pgm_matrix_float'),
            parametroFactory.criar('line','integer','índice da linha da matrix'),
            parametroFactory.criar('col','integer','índice da coluna da matrix'),
            parametroFactory.criar('value','double precision','valor a ser setado'),
          ],[
            retornoFactory.criar('matrix','bigint','ponteiro para pgm_matrix_float com valor setado'),
          ]),
          funcaoFactory.criar('pgm_matrix_free','pgm_matrix_free(matrix bigint) RETURNS void','Desaloca um pgm_matrix_double',[
            parametroFactory.criar('a','bigint','ponteiro para pgm_matrix_double'),
          ],[]),
          funcaoFactory.criar('pgm_matrix_get','pgm_matrix_get(matrix bigint) RETURNS double precision[]','Converte pgm_matrix_double para array bi-dimencional',[
            parametroFactory.criar('matrix','bigint','ponteiro para pgm_matrix_double'),
          ],[
            retornoFactory.criar('array','double precision[]','array bidimencional'),
          ]),
          funcaoFactory.criar('pgm_matrix_get_col','pgm_matrix_get_col(matrix bigint, col integer) RETURNS double precision[]','Obtem os valores de uma coluna.',[
            parametroFactory.criar('matrix','bigint','ponteiro para pgm_matrix_double'),
            parametroFactory.criar('col','integer','número da coluna'),
          ],[
            retornoFactory.criar('coluna','double precision[]','array uni-dimencional'),
          ]),
          funcaoFactory.criar('pgm_matrix_get_elem','pgm_matrix_get_elem(matrix bigint, line integer, col integer) RETURNS double precision','Obtem valor de um elemento da matriz',[
            parametroFactory.criar('matrix','bigint','ponteiro para pgm_matrix_double'),
            parametroFactory.criar('line','integer','índice da linha'),
            parametroFactory.criar('col','integer','índice da coluna'),
          ],[
            retornoFactory.criar('elemento','double precision','valor do elemento'),
          ]),
          funcaoFactory.criar('pgm_matrix_get_line','pgm_matrix_get_line(matrix bigint, line integer) RETURNS double precision[]','Obtem os valores da uma linha da matriz',[
            parametroFactory.criar('a','bigint','ponteiro para pgm_matrix_double'),
            parametroFactory.criar('line','integer','índice da linha'),
          ],[
            retornoFactory.criar('c','double precision[]','array bidimencional'),
          ]),
          funcaoFactory.criar('pgm_matrix_get_min_max_mean','pgm_matrix_get_min_max_mean(matrix bigint, OUT min double precision, OUT max double precision, OUT mean double precision) RETURNS record','Retorna o valor máximo, mínimo e a média dos elementos da matriz',[
            parametroFactory.criar('a','bigint','ponteiro para pgm_matrix_double'),
          ],[
            retornoFactory.criar('mínimo','double precision','valor mínimo da matriz'),
            retornoFactory.criar('máximo','double precision','valor máximo da matriz'),
            retornoFactory.criar('média','double precision','valor média da matriz'),
          ]),
          funcaoFactory.criar('pgm_matrix_inverse','pgm_matrix_inverse(matrix bigint) RETURNS bigint','Inverte a matriz',[
            parametroFactory.criar('matrix','bigint','ponteiro para pgm_matrix_double'),
          ],[
            retornoFactory.criar('matrix_inverse','bigint','ponteiro para pgm_matrix_double invertida'),
          ]),
          funcaoFactory.criar('pgm_matrix_multiply','pgm_matrix_multiply(matrix_a bigint, matrix_b bigint) RETURNS bigint','Multiplicação de matriz. C = A x B',[
            parametroFactory.criar('A','bigint','ponteiro para pgm_matrix_double'),
            parametroFactory.criar('B','bigint','ponteiro para pgm_matrix_double'),
          ],[
            retornoFactory.criar('C','bigint','ponteiro para pgm_matrix_double'),
          ]),
          funcaoFactory.criar('pgm_matrix_ncols','pgm_matrix_ncols(matrix bigint) RETURNS integer','Obtem o número de colunas de uma matriz',[
            parametroFactory.criar('matrix','bigint','ponteiro para pgm_matrix_double'),
          ],[
            retornoFactory.criar('n_cols','integer','número de colunas da matriz'),
          ]),
          funcaoFactory.criar('pgm_matrix_nlines','pgm_matrix_nlines(matrix bigint) RETURNS integer','Obtem o número de linhas de uma matriz',[
            parametroFactory.criar('matrix','bigint','ponteiro para pgm_matrix_double'),
          ],[
            retornoFactory.criar('n_lines','integer','número de linhas da matriz'),
          ]),
          funcaoFactory.criar('pgm_matrix_norm','pgm_matrix_norm(matrix bigint) RETURNS double precision','Calcula a norma da matriz',[
            parametroFactory.criar('matrix','bigint','ponteiro para pgm_matrix_double'),
          ],[
            retornoFactory.criar('norm','double precision','norma da matrix'),
          ]),
          funcaoFactory.criar('pgm_matrix_set_col_value','pgm_matrix_set_col_value(m bigint, col integer, value double precision) RETURNS bigint','Seta o valor de todos os elementos da coluna com value',[
            parametroFactory.criar('matrix','bigint','ponteiro para pgm_matrix_double'),
            parametroFactory.criar('col','integer','índice da coluna'),
            parametroFactory.criar('value','double precision','valor a ser setado'),
          ],[
            retornoFactory.criar('matrix','bigint','ponteiro para pgm_matrix_double resultante'),
          ]),
          funcaoFactory.criar('pgm_matrix_set_elem','pgm_matrix_set_elem(matrix bigint, line integer, col integer, value double precision) RETURNS bigint','',[
            parametroFactory.criar('matrix','bigint','ponteiro para pgm_matrix_double'),
            parametroFactory.criar('line','integer','índice da linha'),
            parametroFactory.criar('col','integer','índice da coluna'),
            parametroFactory.criar('value','bigint','valor a ser setado'),
          ],[
            retornoFactory.criar('matrix','bigint','ponteiro para pgm_matrix_double resultante'),
          ]),
          funcaoFactory.criar('pgm_matrix_set_line','pgm_matrix_set_line(matrix bigint, line integer, valor double precision[]) RETURNS bigint','Seta o valor de todos os elementos de uma linha',[
            parametroFactory.criar('matrix','bigint','ponteiro para pgm_matrix_double'),
            parametroFactory.criar('line','integer','índice da linha'),
            parametroFactory.criar('valor','double precision[]','novo valor da linha'),
          ],[
            retornoFactory.criar('matrix','bigint','ponteiro para pgm_matrix_double resultante'),
          ]),
          funcaoFactory.criar('pgm_matrix_set_line_value','pgm_matrix_set_line_value(m bigint, line integer, value double precision) RETURNS bigint','Seta o valor de todos os elementos de uma linha',[
            parametroFactory.criar('matrix','bigint','ponteiro para pgm_matrix_double'),
            parametroFactory.criar('line','integer','índice da linha'),
            parametroFactory.criar('valor','double precision','novo valor da linha'),
          ],[
            retornoFactory.criar('matrix','bigint','ponteiro para pgm_matrix_double resultante'),
          ]),
          funcaoFactory.criar('pgm_matrix_set_value','pgm_matrix_set_value(m bigint, value double precision) RETURNS bigint','Seta todos os valores da matriz',[
            parametroFactory.criar('matrix','bigint','ponteiro para pgm_matrix_double'),
            parametroFactory.criar('valor','double precision','novo valor'),
          ],[
            retornoFactory.criar('matrix','bigint','ponteiro para pgm_matrix_double resultante'),
          ]),
          funcaoFactory.criar('pgm_matrix_subtract','pgm_matrix_subtract(matrix_a bigint, matrix_b bigint) RETURNS bigint','Subtração de matrizes',[
            parametroFactory.criar('matrix_a','bigint','ponteiro para pgm_matrix_double'),
            parametroFactory.criar('matrix_b','bigint','ponteiro para pgm_matrix_double'),
          ],[
            retornoFactory.criar('matrix_r','bigint','ponteiro para pgm_matrix_double resultante. R = A - B'),
          ]),
          funcaoFactory.criar('pgm_matrix_transpose','pgm_matrix_transpose(matrix bigint) RETURNS bigint','Transpõe a matrix',[
            parametroFactory.criar('a','bigint','ponteiro para pgm_matrix_double'),
          ],[
            retornoFactory.criar('matrix','bigint','ponteiro para pgm_matrix_double transposta'),
          ]),
          funcaoFactory.criar('pgm_matrix_int2array','pgm_matrix_int2array(matrix bigint)','Converte ponteiro para pgm_matrix_int para array bi-dimencional',[
            parametroFactory.criar('matrix','bigint','ponteiro para pgm_matrix_double'),
          ],[
            retornoFactory.criar('array','integer[]','array de valores'),
          ]),
          funcaoFactory.criar('pgm_weighted_mean','pgm_weighted_mean(matrix_a bigint, p double precision, matrix_b bigint, q double precision) RETURNS bigint','Média ponderada entre as matrizes. C(i,j) = (A(i,j) * P + B(i,j) * Q))/(P+Q)',[
            parametroFactory.criar('A','bigint','ponteiro para pgm_matrix_double'),
            parametroFactory.criar('P','double precision','peso da matriz A'),
            parametroFactory.criar('B','bigint','ponteiro para pgm_matrix_double'),
            parametroFactory.criar('Q','double precision','peso da matriz B'),
          ],[
            retornoFactory.criar('C','bigint','ponteiro para pgm_matrix_double resultante'),
          ]),
        ]
      },

      {
        nome: 'vector',
        descricao:'Estrutura para ponteiro de pgm_vector_double',
        tipo: {
          nome: 'vector',
          lang: 'pgPL/SQL',
          campos: [
            tipoFactory.criar('ptr', 'bigint', 'ponteiro para pgm_vector_double'),
          ]
        },
        operador: [
          operadorFactory.criar('*','double precision * double precision[]','pgm_scale_vector'),
          operadorFactory.criar('*','double precision[] * double precision','pgm_scale_vector'),
          operadorFactory.criar('*','double precision[] * double precision[]','pgm_vector_vector'),
          operadorFactory.criar('+','double precision[] + double precision','opr_sum_vector_float'),
          operadorFactory.criar('+','double precision + double precision[]','opr_sum_float_vector'),
          operadorFactory.criar('+','double precision[] + double precision[]', 'opr_sum_vector_vector'),
          operadorFactory.criar('-','double precision[] - double precision[]', 'pgm_vector_distance'),
          operadorFactory.criar('-','double precision[] - double precision', 'opr_sub_vector_float'),
          operadorFactory.criar('-','double precision - double precision[]', 'opr_sub_float_vector'),
          operadorFactory.criar('/','double precision[] / double precision[]', 'opr_div_vector_vector'),
          operadorFactory.criar('/','double precision / double precision[]', 'opr_div_float_vector'),
          operadorFactory.criar('*','vector * matrix','opr_vector_matrix_multiply'),
          operadorFactory.criar('==>','vector ==> text','vector2table'),
        ],
        funcoes: [
          funcaoFactory.criar('pgm_vector_distance','pgm_vector_distance(a double precision[], b double precision[]) RETURNS double precision[]','Distancia entre vetores',[
            parametroFactory.criar('a','double presicion[]','array uni-dimencional'),
            parametroFactory.criar('b','double presicion[]','array uni-dimencional'),
          ],[
            retornoFactory.criar('distance','double precision[]','vetor distancia'),
          ]),
          funcaoFactory.criar('pgm_vector_double2array','pgm_vector_double2array(vector bigint) RETURNS double precision[]','Converte pgm_vector_double em array uni-dimencional',[
            parametroFactory.criar('vector','bigint','ponteiro para pgm_vector_double'),
          ],[
            retornoFactory.criar('array','double precision[]','array uni-dimensional'),
          ]),
          funcaoFactory.criar('pgm_vector_double2array','pgm_vector_double2array(a vector) RETURNS double precision[]','Converte vector em array uni-dimensional',[
            parametroFactory.criar('vector','vector','vetor'),
          ],[
            retornoFactory.criar('array','double precision[]','array uni-dimensional'),
          ]),
          funcaoFactory.criar('pgm_vector_double_copy','pgm_vector_double_copy(vector bigint) RETURNS bigint','Copia um pgm_vector_double',[
            parametroFactory.criar('vector','bigint','ponteiro para pgm_vector_double'),
          ],[
            retornoFactory.criar('vector_c','bigint','ponteiro para pgm_vector_double copiado'),
          ]),
          funcaoFactory.criar('pgm_vector_double_create','pgm_vector_double_create(n_elem integer) RETURNS bigint','Cria um pgm_vector_double',[
            parametroFactory.criar('n_elem','integer','número de elementos do pgm_vetor_double'),
          ],[
            retornoFactory.criar('vector','bigint','ponteiro para pgm_vector_double criado'),
          ]),
          funcaoFactory.criar('pgm_vector_double_free','pgm_vector_double_free(vector bigint) RETURNS void','Libera memória do pgm_vector_double',[
            parametroFactory.criar('vector','bigint','ponteiro para pgm_vector_double'),
          ],[]),
          funcaoFactory.criar('pgm_vector_double_get','pgm_vector_double_get(vector bigint) RETURNS double precision[]','Converte pgm_vector_double para array uni-dimensional',[
            parametroFactory.criar('vector','bigint','ponteiro para pgm_vector_double'),
          ],[
            retornoFactory.criar('array','double precision[]','array uni-dimensional'),
          ]),
          funcaoFactory.criar('pgm_vector_double_get_elem','pgm_vector_double_get_elem(vector bigint, pos integer) RETURNS double precision','Obtem o valor do elemento do vetor',[
            parametroFactory.criar('vector','bigint','ponteiro para pgm_vector_double'),
            parametroFactory.criar('pos','integer','índice do vetor'),
          ],[
            retornoFactory.criar('elemento','double precision','valor do elemento. V(pos)'),
          ]),
          funcaoFactory.criar('pgm_vector_double_matrix_multiply','pgm_vector_double_matrix_multiply(vector bigint, matrix bigint) RETURNS bigint','Multiplica vetor com matriz',[
            parametroFactory.criar('vector','bigint','ponteiro para pgm_vector_double'),
            parametroFactory.criar('matrix','bigint','ponteiro para pgm_matrix_double'),
          ],[
            retornoFactory.criar('matrix','bigint','ponteiro para pgm_matrix_double'),
          ]),
          funcaoFactory.criar('pgm_vector_double_nlines','pgm_vector_double_nlines(vector bigint) RETURNS integer','Obtem o número de elementos de um vetor',[
            parametroFactory.criar('vector','bigint','ponteiro para pgm_vector_double'),
          ],[
            retornoFactory.criar('n_elems','integer','Número de elementos do vector'),
          ]),
          funcaoFactory.criar('pgm_vector_double_set_elem','pgm_vector_double_set_elem(vector bigint, pos integer, val double precision) RETURNS bigint','Seta o valor do vetor',[
            parametroFactory.criar('vector','bigint','ponteiro para pgm_vector_double'),
            parametroFactory.criar('pos','integer','índice do vetor'),
            parametroFactory.criar('valor','double precision','novo valor'),
          ],[
            retornoFactory.criar('vector','bigint','ponteiro para pgm_vector_double'),
          ]),
          funcaoFactory.criar('pgm_vector_int2array','pgm_vector_int2array(vector bigint) RETURNS integer[]','Converte vetor pgm_vector_int para array uni-dimencional',[
            parametroFactory.criar('vector','bigint','ponteiro para pgm_vector_int'),
          ],[
            retornoFactory.criar('array','integer[]','array uni-dimensional'),
          ]),
          funcaoFactory.criar('pgm_vector_int_create','pgm_vector_int_create(n_elems integer) RETURNS bigint','cria pgm_vector_int',[
            parametroFactory.criar('n_elems','integer','número de elementos do vetor'),
          ],[
            retornoFactory.criar('vector','bigint','ponteiro para pgm_vector_int'),
          ]),
          funcaoFactory.criar('pgm_vector_int_free','pgm_vector_int_free(vector bigint) RETURNS void','libera memória do pgm_vector_int',[
            parametroFactory.criar('vector','bigint','ponteiro para pgm_vector_int'),
          ],[]),
          funcaoFactory.criar('pgm_vector_int_get','pgm_vector_int_get(vector bigint) RETURNS integer[]','Converte vetor pgm_vector_int para array uni-dimencional',[
            parametroFactory.criar('vector','bigint','ponteiro para pgm_vector_int'),
          ],[
            retornoFactory.criar('array','integer[]','array uni-dimensional'),
          ]),
          funcaoFactory.criar('pgm_vector_int_get_elem','pgm_vector_int_get_elem(vetor bigint, posicao integer) RETURNS integer','Obtem o valor de uma posição do vetor',[
            parametroFactory.criar('vector','bigint','ponteiro para pgm_vector_int'),
          ],[
            retornoFactory.criar('elemento','integer','valor do elemento na posição pos. V(i)'),
          ]),
          funcaoFactory.criar('pgm_vector_int_nlines','pgm_vector_int_nlines(vetor bigint) RETURNS integer','Retorna o número de elementos do vetor',[
            parametroFactory.criar('vector','bigint','ponteiro para pgm_vector_int'),
          ],[
            retornoFactory.criar('nlines','integer','Número de elementos do vetor'),
          ]),
          funcaoFactory.criar('pgm_vector_int_set_elem','pgm_vector_int_set_elem(vector bigint, pos integer, val integer) RETURNS bigint','Seta o valor de um elemento do vetor',[
            parametroFactory.criar('vector','bigint','ponteiro para pgm_vector_int'),
            parametroFactory.criar('pos','integer','índice do elemento'),
            parametroFactory.criar('val','integer','novo valor'),
          ],[
            retornoFactory.criar('vector','bigint','ponteiro para pgm_vector_int'),
          ]),
          funcaoFactory.criar('pgm_vector_norm','pgm_vector_norm(vector bigint) RETURNS double precision','Calcula a norma do vetor',[
            parametroFactory.criar('vector','bigint','ponteiro para pgm_vector_double'),
          ],[
            retornoFactory.criar('norm','double precision','Norma do vetor'),
          ]),
          funcaoFactory.criar('pgm_vector_vector','pgm_vector_vector(a double precision[], b double precision[]) RETURNS double precision[]','C é dado por: Para cada elemento é em A é multiplicado por cada elemento em B.',[
            parametroFactory.criar('A','double precision[]','array uni-dimensional'),
            parametroFactory.criar('B','double precision[]','array uni-dimensional'),
          ],[
            retornoFactory.criar('vector_vector','double precision[]','array resultante'),
          ]),
          funcaoFactory.criar('pgm_vectorint2array','pgm_vectorint2array(vector bigint) RETURNS integer[]','Converte um pgm_vetor_int para array uni-dimensional',[
            parametroFactory.criar('vector','bigint','ponteiro para pgm_vector_int'),
          ],[
            retornoFactory.criar('array','integer[]','array uni-dimensional'),
          ]),
          funcaoFactory.criar('pgm_array2vector_double','pgm_array2vector_double(unidimensional_array double precision[]) RETURNS bigint','Converte array uni-dimensional para pgm_vector_double',[
            parametroFactory.criar('unidimensional_array','double precision[]','array uni-dimensional'),
          ],[
            retornoFactory.criar('vector','bigint','ponteiro para pgm_vector_double'),
          ]),
          funcaoFactory.criar('pgm_array2vector_int','pgm_array2vector_int(unidimensional_array integer[]) RETURNS bigint','Converte array uni-dimensional para pgm_vector_int',[
            parametroFactory.criar('unidimensional_array','integer[]','array uni-dimensional'),
          ],[
            retornoFactory.criar('vector','bigint','ponteiro para pgm_vector_integer'),
          ]),
        ]
      },
    ];

    $scope.lista = setup(listaFuncoes);
  });

})(angular.module('pgminer'));
