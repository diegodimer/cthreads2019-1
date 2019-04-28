
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/support.h"
#include "../include/cthread.h"
#include "../include/cdata.h"
#include <ucontext.h>

#define ALTA_PRIORIDADE 0
#define MEDIA_PRIORIDADE 1
#define BAIXA_PRIORIDADE 2

#define FROM_END 0
#define FROM_YIELD 1
#define FROM_JOIN 2
#define FROM_CWAIT 3

// cidentify
#define ERROR_MINIMUM_SIZE_NOT_ENOUGH -1
#define MINIMUM_STRING_SIZE 69
#define SUCCESS 0

// cwait
#define ERROR_NULL_PARAM -1
#define ERROR_CANT_BLOCK_THREAD -2
#define ERROR_CANT_APPEND_THREAD -3

//csignal
#define ERROR_REMOVE_THREAD -1
#define ERROR_APPEND_THREAD -2

// essa é a thread que está em execução no momento
TCB_t *runningThread;

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

void setIteratorToFirst()
{
	FirstFila2(pfilaAptoAlta);
	FirstFila2(pfilaAptoMedia);
	FirstFila2(pfilaAptoBaixa);
	FirstFila2(pfilaBloqueado);
};

void scheduler()
{
	// seta iteradores das filas pra primeiro
    setIteratorToFirst();

    if (runningThread->whereFrom == FROM_YIELD)
    {
        // se veio da cyield -> thread volta pra sua fila de aptos
        runningThread->state = PROCST_APTO;
        switch (runningThread->prio)
        {
        case BAIXA_PRIORIDADE:
            if (AppendFila2(pfilaAptoBaixa, (void *)runningThread) != 0)
                printf(" func yield: Erro ao inserir a thread na fila de aptos\n");
            else
                FirstFila2(pfilaAptoBaixa);
            break;
        case MEDIA_PRIORIDADE:
            if (AppendFila2(pfilaAptoMedia, (void *)runningThread) != 0)
                printf(" func yield: Erro ao inserir a thread na fila de aptos\n");
            else
                FirstFila2(pfilaAptoMedia);
            break;
        case ALTA_PRIORIDADE:
            if (AppendFila2(pfilaAptoAlta, (void *)runningThread) != 0)
                printf(" func yield: Erro ao inserir a thread na fila de aptos\n");
            else
                FirstFila2(pfilaAptoAlta);
            break;
        }
        // faz com que quando o contexto da thread voltar ela não caia no scheduler
        runningThread->whereFrom = FROM_END;
    }
    else if (runningThread->whereFrom == FROM_JOIN)
    {

        runningThread->state = PROCST_BLOQ;
        // veio do join: poem a thread em bloqueado
        if (AppendFila2(pfilaBloqueado, (void *)runningThread) != 0)
            printf(" func scheduler: Erro ao inserir a thread na fila de bloqueados\n");
        else
        {
            FirstFila2(pfilaBloqueado);
        }
        // faz com que quando o contexto da thread voltar ela não caia no scheduler
        runningThread->whereFrom = FROM_END;
    }
    else if (runningThread->whereFrom == FROM_END)
    {
        // se não veio pelo yield nem join, é pq ta terminando
        //poem na fila de terminados
        runningThread->state = PROCST_TERMINO;
        if (AppendFila2(pfilaTerminado, (void *)runningThread) != 0)
            printf(" func scheduler: Erro ao inserir a thread na fila de terminados\n");

		// desbloqueia todas threads que estavam esperando a runninghtread terminar
        TCB_t *blockedThread;

        do
        {

            blockedThread = (TCB_t *)GetAtIteratorFila2(pfilaBloqueado);

            if (blockedThread != NULL && blockedThread->tid == runningThread->isWaitingMe) // se a thread bloqueada esta esperando pela running
            {

                blockedThread->state = PROCST_APTO;

                switch (blockedThread->prio)
                {
                case BAIXA_PRIORIDADE:
                    if (AppendFila2(pfilaAptoBaixa, (void *)blockedThread) != 0)
                        printf(" func scheduler/from_end: Erro ao inserir a thread na fila de aptos\n");
                    else
                        FirstFila2(pfilaAptoBaixa);
                    break;
                case MEDIA_PRIORIDADE:
                    if (AppendFila2(pfilaAptoMedia, (void *)blockedThread) != 0)
                        printf(" func scheduler/from_end: Erro ao inserir a thread na fila de aptos\n");
                    else
                        FirstFila2(pfilaAptoMedia);
                    break;
                case ALTA_PRIORIDADE:
                    if (AppendFila2(pfilaAptoAlta, (void *)blockedThread) != 0)
                        printf(" func scheduler/from_end: Erro ao inserir a thread na fila de aptos\n");
                    else
                        FirstFila2(pfilaAptoAlta);
                    break;
                }
                DeleteAtIteratorFila2(pfilaBloqueado);
                FirstFila2(pfilaBloqueado);
            }

        }
        while (!NextFila2(pfilaBloqueado));
    } else if (runningThread->whereFrom == FROM_CWAIT)
    {

        runningThread->state = PROCST_BLOQ;
        // veio do cwait: poem a thread em bloqueado
        if (AppendFila2(pfilaBloqueado, (void *)runningThread) != 0)
            printf(" func scheduler: Erro ao inserir a thread na fila de bloqueados\n");
        else
        {
            FirstFila2(pfilaBloqueado);
        }
        // faz com que quando o contexto da thread voltar ela não caia no scheduler
        runningThread->whereFrom = FROM_END;
    }

    // o que acontece pra todas -> seleciona uma da fila de aptos (da maior prioridade possivel)
    // tira ela (delete) e atualiza a runningthread;

    TCB_t *thread = (TCB_t *)GetAtIteratorFila2(pfilaAptoAlta); // thread que vai ser a proxima running

    // tenta tirar primeiro da alta, depois media depois baixa
    if (thread == NULL)
    {
        thread = (TCB_t *)GetAtIteratorFila2(pfilaAptoMedia);
        if (thread == NULL)
        {
            thread = (TCB_t *)GetAtIteratorFila2(pfilaAptoBaixa);
            if (thread == NULL)
                printf("eita geovanaa\n");
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
    // seta o contexto pra thread que está pra rodar

    setcontext(&(thread->context));

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
		printf("Erro criando fila apto alta prioridade!\n");

	pfilaAptoMedia = &filaAptoMedia;
	if (CreateFila2(pfilaAptoMedia) != 0)
		printf("Erro criando fila AptoMedia!\n");

	pfilaAptoBaixa = &filaAptoBaixa;
	if (CreateFila2(pfilaAptoBaixa) != 0)
		printf("Erro criando fila AptoBaixa!\n");

	pfilaBloqueado = &filaBloqueado;
	if (CreateFila2(pfilaBloqueado) != 0)
		printf("Erro criando fila Bloqueado!\n");

	pfilaTerminado = &filaTerminado;
	if (CreateFila2(pfilaTerminado) != 0)
		printf("Erro criando fila Terminado!\n");

	setIteratorToFirst();

	// thread main
    TCB_t *mainThread;
	//main thread: alocação dela
	mainThread = malloc(sizeof(TCB_t));
	mainThread->tid = 0;				 // thread main tem o tid = 0
	mainThread->state = PROCST_EXEC;	 // main thread ta executando
	mainThread->prio = BAIXA_PRIORIDADE; // main tem baixa prioridade
	mainThread->context.uc_link = NULL;
	mainThread->context.uc_stack.ss_size = SIGSTKSZ;
	mainThread->context.uc_stack.ss_sp = malloc(sizeof(SIGSTKSZ));
	mainThread->isWaitingMe = -1;
	mainThread->whereFrom = FROM_END;
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
	newThread->isWaitingMe = -1;

	//cria um contexto, passas a função start (parametro da ccreate), é 1 parametro pra ela que tá em arg
	makecontext(&(newThread->context), (void (*)(void))start, 1, arg);

	//append na fila de aptos
	switch (prio)
	{
	case BAIXA_PRIORIDADE:
		if (AppendFila2(pfilaAptoBaixa, (void *)newThread) != 0)
			printf("Erro ao inserir a nova thread na fila de aptos\n");
		else
			FirstFila2(pfilaAptoBaixa);
		break;
	case MEDIA_PRIORIDADE:
		if (AppendFila2(pfilaAptoMedia, (void *)newThread) != 0)
			printf("Erro ao inserir a nova thread na fila de aptos\n");
		else
			FirstFila2(pfilaAptoMedia);
		break;
	case ALTA_PRIORIDADE:
		if (AppendFila2(pfilaAptoAlta, (void *)newThread) != 0)
			printf("Erro ao inserir a nova thread na fila de aptos\n");
		else
			FirstFila2(pfilaAptoAlta);
		break;
	}

	return newThread->tid;
}

int csetprio(int tid, int prio)
{
	if (tid != (int)NULL || prio > BAIXA_PRIORIDADE || prio < ALTA_PRIORIDADE)
		return -1;
	else
	{
		runningThread->prio = prio;
		return 0;
	}
}

int cyield(void)
{

	//muda o estado para apto e coloca em uma das filas

	runningThread->whereFrom = FROM_YIELD;

	// salva o contexto na thread

	if (getcontext(&(runningThread->context)) == -1)
	{
		printf("func yield: Erro ao salvar o contexto da thread atual\n");
		return -1;
	}

	//se a thread chamou essa função agora (isto é, não está retornando sua execução) o scheduler é chamado
	if (runningThread->whereFrom == FROM_YIELD)
		scheduler();

	return 0;
}

int cjoin(int tid)
{
	int retorno;

	//procura a thread com tid
	TCB_t *toBeWaited;

	setIteratorToFirst();

	int found = 0;

	do
	{
		toBeWaited = (TCB_t *)GetAtIteratorFila2(pfilaAptoAlta);
		if (toBeWaited != NULL && toBeWaited->tid == tid)
		{
			found = 1;
		}
	} while (!NextFila2(pfilaAptoAlta) && !found);
	if (!found)
	{
		// nao achou > proxima fila
		do
		{
			toBeWaited = (TCB_t *)GetAtIteratorFila2(pfilaAptoMedia);
			if (toBeWaited != NULL && toBeWaited->tid == tid)
			{
				found = 1;
			}
		} while (!NextFila2(pfilaAptoMedia) && !found);
		if (!found)
		{
			// nao achou > proxima fila
			do
			{
				toBeWaited = (TCB_t *)GetAtIteratorFila2(pfilaAptoBaixa);
				if (toBeWaited != NULL && toBeWaited->tid == tid)
				{
					found = 1;
				}
			} while (!NextFila2(pfilaAptoBaixa) && !found);
			if (!found)
			{
				do
				{
					toBeWaited = (TCB_t *)GetAtIteratorFila2(pfilaBloqueado);
					if (toBeWaited != NULL && toBeWaited->tid == tid)
					{
						found = 1;
					}
				} while (!NextFila2(pfilaBloqueado) && !found);
				if (!found)
				{
					// nao achou nao tem
					retorno = -1;
				}
			}
		}
	}

	if (found)
	{
		// achou a thread com o tid buscado
		if (toBeWaited->isWaitingMe != -1) // ja tem alguem esperando essa thread
		{
			retorno = -2;
		}
		else
		{
			// tudo bem pode esperar
			// agora a runningthread ta esperando a thread acabar
			toBeWaited->isWaitingMe = runningThread->tid;
			// ela vai cair no if pra fazer o scheduler e quando voltar não vai (ja fez)
			runningThread->whereFrom = FROM_JOIN;
			// salva o contexto da thread que ta rodando (ela vai ser bloqueada)
			getcontext(&(runningThread->context));
			//quando a execução dela voltar ela volta aqui, mas ja passou pelo scheduler
			// que mudou o whereFrom dela
			if ((runningThread->whereFrom) == FROM_JOIN)
			{
				scheduler();
			}
			else
			{
				retorno = 0;
			}
		}
	}

	return retorno;
}

int csem_init(csem_t *sem, int count)
{
    sem->count = count;

    sem->fila = (PFILA2) malloc(sizeof(PFILA2));

    if(CreateFila2(sem->fila) != 0)
        return -1;
    else
        FirstFila2(sem->fila);

    return 0;
}


int cwait(csem_t *sem)
{
    if(sem == NULL)
    {
        return ERROR_NULL_PARAM;
    }

    int status = 0;

    if(sem->count <= 0)
    {
        runningThread->whereFrom = FROM_CWAIT;
        status = AppendFila2(sem->fila, runningThread);
        getcontext(&(runningThread->context));

        if (status != 0)
        {
            return ERROR_CANT_APPEND_THREAD;
        }


        if(runningThread->whereFrom == FROM_CWAIT)
            scheduler();
    }
    sem->count--;
    return SUCCESS;
}

int csignal(csem_t *sem)
{
    //incrementa o contador para indicar que o recurso foi liberado
    sem->count += 1;

    //procura uma thread de alta prioridade para desalocar da fila do semáforo
    int found =0;
    TCB_t *ThreadFromSem;
    FirstFila2(sem->fila);

    do
    {
        ThreadFromSem = (TCB_t *)GetAtIteratorFila2(sem->fila);
        if (ThreadFromSem != NULL && ThreadFromSem->prio == ALTA_PRIORIDADE)
        {
            found = 1;
            DeleteAtIteratorFila2(sem->fila);
        }
    }
    while(!NextFila2(sem->fila) && !found);
    if (!found)
    {
        //se não houver uma de alta prioridade procura uma thread de media prioridade na fila do semáforo
        FirstFila2(sem->fila);
        do
        {
            ThreadFromSem = (TCB_t *)GetAtIteratorFila2(sem->fila);
            if (ThreadFromSem != NULL && ThreadFromSem->prio == MEDIA_PRIORIDADE)
            {
                found = 1;
                DeleteAtIteratorFila2(sem->fila);
            }
        }
        while(!NextFila2(sem->fila) && !found);
        if(!found)
        {
            // se não houver de media prioridade procura uma de baixa prioridade na fila do semáforo
            FirstFila2(sem->fila);
            do
            {
                ThreadFromSem = (TCB_t *)GetAtIteratorFila2(sem->fila);
                if(ThreadFromSem != NULL && ThreadFromSem->prio == BAIXA_PRIORIDADE)
                {
                    found =1;
                    DeleteAtIteratorFila2(sem->fila);
                }
            }
            while(!NextFila2(sem->fila) && !found);
				//printf(" Nao tem nenhuma thread na fila do semaforo \n");

        }
    }
    // coloca a thread em alguma fila de aptos

    if(found)
    {
        ThreadFromSem->state = PROCST_APTO;
        setIteratorToFirst();
        switch (ThreadFromSem->prio)
        {
        case BAIXA_PRIORIDADE:
            if (AppendFila2(pfilaAptoBaixa, (void *)ThreadFromSem) != 0)
            {
                printf(" func csignal: Erro ao inserir a thread na fila de aptos\n");
                return ERROR_APPEND_THREAD;
            }
            else
                FirstFila2(pfilaAptoBaixa);
            break;
        case MEDIA_PRIORIDADE:
            if (AppendFila2(pfilaAptoMedia, (void *)ThreadFromSem) != 0)
            {
                printf(" func csignal: Erro ao inserir a thread na fila de aptos\n");
                return ERROR_APPEND_THREAD;
            }
            else
                FirstFila2(pfilaAptoMedia);
            break;
        case ALTA_PRIORIDADE:
            if (AppendFila2(pfilaAptoAlta, (void *)ThreadFromSem) != 0)
            {
                printf(" func csignal: Erro ao inserir a thread na fila de aptos\n");
                return ERROR_APPEND_THREAD;
            }
            else
                FirstFila2(pfilaAptoAlta);
            break;
        }
        // remove a thread da fila dos bloqueados
        FirstFila2(pfilaBloqueado);

        int tid_ThreadFromSem = ThreadFromSem->tid;
        found=0;

        do
        {
            ThreadFromSem = (TCB_t *)GetAtIteratorFila2(pfilaBloqueado);
            if (ThreadFromSem != NULL && ThreadFromSem->tid == tid_ThreadFromSem)
            {
                found = 1;
                DeleteAtIteratorFila2(pfilaBloqueado);
            }
        }
        while(!NextFila2(pfilaBloqueado) && !found);
    }
    return SUCCESS;
}

int cidentify(char *name, int size)
{

	if (size < MINIMUM_STRING_SIZE)
	{
		return ERROR_MINIMUM_SIZE_NOT_ENOUGH;
	}

	strncpy(name, "Afonso Ferrer - 252856\nDiego Dimer - 287690\nEduardo Paim - 277322", size);
	return SUCCESS;
}
