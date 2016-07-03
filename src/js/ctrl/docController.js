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
        cabecalho:'las_multiply(a bigint, b bigint) RETURNS bigint',
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
    ];
  });

})(angular.module('pgminer'));