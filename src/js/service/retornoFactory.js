(function(app){

app.service('retornoFactory', function(){
  
  this.criar = function(nome, tipo, descricao){
    return {
      nome: nome,
      tipo: tipo,
      descricao: descricao
    }
  };

});

})(angular.module('pgminer'));