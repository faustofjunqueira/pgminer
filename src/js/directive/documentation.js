(function(app){

app.directive('pgmDocumentacao', function(){
  return {
    templateUrl: '/pgminer/src/template/pgmdocumentacao.html',
    restrict: 'EA',
    scope: {
      listaFuncao: '=lista'
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
    templateUrl: '/pgminer/src/template/pgmfuncao.html',
    controller: function($scope){
      $scope.isArray = function(e){
        return Array.isArray(e);
      }
    }
  };
});

app.directive('pgmTipo', function(){
  return {
    scope: {
      funcao: '='
    },
    restrict: 'EA',
    templateUrl: '/pgminer/src/template/pgmtipo.html',
    controller: function($scope){
      $scope.isArray = function(e){
        return Array.isArray(e);
      }
    }
  };
});

})(angular.module('pgminer'));