PORT=5431
CONTAINERNAME=pgminer
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

%.o:
	sudo docker exec $(CONTAINERNAME) make $@