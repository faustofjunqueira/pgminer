DIST=BETA
PORT=5431
CONTAINERNAME=pgminer1
VERSION=9.4

all:
	sudo docker exec $(CONTAINERNAME) make

clean:
	sudo docker exec $(CONTAINERNAME) make Clear

start:
	sudo docker start $(CONTAINERNAME)

install:
	sudo docker run -d -e POSTGRES_PASSWORD="postgres" -e POSTGRES_USER="postgres" -w /pgminer/pgminer -v $(shell pwd)/src:/pgminer -e VERSIONPG="$(VERSION)" -p $(PORT):5432 --name $(CONTAINERNAME) faustofjunqueira/pgminer

destroy:
	sudo docker rm -f $(CONTAINERNAME)

deploy:
	sudo docker exec $(CONTAINERNAME) make deploy
	sudo docker restart $(CONTAINERNAME)

dist: deploy
	rm -f pgminer*.tar.gz
	mkdir -p pgminer
	cp src/bin/pgminer.so pgminer
	cp -r src/lapacke/ pgminer
	cp src/sql/extension.pgminer.sql pgminer
	chmod +x install.sh
	cp install.sh pgminer
	tar -zcvf pgminer_$(shell date +%s)_$(DIST).tar.gz pgminer/*
	rm -rf pgminer

%.o:
	sudo docker exec $(CONTAINERNAME) make $@
