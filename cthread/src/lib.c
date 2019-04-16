
#include <stdio.h>
#include <string.h>
#include "../include/support.h"
#include "../include/cthread.h"
#include "../include/cdata.h"
#include <ucontext.h>

#define ALTA_PRIORIDADE 0
#define MEDIA_PRIORIDADE 1
#define BAIXA_PRIORIDADE 2
 
// essa é a thread que está em execução no momento
TCB_t *runningThread;

// thread main
TCB_t *mainThread;

// scheduler: toda thread quando finaliza aponta pra esse contexto. Ele vai ser o responsável
// por tirar uma thread da fila de aptos e por em execução
ucontext_t schedulerC;

//fila de alta prioridade
PFILA2 iterador_alta_prio; //esse ponteiro tem que ser inicializado (com CreateFila2()) em initQueues() ou ccreate(), não sei dizer qual
//media prioridade
PFILA2 iterador_media_prio;
//baixa prioridade
PFILA2 iterador_baixa_prio;

int initialized = 0;
int serialId;

void scheduler(){
	printf("scheduler running");
    // troca o processo em execução e coordena as filas
};

void initQueues(){
	
    initialized = 1;
    // pega o contexto atual pra iniciar o scheduler
    getcontext(&schedulerC);
    schedulerC.uc_link = NULL; // o scheduler em tese só acaba quando não tem nada nas filas pra escalonar, então volta pra main
    schedulerC.uc_stack.ss_sp = (char *)malloc(sizeof(SIGSTKSZ));
    schedulerC.uc_stack.ss_size = SIGSTKSZ;

    makecontext(&schedulerC, (void(*)(void))scheduler, 0);
    /// inicializa as filas
    // filas apto baixa media e alta prioridade
    // fila terminado
    // fila bloqueado

    mainThread = malloc(sizeof(TCB_t));
    mainThread->tid = 0; // thread main tem o tid = 0
    mainThread->state = PROCST_APTO; // main thread ta executando mesmo
    mainThread->prio = 2; // main tem baixa prioridade
	mainThread->context.uc_link = NULL;
	mainThread->context.uc_stack.ss_size = SIGSTKSZ;
	mainThread->context.uc_stack.ss_sp = malloc(sizeof(SIGSTKSZ));
    getcontext(&(mainThread->context));
	runningThread = mainThread;

};


int ccreate (void* (*start)(void*), void *arg, int prio) {

	if(!initialized)
        initQueues(); // inicializa as filas (todas)

	TCB_t* newThread = malloc(sizeof(TCB_t));
	newThread->tid = ++serialId;
	newThread->state = PROCST_APTO;
	newThread->prio = prio;
	getcontext(&(newThread->context)); // pega o contexto(atual) e poem no contexto da thread nova
	// quando cada thread termina ela vai pro scheduler que daí decide qual fila por
	newThread->context.uc_link = &schedulerC;

	newThread->context.uc_stack.ss_sp = malloc(sizeof(SIGSTKSZ));
	newThread->context.uc_stack.ss_size = SIGSTKSZ;
	
	//cria um contexto, passas a função start (parametro da ccreate), é 1 parametro pra ela que tá em arg
	makecontext(&(newThread->context), (void(*)(void))start, 1, arg);

	//append na fila de aptos (ainda nao foi feita)
	return newThread->tid;
}


int csetprio(int tid, int prio) {
	return -1;
}

int cyield(void) {
	
	//salva o contexto atual da thread e muda o estado para apto
	runningThread->state = PROCST_APTO;
	getcontext(runningThread->context);
	
	//coloca o processo atual na fila de aptos
	if(runningThread->prio=ALTA_PRIORIDADE){
		//cria um novo nodo na fila;
		NODE2 novo_elemento = malloc(sizeof(NODE2));
		// adiciona o processo atual ao nodo
		novo_elemento.next = *runningThread;
		
		//coloca o novo nodo no ultimo lugar da fila
		AppendFila2(iterador_alta_prio, &novo_elemento);
		
		
		
		
		
	}	
	
	
	
	
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


