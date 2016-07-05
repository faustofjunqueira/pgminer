
(function(app){

app.service('tipoFactory', function(){
  
  this.criar = function(nome, tipo, descricao){
    return {
      nome: nome,
      tipo: tipo,
      descricao: descricao
    }
  };

});

})(angular.module('pgminer'));