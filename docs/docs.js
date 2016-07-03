{
  nome: 'blas_multiply',
  descricao:'Função executa a multiplicação de duas matrizes. C = A * B',
  cabecalho:'las_multiply(a bigint, b bigint) RETURNS bigint',
  parametros: [
    parametroFactory.criar('A', 'bigint', 'Ponteiro para Matriz A')
    parametroFactory.criar('B', 'bigint', 'Ponteiro para Matriz B')
  ],
  retorno:[
    retornoFactory.criar('C', 'bigint', 'Ponteiro para matriz resultante')
  ]
},
{
  nome: 'classe2vector',
  descricao:'Transforma o conjunto de classes para vetores de valores 0 a 1',
  cabecalho:'classe2vector(sql_distinct_classes text, OUT classe text, OUT vector double precision[]) RETURNS SETOF record',
  parametros: [
    parametroFactory.criar("sql_distinct_classes', 'text', 'EX: select * from classe2vector( 'select distinct classe from iris order by classe' )")
  ],
  retorno:[
    retornoFactory.criar('classe', 'matrix', 'Matriz com valores aplicados na função sigmoid'),
    retornoFactory.criar('vector', 'double precision[]', 'Matriz com valores aplicados na função sigmoid')
  ]
},

