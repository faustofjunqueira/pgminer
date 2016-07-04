{
  nome: 'free',
  descricao:'Desaloca uma matriz',
  cabecalho:'free(m matrix) RETURNS void',
  parametros: [
    parametroFactory.criar('m', 'matrix', 'Matriz de valores')
  ],
  retorno:[]
},
{
  nome: 'get_col',
  descricao:'Retorna a coluna de uma matriz',
  cabecalho:'get_col(m matrix, col integer) RETURNS double precision[]',
  parametros: [
    parametroFactory.criar('m', 'matrix', 'Matriz de valores'),
    parametroFactory.criar('col', 'integer', 'índice da coluna'),
  ],
  retorno:[
    retornoFactory.criar('coluna', 'double precision[]', 'array com os valores da coluna')
  ]
},
{
  nome: 'get_line',
  descricao:'Retorna a linha de uma matriz',
  cabecalho:'get_line(m matrix, line integer) RETURNS double precision[]',
  parametros: [
    parametroFactory.criar('m', 'matrix', 'Matriz de valores'),
    parametroFactory.criar('line', 'integer', 'índice da linha')
  ],
  retorno:[
    retornoFactory.criar('line', 'double precision[]', 'array com os valores da linha')
  ]
},
{
  nome: 'inv',
  descricao:'Retorna a matriz inversa',
  cabecalho:'inv(m matrix, OUT c matrix) RETURNS matrix',
  parametros: [
    parametroFactory.criar('m', 'matrix', 'Matriz de valores')
  ],
  retorno:[
    retornoFactory.criar('c', 'matrix', 'matriz inversa de m')
  ]
},
{
  nome: 'inverse_sigmoid',
  descricao:'Retorna o valor da sigmoid inversa. Inversa da sigmoid: ln( x/(1.0 - x) )/2.0',
  cabecalho:[
    'inverse_sigmoid(a double precision[]) RETURNS double precision[]',
    'inverse_sigmoid(e double precision) RETURNS double precision'
  ]
  parametros: [
    parametroFactory.criar('a', 'double precision[]', 'array de valores que serão aplicados a x na função '),
    parametroFactory.criar('x', 'double precision', 'valor a ser aplicado na função')
  ],
  retorno:[
    retornoFactory.criar('inv', 'double precision[]', 'valores da inversa da sigmoid'),
    retornoFactory.criar('inv', 'double precision', 'valor da inversa da sigmoid')
  ]
},
{
  nome: 'make_dense_matrix',
  descricao:"Cria uma matiz densa a partir de um SQL de uma matriz esparsa",
  cabecalho:"make_dense_matrix(sql text, zero text DEFAULT ''::text, centrar boolean DEFAULT true) RETURNS bigint",
  parametros: [
    parametroFactory.criar('sql', 'text', "SQL de uma matriz esparsa do tipo 'select linha, coluna, valor from ..."),
    parametroFactory.criar('zero', 'text', "significa que zero é zero; não haverá substituição; 'mavg' significa substituir zero pela média dos valores da matriz inteira; 'lavg' significa substituir os zeros de cada linha pela média de cada linha; 'cavg' significa substituir os zeros de cada coluna pela média de cada coluna"),
    parametroFactory.criar('centrar', 'bool', "centrar a média em zero, o que equivale a subtrair de cada valor diferente de zero a média escolhida."),
  ],
  retorno:[
    retornoFactory.criar('matrix', 'bigint', "ponteiro para a matriz densa")
  ]
},
{
  nome: 'make_matrix',
  descricao:'Cria uma matriz a partir de um sql',
  cabecalho:'make_matrix(sql text, OUT matrix bigint, OUT keys bigint) RETURNS record',
  parametros: [
    parametroFactory.criar('sql', 'text', "'sql' deve ser uma string retornando (id,valor), onde 'id integer primary key' e valor é um array float8[] com os valores do ponto, ou uma tabela/view com as colunas id::integer, valor::float8[] EX1: 'select id, array[ x, y ] valor from tabela' EX2: 'select cast( row_number() over() as integer ) id, array[ x, y ] valor from tabela'")
  ],
  retorno:[
    retornoFactory.criar('matrix', 'bigint', "ponteiro para matrix"),
    retornoFactory.criar('keys', 'bigint', "ponteiro para vetor contendo a chave (id) de cada linha da matriz.")
  ]
},
{
  nome: 'make_random_matrix',
  descricao:'Criar uma matriz com valores randômicos',
  cabecalho:'make_random_matrix(n_lines integer, n_cols integer, a double precision DEFAULT 0.0, b double precision DEFAULT 1.0) RETURNS bigint',
  parametros: [
    parametroFactory.criar('n_lines', 'integer', 'número de linhas'),
    parametroFactory.criar('n_cols', 'integer', 'número de colunas'),
    parametroFactory.criar('a', 'double precision', 'valores de'),
    parametroFactory.criar('b', 'double precision', 'valores até'),
  ],
  retorno:[
    retornoFactory.criar('matrix', 'bigint', 'ponteiro para matriz')
  ]
},
{
  nome: 'matrix',
  descricao:'Criar uma matrix',
  cabecalho:[
    'matrix(ptr_matrix bigint, OUT c matrix) RETURNS matrix',
    'matrix(bidimensional_array double precision[], OUT c matrix) RETURNS matrix',
    'matrix(sql text, OUT c matrix) RETURNS matrix'
  ],
  parametros: [
    parametroFactory.criar('ptr_matrix', 'bigint', 'ponteiro para a matrix'),
    parametroFactory.criar('bidimensional_array','double precision[]', 'array de valores de duas dimensões'),
    parametroFactory.criar('sql','text', 'sql similar ao make_matrix')
  ],
  retorno:[
    retornoFactory.criar('c', 'matrix', 'matrix estruturada')
  ]
},
{
  nome: 'matrix2table',
  descricao:'Converte matriz em tabela',
  cabecalho:[
    'matrix2table(m matrix, table_name text, OUT nlines integer, OUT ncols integer) RETURNS record',
    'matrix2table(ptr_matrix bigint, table_name text, OUT nlines integer, OUT ncols integer) RETURNS record',
    'matrix2table(keys bigint, matrix bigint, table_name text, OUT nlines integer, OUT ncols integer) RETURNS record'
  ],
  parametros: [
    parametroFactory.criar('matrix', 'matrix', 'Matriz de valores'),
    parametroFactory.criar('ptr_matrix', 'bigint', 'ponteiro para matriz de valores'),
    parametroFactory.criar('keys', 'bigint', 'ponteiro para vetor com as chaves da nova tabela'),
    parametroFactory.criar('table_name', 'text', 'nome da tabela'),
  ],
  retorno:[
    retornoFactory.criar('nlines', 'integer', 'número de linhas da tabela'),
    retornoFactory.criar('ncols', 'integer', 'número de colunas da tabela'),
  ]
},
{
  nome: 'matrix2table_transpose',
  descricao:'Converte matriz em tabela, com a transposta',
  cabecalho:[
    'matrix2table_transpose(matrix bigint, table_name text, OUT nlines integer, OUT ncols integer) RETURNS record',
    'matrix2table_transpose(keys bigint, matrix bigint, table_name text, OUT nlines integer, OUT ncols integer) RETURNS record'
  ],
  parametros: [
    parametroFactory.criar('matrix', 'bigint', 'ponteiro para matriz de valores'),
    parametroFactory.criar('keys', 'bigint', 'ponteiro para vetor com as chaves da nova tabela'),
    parametroFactory.criar('table_name', 'text', 'nome da tabela'),
  ],
  retorno:[
    retornoFactory.criar('nlines', 'integer', 'número de linhas da tabela'),
    retornoFactory.criar('ncols', 'integer', 'número de colunas da tabela'),
  ]
},
{
  nome: 'max',
  descricao:'Retorna o maior valor',
  cabecalho:'max(a double precision, b double precision) RETURNS double precision',
  parametros: [
    parametroFactory.criar('a', 'double precision', 'valor a ser comparado'),
    parametroFactory.criar('b', 'double precision', 'valor a ser comparado'),
  ],
  retorno:[
    retornoFactory.criar('max', 'double precision', 'max = a > b ? a : b')
  ]
},
{
  nome: 'max_elem',
  descricao:'Retorna o maior valor presente no array',
  cabecalho:'max_elem(a double precision[]) RETURNS double precision',
  parametros: [
    parametroFactory.criar('a', 'double precision[]', 'array de valores')
  ],
  retorno:[
    retornoFactory.criar('max', 'double precision', 'elemento de maior valor no array')
  ]
},
{
  nome: 'min',
  descricao:'Retorna o menor valor',
  cabecalho:'min(a double precision, b double precision) RETURNS double precision',
  parametros: [
    parametroFactory.criar('a', 'double precision', 'valor a ser comparado'),
    parametroFactory.criar('b', 'double precision', 'valor a ser comparado'),
  ],
  retorno:[
    retornoFactory.criar('min', 'double precision', 'max = a < b ? a : b')
  ]
},
{
  nome: 'ncols',
  descricao:'Retorna o número de colunas de uma matrix',
  cabecalho:'ncols(m matrix, OUT cols integer) RETURNS integer',
  parametros: [
    parametroFactory.criar('m', 'matrix', 'Matriz de valores')
  ],
  retorno:[
    retornoFactory.criar('cols', 'integer', 'Número de colunas da matriz m')
  ]
},
{
  nome: 'nlines',
  descricao:'Retorna o número de linhas de uma matrix',
  cabecalho:'nlines(m matrix, OUT nlines integer) RETURNS integer',
  parametros: [
    parametroFactory.criar('m', 'matrix', 'Matriz de valores')
  ],
  retorno:[
    retornoFactory.criar('nlines', 'integer', 'Número de linhas da matriz m')
  ]
},
{
  nome: 'norm',
  descricao:'Retorna a norma do array a',
  cabecalho:['norm(a double precision[]) RETURNS double precision','norm(m matrix) RETURNS double precision'],
  parametros: [
    parametroFactory.criar('a', 'double precision[]', 'array de valores'),
    parametroFactory.criar('m', 'matrix', 'matriz de valores')
  ],
  retorno:[
    retornoFactory.criar('norm', 'double precision', 'norma do array')
  ]
},