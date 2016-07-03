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
      }
    ];
  });

})(angular.module('pgminer'));