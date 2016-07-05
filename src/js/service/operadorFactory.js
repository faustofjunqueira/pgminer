
(function(app){

app.service('operadorFactory', function(){
  
  this.criar = function(operador, tipo, funcao){
    return {
      operador: operador,
      tipo: tipo,
      funcao: funcao
    }
  };

});

})(angular.module('pgminer'));