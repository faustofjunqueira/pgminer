(function(app){

app.service('funcaoFactory', function(){
  
  this.criar = function(nome, cabecalho, descricao, paramList, retList){
    return {
      nome: nome,
      cabecalho: cabecalho,
      descricao: descricao,
      parametros: paramList,
      retorno: retList
    };
  };

});

})(angular.module('pgminer'));