
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

//****************************//
//			Filas			   //
//****************************//
FILA2 filaAptoBaixa;
FILA2 filaAptoMedia;
FILA2 filaAptoAlta;
FILA2 filaBloqueado;
FILA2 filaTerminado;

PFILA2 pfilaAptoBaixa;
PFILA2 pfilaAptoMedia;
PFILA2 pfilaAptoAlta;
PFILA2 pfilaBloqueado;
PFILA2 pfilaTerminado;
//////////////////////////////

int initialized = 0;
int serialId;

void scheduler()
{
	printf("scheduler running");
	// troca o processo em execução e coordena as filas
};

void initQueues()
{

	initialized = 1;
	// pega o contexto atual pra iniciar o scheduler
	getcontext(&schedulerC);
	schedulerC.uc_link = NULL;
	schedulerC.uc_stack.ss_sp = (char *)malloc(sizeof(SIGSTKSZ));
	schedulerC.uc_stack.ss_size = SIGSTKSZ;
	// aqui não tem o problema do setcontext pq o makecontext faz o scheduler ir pra função dele
	makecontext(&schedulerC, (void (*)(void))scheduler, 0);

	/// inicializa as filas
	pfilaAptoAlta = &filaAptoAlta;
	if (CreateFila2(pfilaAptoAlta) != 0)
		printf("Erro criando fila apto alta prioridade!");
	
	pfilaAptoMedia = &filaAptoMedia;
	if (CreateFila2(pfilaAptoMedia) != 0)
		printf("Erro criando fila AptoMedia!");

	pfilaAptoBaixa = &filaAptoBaixa;
	if (CreateFila2(pfilaAptoBaixa) != 0)
		printf("Erro criando fila AptoBaixa!");

	pfilaBloqueado = &filaBloqueado;
	if (CreateFila2(pfilaBloqueado) != 0)
		printf("Erro criando fila Bloqueado!");

	pfilaTerminado = &filaTerminado;
	if (CreateFila2(pfilaTerminado) != 0)
		printf("Erro criando fila Terminado!");

	FirstFila2(pfilaAptoAlta);
	FirstFila2(pfilaAptoMedia);
	FirstFila2(pfilaAptoBaixa);
	FirstFila2(pfilaBloqueado);
	FirstFila2(pfilaTerminado);

	//main thread: alocação dela
	mainThread = malloc(sizeof(TCB_t));
	mainThread->tid = 0;				 // thread main tem o tid = 0
	mainThread->state = PROCST_EXEC;	 // main thread ta executando
	mainThread->prio = BAIXA_PRIORIDADE; // main tem baixa prioridade
	mainThread->context.uc_link = NULL;
	mainThread->context.uc_stack.ss_size = SIGSTKSZ;
	mainThread->context.uc_stack.ss_sp = malloc(sizeof(SIGSTKSZ));
	runningThread = mainThread;
	getcontext(&(mainThread->context));
};

int ccreate(void *(*start)(void *), void *arg, int prio)
{

	if (!initialized)
		initQueues(); // inicializa as filas (todas)

	TCB_t *newThread = malloc(sizeof(TCB_t));
	newThread->tid = ++serialId;
	newThread->state = PROCST_APTO;
	newThread->prio = prio;
	getcontext(&(newThread->context)); // pega o contexto(atual) e poem no contexto da thread nova
	// quando cada thread termina ela vai pro scheduler que daí decide qual fila por
	newThread->context.uc_link = &schedulerC;

	newThread->context.uc_stack.ss_sp = malloc(sizeof(SIGSTKSZ));
	newThread->context.uc_stack.ss_size = SIGSTKSZ;

	//cria um contexto, passas a função start (parametro da ccreate), é 1 parametro pra ela que tá em arg
	makecontext(&(newThread->context), (void (*)(void))start, 1, arg);

	//append na fila de aptos
	switch (prio)
	{
	case BAIXA_PRIORIDADE:
		if (AppendFila2(pfilaAptoBaixa, (void *)newThread) != 0)
			printf("Erro ao inserir a nova thread na fila de aptos");
		break;
	case MEDIA_PRIORIDADE:
		if (AppendFila2(pfilaAptoMedia, (void *)newThread) != 0)
			printf("Erro ao inserir a nova thread na fila de aptos");
		break;
	case ALTA_PRIORIDADE:
		if (AppendFila2(pfilaAptoAlta, (void *)newThread) != 0)
			printf("Erro ao inserir a nova thread na fila de aptos");
		break;
	}

	return newThread->tid;
}

int csetprio(int tid, int prio)
{
	return -1;
}

int cyield(void)
{

	//salva o contexto atual da thread e muda o estado para apto
	runningThread->state = PROCST_APTO;
	getcontext(runningThread->context);

	//coloca o processo atual na fila de aptos
	if (runningThread->prio = ALTA_PRIORIDADE)
	{
		//cria um novo nodo na fila;
		NODE2 novo_elemento = malloc(sizeof(NODE2));
		// adiciona o processo atual ao nodo
		novo_elemento.next = *runningThread;

		//coloca o novo nodo no ultimo lugar da fila
		AppendFila2(iterador_alta_prio, &novo_elemento);
	}

	return -1;
}

int cjoin(int tid)
{
	return -1;
}

int csem_init(csem_t *sem, int count)
{
	return -1;
}

int cwait(csem_t *sem)
{
	return -1;
}

int csignal(csem_t *sem)
{
	return -1;
}

int cidentify(char *name, int size)
{
	strncpy(name, "Sergio Cechin - 2017/1 - Teste de compilacao.", size);
	return 0;
}
