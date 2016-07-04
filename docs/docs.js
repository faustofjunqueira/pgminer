
{
  nome: 'pgm_hello_world',
  descricao:'Função de Hello World do Pgminer',
  cabecalho:'FUNCTION pgm_hello_world(entrada CSTRING) RETURNS CSTRING',
  parametros: [
    parametroFactory.criar('entrada', 'CSTRING', 'uma palavra qualquer')
  ],
  retorno:[
    retornoFactory.criar('saida', 'CSTRING', 'a mesma palavra da entrada')
  ]
},
{
  nome: 'pgm_kmeans',
  descricao:'Executa o algoritmo de clusterização k-means',
  cabecalho:'pgm_kmeans(pontos bigint, k integer, seed integer, fim integer, OUT centroide bigint, OUT grupo bigint, OUT sumd double precision) RETURNS record',
  parametros: [
    parametroFactory.criar('pontos', 'bigint', 'matriz de pontos'),
    parametroFactory.criar('k', 'integer', 'número de grupos desejados'),
    parametroFactory.criar('seed', 'integer', '1 - para sortear k pontos para serem os centros dos grupos; 2 - para sortear cada ponto em um grupo'),
    parametroFactory.criar('fim', 'integer', '1 - se um Grupo Secar , Elimine - o; 2 - se um Grupo Secar , cria um grupo com o elemento mais longe de seu centroide; 3 - se um Grupo Secar , Erro .'),
  ],
  retorno:[
    retornoFactory.criar('centroide', 'bigint', 'ponteiro para matriz com os centroídes'),
    retornoFactory.criar('grupos', 'bigint', 'O grupo é um ponteiro para vetor de n posições, onde n é o número total de pontos p k , que armazena na posição i, onde i é o índice na matriz correspondente ao ponto p i , o grupo que o ponto p i está alocado. Logo, gi = grupo(pi)'),
    retornoFactory.criar('sumd', 'double precision', 'soma quadrática mínima'),
  ]
},
{
  nome: 'pgm_cos',
  descricao:'Aplica função cosseno em duas matrizes',
  cabecalho:'pgm_cos(matrix_a bigint, matrix_b bigint) RETURNS bigint',
  parametros: [
    parametroFactory.criar('a', 'bigint', 'Matriz de valores'),
    parametroFactory.criar('b', 'bigint', 'Matriz de valores'),
  ],
  retorno:[
    retornoFactory.criar('cosseno', 'bigint', 'ponteiro para vetor')
  ]
},
