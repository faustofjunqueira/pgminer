(function(app){

app.service('parametroFactory', function(){
  
  this.criar = function(nome, tipo, descricao){
    return {
      nome: nome,
      tipo: tipo,
      descricao: descricao
    }
  };

});

})(angular.module('pgminer'));