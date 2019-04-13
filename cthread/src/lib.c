
#include <stdio.h>
#include <string.h>
#include "../include/support.h"
#include "../include/cthread.h"
#include "../include/cdata.h"
#include <ucontext.h>

// essa é a thread que está em execução no momento
TCB_t runningThread;

// thread main
TBC_t mainThread;

// scheduler: toda thread quando finaliza aponta pra esse contexto. Ele vai ser o responsável
// por tirar uma thread da fila de aptos e por em execução
ucontext_t scheduler;


int initialized = 0;
int serialId;

void scheduler(){
    // troca o processo em execução e coordena as filas
};

void initQueues(){
    initialized = 1;
    // pega o contexto atual pra iniciar o scheduler
    getcontext(&scheduler);
    scheduler.uc_link = mainThread; // o scheduler em tese só acaba quando não tem nada nas filas pra escalonar, então volta pra main
    scheduler.uc_stack.ss_sp = malloc(sizeof(SIGSTKSZ));
    scheduler.uc_stack.ss_size = SIGSTKSZ;

    makecontext(&(newThread->context), (void(*)(void))&scheduler, 1, arg);
    /// inicializa as filas
    // filas apto baixa media e alta prioridade
    // fila terminado
    // fila bloqueado

    mainThread = malloc(sizeof(TCB_t));
    mainThread.tid = 0; // thread main tem o tid = 0
    mainThread.state = PROCST_EXEC; // main thread ta executando mesmo
    mainThread.prio = 2; // main tem baixa prioridade
    getcontext(&(mainThread.context));
    //descobrir se precisa alocar coisas pra pilha da main thread?
}

int ccreate (void* (*start)(void*), void *arg, int prio) {

	if(!initialized)
        initQueues(); // inicializa as filas (todas)

	TCB_t* newThread = malloc(sizeof(TCB_t));
	newThread->tid = ++serialId;
	newThread->state = PROCST_APTO;
	newThread->prio = prio;
	getcontext(&(newThread->context)); // pega o contexto(atual) e poem no contexto da thread nova
	// quando cada thread termina ela vai pro scheduler que daí decide qual fila por
	newThread->context.uc_link = &scheduler;


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


