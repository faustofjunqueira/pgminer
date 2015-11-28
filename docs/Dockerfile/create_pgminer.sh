docker run -d \
  -p 5432:5432 \
  -v pgminer/psqldata/:/var/lib/postgresql/data \
  -v pgminer/psqllib/:/usr/lib/postgresql/$PG_MAJOR/lib \
  faustofjunqueira/pgminer 