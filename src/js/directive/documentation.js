(function(app){

app.directive('pgmDocumentacao', function(){
  return {
    templateUrl: '/src/template/pgmdocumentacao.html',
    restrict: 'EA',
    scope: {
      listaFuncao: '=lista'
    },
    controller: function($scope, $element, $attrs){
      this.teste = function(){
        alert($scope.listaFuncao);
      };
    },
    link: function($scope,$element,$attr, ctrl){
      $scope.escolhida = $scope.listaFuncao[0];
      $scope.escolherFuncao = function(funcao){
        $scope.escolhida = funcao;
      };
    }
  };

});

app.directive('pgmFuncao', function(){
  return {
    scope: {
      funcao: '='
    },
    restrict: 'EA',
    templateUrl: '/src/template/pgmfuncao.html'
  };
});



})(angular.module('pgminer'));