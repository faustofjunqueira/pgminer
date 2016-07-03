{
  nome: 'cleanup_table',
  descricao:'Zera ou cria uma tabela',
  cabecalho:'cleanup_table(table_name text, table_columns text, make_temp bool)',
  parametros: [
    parametroFactory.criar('table_name', 'text', 'Nome da tabela'),
    parametroFactory.criar('table_columns', 'text', "Colunas da tabela. Ex: '( GRUPO INTEGER PRIMARY KEY, VALOR FLOAT8[] NOT NULL )'"),
    parametroFactory.criar('make_temp', 'bool', 'Se true, a tabela será temporária, caso contrário a tabela será permanente')
  ],
  retorno:[]
},
{
  nome: 'apply_sigmoid',
  descricao:'Aplica a função sigmoid sobre cada elemento x da matriz m. Sigmoid: 2.0/(1.0 + exp( -16.0*x ) ) - 1.0',
  cabecalho:'cos(a matrix, b matrix) | cos(a matrix, b matrix)',
  parametros: [
    parametroFactory.criar('m', 'matrix', 'Matriz de valores')
  ],
  retorno:[
    retornoFactory.criar('c', 'matrix', 'Matriz com valores aplicados na função sigmoid')
  ]
},


{
  nome: 'apply_sigmoid',
  descricao:'Aplica a função sigmoid sobre cada elemento x da matriz m. Sigmoid: 2.0/(1.0 + exp( -16.0*x ) ) - 1.0',
  cabecalho:'cleanup_table(table_name text, table_columns text, make_temp bool)',
  parametros: [
    parametroFactory.criar('m', 'matrix', 'Matriz de valores')
  ],
  retorno:[
    retornoFactory.criar('c', 'matrix', 'Matriz com valores aplicados na função sigmoid')
  ]
}

