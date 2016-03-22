PORT=5431
CONTAINERNAME=pgminer

all:
	sudo docker exec $(CONTAINERNAME) make

clean:
	sudo docker exec $(CONTAINERNAME) make clean

install:
	mkdir -p src
	sudo docker run -d -e POSTGRES_PASSWORD="postgres" -e POSTGRES_USER="postgres" -w /pgminer/pgminer -v $(shell pwd)/src:/pgminer -p $(PORT):5432 --name $(CONTAINERNAME) faustofjunqueira/pgminer

destroy:
	sudo docker rm -f $(CONTAINERNAME)

copy:
	sudo docker exec
	sudo docker cp src $(CONTAINERNAME):/pgminer