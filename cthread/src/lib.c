
#include <stdio.h>
#include <string.h>
#include "../include/support.h"
#include "../include/cthread.h"
#include "../include/cdata.h"
#include <ucontext.h>

#define ALTA_PRIORIDADE 0
#define MEDIA_PRIORIDADE 1
#define BAIXA_PRIORIDADE 2

#define FROM_END 0;
#define FROM_YIELD 1;
#define FROM_JOIN 2;

// cwait
#define SUCCESS 0;
#define ERROR_NULL_PARAM -1
#define ERROR_CANT_BLOCK_THREAD -2
#define ERROR_CANT_APPEND_THREAD -3


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

	// se ta vindo do cyield
	if (runningThread->whereFrom == FROM_YIELD)
	{
		// poem a thread na sua correspondente fila de aptos
		switch (runningThread->prio)
		{
		case BAIXA_PRIORIDADE:
			if (AppendFila2(pfilaAptoBaixa, (void *)runningThread) != 0)
				printf(" func yield: Erro ao inserir a thread na fila de aptos");
			else
				FirstFila2(pfilaAptoBaixa);
			break;
		case MEDIA_PRIORIDADE:
			if (AppendFila2(pfilaAptoMedia, (void *)runningThread) != 0)
				printf(" func yield: Erro ao inserir a thread na fila de aptos");
			else
				FirstFila2(pfilaAptoMedia);
			break;
		case ALTA_PRIORIDADE:
			if (AppendFila2(pfilaAptoAlta, (void *)runningThread) != 0)
				printf(" func yield: Erro ao inserir a thread na fila de aptos");
			else
				FirstFila2(pfilaAptoAlta);
			break;
		}
		runningThread->whereFrom = FROM_END;
	}
	else if(runningThread->whereFrom == FROM_END)
	{ // até entao: se ta vindo sem yield é pq ta terminando
		if (AppendFila2(pfilaTerminado, (void *)runningThread) != 0)
			printf(" func scheduler: Erro ao inserir a thread na fila de terminados");
	} else if(runningThread->whereFrom == FROM_JOIN){
		// veio do join: poem a thread em bloqueado
		if (AppendFila2(pfilaBloqueado, (void *)runningThread) != 0)
			printf(" func scheduler: Erro ao inserir a thread na fila de bloqueados");

		// procurar nos bloqueados quem ta com o tid == runningThread -> isWaitingMe
		// por essa thread na sua respective fila de aptos

		runningThread->whereFrom = FROM_END;
	}


	TCB_t *thread = (TCB_t *)GetAtIteratorFila2(pfilaAptoAlta); // thread que vai ser a proxima running
	// tenta tirar primeiro da alta, depois media depois baixa
	if (thread == NULL)
	{
		thread = (TCB_t *)GetAtIteratorFila2(pfilaAptoMedia);
		if (thread == NULL)
		{
			thread = (TCB_t *)GetAtIteratorFila2(pfilaAptoBaixa);
			if (thread == NULL)
				printf("eita geovanaa");
			else
			{
				DeleteAtIteratorFila2(pfilaAptoBaixa);
			}
		}
		else
		{
			DeleteAtIteratorFila2(pfilaAptoMedia);
		}
	}
	else
	{
		DeleteAtIteratorFila2(pfilaAptoAlta);
	}

	thread->state = PROCST_EXEC;
	runningThread = thread;
	setcontext(&(runningThread->context));


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

	newThread->whereFrom = FROM_END;

	//cria um contexto, passas a função start (parametro da ccreate), é 1 parametro pra ela que tá em arg
	makecontext(&(newThread->context), (void (*)(void))start, 1, arg);
	
	//append na fila de aptos
	switch (prio)
	{
	case BAIXA_PRIORIDADE:
		if (AppendFila2(pfilaAptoBaixa, (void *)newThread) != 0)
			printf("Erro ao inserir a nova thread na fila de aptos");
		else
			FirstFila2(pfilaAptoBaixa);
		break;
	case MEDIA_PRIORIDADE:
		if (AppendFila2(pfilaAptoMedia, (void *)newThread) != 0)
			printf("Erro ao inserir a nova thread na fila de aptos");
		else
			FirstFila2(pfilaAptoMedia);
		break;
	case ALTA_PRIORIDADE:
		if (AppendFila2(pfilaAptoAlta, (void *)newThread) != 0)
			printf("Erro ao inserir a nova thread na fila de aptos");
		else
			FirstFila2(pfilaAptoAlta);
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
	printf("chegando na cyiled");

	//muda o estado para apto e coloca em uma das filas
	runningThread->state = PROCST_APTO;
	runningThread->whereFrom = FROM_YIELD;

	// salva o contexto na thread

	if (getcontext(&(runningThread->context)) == -1)
	{
		printf("func yield: Erro ao salvar o contexto da thread atual");
		return -1;
	}

	//se a thread chamou essa função agora (isto é, não está retornando sua execução) o scheduler é chamado
	if (runningThread->whereFrom == FROM_YIELD)
		scheduler();

	return 0;
}

int cjoin(int tid)
{
	// procurar em todas as filas uma thread com a thread->tid = tid
	// se ela tiver isWaitingMe != 1, retorna código de erro (já tem uma thread esperando ela)
	// se não, poem a runningThread na fila de bloqueados, thread->isWaitingMe = runningThread->tid
	
	
	
	return -1;
}

int csem_init(csem_t *sem, int count)
{
	return -1;
}

int cwait(csem_t *sem)
{
	if(sem == NULL) {
			return ERROR_NULL_PARAM;
	}

	int status = 0;

	 if(sem->count < 0) {
		getcontext(&(runningThread->context));
		
		/* 
			status = funcao que bloqueia thread
		*/

		if(status < 0) {
					return ERROR_CANT_BLOCK_THREAD;
		}

		status = AppendFila2(sem->fila, runningThread);
		
		if (status != 0) {
			return ERROR_CANT_APPEND_THREAD;
		}
	}
	
	sem->count--;
	return SUCCESS;	
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
