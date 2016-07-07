EXTENSION = pgminer        # the extensions name
DATA = pgminer--0.0.1.sql  # script files to install
MODULES = pgminer

PG_CONFIG = pg_config
PGXS = $(shell $(PG_CONFIG) --pgxs)
INCLUDEDIR = $(shell $(PG_CONFIG) --includedir-server)
include $(PGXS)

VERSIONPG=9.4

install: all

all:
	echo "Instalação de dependencias..."
	apt-get install -y libblas-doc libblas-dev libblas-test libblas3gf liblapack-pic liblapack-dev liblapack-doc liblapack-test liblapack3gf
	echo "Instalando o Lapacke..."
	cp -r lapacke/ /usr/include/
	echo "Instalando o Pgminer..."
	cp pgminer.so /usr/lib/postgresql/$VERSIONPG/lib/
