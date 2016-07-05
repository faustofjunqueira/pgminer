(function(app){

app.service('funcaoFactory', function(){
  
  this.criar = function(nome, cabecalho, descricao, paramList, retList){
    return {
      nome: nome,
      cabecalho: cabecalho,
      descricao: descricao,
      param: paramList,
      ret: retList
    };
  };

});

})(angular.module('pgminer'));