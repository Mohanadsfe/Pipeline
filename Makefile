CPP=g++
CC=gcc
AR=ar
FLAGS= -Wall -g3

all: st_pipelie   

st_pipelie: st_pipelie.o 
	$(CC) $(FLAGS) -o st_pipelie st_pipelie.o -lpthread

st_pipelie.o: st_pipelie.c
	$(CC) $(FLAGS) -c st_pipelie.c


.PHONY: clean all
clean:
	rm -f *.o st_pipelie   