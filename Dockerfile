FROM postgres:9.4
MAINTAINER Fausto Junqueira <https://github.com/faustofjunqueira>

#Config sourcelist to add libpq-dev
RUN echo "deb http://apt.postgresql.org/pub/repos/apt/ jessie-pgdg main" > pgdg.list
RUN apt-get update

#dev dependences
RUN apt-get install build-essential -y

#postgresql 9.4 dependences
RUN apt-get install libpq5 libpq-dev postgresql-server-dev-9.4 -y

#lapack dependences
RUN apt-get install -y libblas-doc libblas-dev libblas-test libblas3gf liblapack-pic liblapack-dev liblapack-doc liblapack-test liblapack3gf


ENV PGMINERDIR="/pgminer"
ENV PGMINERMAKEDIR="/pgminer/pgminer"

#Add lapack lib
ADD src/lapacke/ /usr/include/
