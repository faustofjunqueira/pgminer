FROM ubuntu:14.04
MAINTAINER Fausto Junqueira (https://github.com/faustofjunqueira/pgminer)


# explicitly set user/group IDs
RUN groupadd -r postgres --gid=999 && useradd -r -g postgres --uid=999 postgres

# Update (and upgrade) Image
RUN apt-get update && apt-get upgrade -y
RUN apt-get install -y \
  build-essential \
  gcc \
  libpgtypes3 \
  libpq5 \
  postgresql-9.3 \
  postgresql-common \
  postgresql-contrib-9.3 \
  postgresql-doc-9.3 \
  postgresql-server-dev-9.3

RUN localedef -i en_US -c -f UTF-8 -A /usr/share/locale/locale.alias en_US.UTF-8

ENV LANG en_US.utf8

RUN mkdir /docker-entrypoint-initdb.d

RUN apt-key adv --keyserver ha.pool.sks-keyservers.net --recv-keys B97B0AFCAA1A47F044F244A07FCC7D46ACCC4CF8

ENV PG_MAJOR 9.3
ENV PG_VERSION 9.3.10-1.pgdg80+1

RUN mkdir -p /var/run/postgresql && chown -R postgres /var/run/postgresql

ENV PATH /usr/lib/postgresql/$PG_MAJOR/bin:$PATH
ENV PGDATA /var/lib/postgresql/data

VOLUME /var/lib/postgresql/data

ADD docker-entrypoint.sh /

VOLUME /var/lib/postgresql/data
VOLUME /usr/lib/postgresql/$PG_MAJOR/lib

EXPOSE 5432
ENTRYPOINT ["/docker-entrypoint.sh"]
CMD ["postgres"]