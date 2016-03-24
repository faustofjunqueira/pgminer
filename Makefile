PORT=5431
CONTAINERNAME=pgminer

all:
	sudo docker exec $(CONTAINERNAME) make

clean:
	sudo docker exec $(CONTAINERNAME) make clean

start:
	sudo docker start $(CONTAINERNAME)

install:
	sudo docker run -d -e POSTGRES_PASSWORD="postgres" -e POSTGRES_USER="postgres" -w /pgminer/pgminer -v $(shell pwd)/src:/pgminer -p $(PORT):5432 --name $(CONTAINERNAME) faustofjunqueira/pgminer

destroy:
	sudo docker rm -f $(CONTAINERNAME)

%.o:
	sudo docker exec $(CONTAINERNAME) make $@