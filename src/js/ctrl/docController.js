(function(app){

  app.controller('DocController', function($scope,parametroFactory, retornoFactory){
    $scope.listaFuncoes = [
      {
        id: 1,
        nome: 'apply_inverse_sigmoid',
        descricao:'oi mundo',
        cabecalho:'qlqr(1,b)',
        parametros: [
          parametroFactory.criar('par1', 'integer', 'simples parametro'),
          parametroFactory.criar('par2', 'string', 'simples parametro22')
        ],
        retorno:[
          retornoFactory.criar('par1', 'integer', 'simples parametro'),
          retornoFactory.criar('par2', 'string', 'simples parametro22')
        ],
        veja:[
          '/lin', '/das'
        ]
      },
      {
        id: 2,
        nome: 'qlqr coisa2',
        descricao:'oi mu3123123ndo',
        cabecalho:'qlqr(1,b)',
        parametros: [
          parametroFactory.criar('par1', 'integer', 'simples parametro'),
          parametroFactory.criar('par2', 'string', 'simples parametro22132231')
        ],
        retorno:[
          retornoFactory.criar('par1', 'integer', 'simples parametro'),
          retornoFactory.criar('par2', 'string', 'simples parametro22')
        ],
        veja:[
          '1', '2'
        ]
      },
    ];
  });

})(angular.module('pgminer'));