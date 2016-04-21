#!/bin/bash

VERSIONPG=9.4

apt-get install -y libblas-doc libblas-dev libblas-test libblas3gf liblapack-pic liblapack-dev liblapack-doc liblapack-test liblapack3gf

cp src/lapacke/ /usr/include/

cp dist/pgminer.so /usr/lib/postgresql/$(VERSIONPG)/lib/
