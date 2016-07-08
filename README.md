# PGMINER
## Extensão para mineração de dados integrado ao PostgreSQL

O Pgminer é um extensão para mineração de dados, aprendizado de máquina e álgebra linear integrado ao sistema de gerenciamento de banco de dados PostgreSQL. A extensão é composta por uma série de algoritmos clássicos de mineração de dados, tais como Rede Neural Artificial, Clusterização K-means, Singular Value Decomposition(SVD), entre outros, além de também contar com algoritmos de manipulação vetorial e matricial, como multiplicação de matriz e vetor, cálculo de cosseno e mais. O Pgminer é basicamente desenvolvido em linguagem C/C++ e integrado com o PosgreSQL utilizando PgPLSQL ( linguagem nativa do SGBD )...

Para acessar a **documentação** do Pgminer [clique aqui](http://faustofjunqueira.github.io/pgminer/documentacao.html)


### Instalação

Instalar o Pgminer é muito simples. Siga os passos abaixo:

* basta fazer o download da ferramenta([download aqui](https://github.com/faustofjunqueira/pgminer/releases/download/v0.0.1/pgminer_1467942579_BETA_0.0.1.tar.gz))

* Extraia o arquivo do pgminer

```sh
 tar -xzf pgminer_1467942579_BETA_0.0.1.tar.gz
```

* Navegue para a pasta extraida
```sh
 cd pgminer
```

* Execute o Makefile install
```sh
 sudo make install
```

Aguarde finalizar a instalação. Após acesse seu banco de dados e execute a criação da extensão Pgminer:
```sql
 create extension pgminer;
```

#### Requerimento de instalação

Para instalar o pgminer necessita atender a alguns requerimentos.

* Verifique se os pacotes abaixo estão instalados:
  * build-essential
  * libpq5
  * libpq-dev
  * postgresql-server


## O Pgminer 
**EM BREVE**