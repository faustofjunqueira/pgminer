#!/bin/bash

sudo docker run -d \
  -p 5432:5432 \
  -e POSTGRES_PASSWORD=postgresql \
  -v pgminer/psqldata/:/var/lib/postgresql/data \
  -v pgminer/psqllib/:/usr/lib/postgresql/9.3/lib \
  faustofjunqueira/pgminer