#
# Makefile de EXEMPLO
#
# OBRIGATÓRIO ter uma regra "all" para geração da biblioteca e de uma
# regra "clean" para remover todos os objetos gerados.
#
# É NECESSARIO ADAPTAR ESSE ARQUIVO de makefile para suas necessidades.
#  1. Cuidado com a regra "clean" para não apagar o "support.o"
#
# OBSERVAR que as variáveis de ambiente consideram que o Makefile está no diretótio "cthread"
# 

CC=gcc
LIB_DIR=./lib
INC_DIR=./include
BIN_DIR=./bin
SRC_DIR=./src


all: regra1 regra2 
	
regra1: #cria o .o da biblioteca
	@echo "Criando arquivo lib.o"
	$(CC) -c $(SRC_DIR)/lib.c -o $(BIN_DIR)/lib.o -Wall
	@echo " "

regra2: #cria o .a da biblioteca
	@echo "Criando arquivo libcthread.a"
	ar rcs $(LIB_DIR)/libcthread.a $(BIN_DIR)/*.o 
	@echo " "

clean:
	@echo "Arquivos da biblioteca e binarios de teste sendo apagados"
	rm -rf $(LIB_DIR)/*.a $(BIN_DIR)/lib.o ./testes/*.run
	@echo " "
