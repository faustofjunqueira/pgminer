FROM postgres:latest
MAINTAINER Fausto Junqueira (https://github.com/faustofjunqueira/pgminer)

# Update (and upgrade) Image
RUN apt-get update
RUN apt-get upgrade -y

RUN apt-get install -y \
  build-essential \
  gcc \
  g++ \
  libpgtypes3 \
  libpq-dev \
  postgresql-common \
  postgresql-doc \
  postgresql-server-dev-all

VOLUME /var/lib/postgresql/data
VOLUME /usr/lib/postgresql/$PG_MAJOR/lib

EXPOSE 5432
