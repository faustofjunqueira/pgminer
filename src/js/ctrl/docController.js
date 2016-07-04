(function(app){

  app.controller('DocController', function($scope,parametroFactory, retornoFactory){
    $scope.listaFuncoes = [
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
    ];
  });

})(angular.module('pgminer'));