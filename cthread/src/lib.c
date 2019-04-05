
#include <stdio.h>
#include <string.h>
#include "../include/support.h"
#include "../include/cthread.h"
#include "../include/cdata.h"
#include <ucontext.h>

TCB_t runningThread;
int initialized = 0;
int serialId;

int ccreate (void* (*start)(void*), void *arg, int prio) {
	initQueues(); // inicializa as filas (todas)
	
	TCB_t* newThread = malloc(sizeof(TCB_t));
	newThread->tid = ++serialId;
	newThread->state = PROCST_APTO;
	newThread->prio = prio;
	getcontext(&(newThread->context)); // pega o contexto(atual) e poem no contexto da thread nova
	// precisa pensar no que poem aqui
	// newThread->context.uc_link = 
	// uc_link aponta pro contexto que vai rolar quando essa thread
	// que eu to definindo aqui acabar

	newThread->context.uc_stack.ss_sp = malloc(sizeof(SIGSTKSZ));
	newThread->context.uc_stack.ss_size = SIGSTKSZ;
	// @todo: ver se esse SIGSTKSZ funciona de vdd (im not sure)

	//cria um contexto, passas a função start (parametro da ccreate), é 1 parametro pra ela que tá em arg
	makecontext(&(newThread->context), (void(*)(void))start, 1, arg);
	
	//append na fila de aptos (ainda nao foi feita)
	
	return -1;
}

int csetprio(int tid, int prio) {
	return -1;
}

int cyield(void) {
	return -1;
}

int cjoin(int tid) {
	return -1;
}

int csem_init(csem_t *sem, int count) {
	return -1;
}

int cwait(csem_t *sem) {
	return -1;
}

int csignal(csem_t *sem) {
	return -1;
}

int cidentify (char *name, int size) {
	strncpy (name, "Sergio Cechin - 2017/1 - Teste de compilacao.", size);
	return 0;
}


