CC=gcc

all: regra1 regra2 clean

regra1: #dependências para a regra1
	$(CC) -c lib.c ../include/support.h ../include/cthread.h ../include/cdata.h

regra2: #dependências para a regra2
	$(CC) lib.o ../bin/support.o -o saida

clean:
	rm -rf *.gch lib.o