#!/bin/bash

VERSIONPG=9.4

echo "Instalação de dependencias..."
apt-get install -y libblas-doc libblas-dev libblas-test libblas3gf liblapack-pic liblapack-dev liblapack-doc liblapack-test liblapack3gf

echo "Instalando o Lapacke..."
cp -r lapacke/ /usr/include/

echo "Instalando o Pgminer..."
cp pgminer.so /usr/lib/postgresql/$VERSIONPG/lib/
