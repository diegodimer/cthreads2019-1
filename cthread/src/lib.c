
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

/**
 * @brief  Seta de uma vez só todos os iteradores de todas as filas para o primeiro elemento
 * @note   
 * @retval None
 */
void setIteratorToFirst()
{
	FirstFila2(pfilaAptoAlta);
	FirstFila2(pfilaAptoMedia);
	FirstFila2(pfilaAptoBaixa);
	FirstFila2(pfilaBloqueado);
};

/**
 * @brief  Função responsável pela manutenção das filas da biblioteca. É para onde as thread em execução vai quando precisa ser bloqueada, terminada ou ceder a CPU.
 * @note   
 * @retval None
 */
void scheduler()
{

	// seta iteradores das filas pra primeiro
    setIteratorToFirst();

    if (runningThread->state == PROCST_APTO)
    {
        // se veio da cyield -> thread volta pra sua fila de aptos
        switch (runningThread->prio)
        {
        case BAIXA_PRIORIDADE:
            if (AppendFila2(pfilaAptoBaixa, (void *)runningThread) != 0)
                exit(-1);
            else
                FirstFila2(pfilaAptoBaixa);
            break;
        case MEDIA_PRIORIDADE:
            if (AppendFila2(pfilaAptoMedia, (void *)runningThread) != 0)
                exit(-1);
            else
                FirstFila2(pfilaAptoMedia);
            break;
        case ALTA_PRIORIDADE:
            if (AppendFila2(pfilaAptoAlta, (void *)runningThread) != 0)
                exit(-1);
            else
                FirstFila2(pfilaAptoAlta);
            break;
        }
        // faz com que quando o contexto da thread voltar ela não caia no scheduler
        
    }
    else if (runningThread->state == PROCST_BLOQ)
    {
        // veio do join: poem a thread em bloqueado
        if (AppendFila2(pfilaBloqueado, (void *)runningThread) != 0)
            exit(-1);
        else
        {
            FirstFila2(pfilaBloqueado);
        }
    }
    else if (runningThread->state == PROCST_EXEC)
    {
        // se a thread estava em execução e chega no scheduler significa que ela acabou
        runningThread->state = PROCST_TERMINO;
        if (AppendFila2(pfilaTerminado, (void *)runningThread) != 0)
            exit(-1);

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
                        exit(-1);
                    else
                        FirstFila2(pfilaAptoBaixa);
                    break;
                case MEDIA_PRIORIDADE:
                    if (AppendFila2(pfilaAptoMedia, (void *)blockedThread) != 0)
                        exit(-1);
                    else
                        FirstFila2(pfilaAptoMedia);
                    break;
                case ALTA_PRIORIDADE:
                    if (AppendFila2(pfilaAptoAlta, (void *)blockedThread) != 0)
                        exit(-1);
                    else
                        FirstFila2(pfilaAptoAlta);
                    break;
                }
                DeleteAtIteratorFila2(pfilaBloqueado);
                FirstFila2(pfilaBloqueado);
            }

        }
        while (!NextFila2(pfilaBloqueado));
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
                exit(-1);
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
    if(setcontext(&(thread->context)) == -1){
        exit(-1);
    };

};

/**
 * @brief  Inicializa todas as filas e lida com a thread main, alocando uma estrutura do tipo TCB_t para ela e criando um contexto.
 * @note   
 * @retval None
 */
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
		exit(-1);

	pfilaAptoMedia = &filaAptoMedia;
	if (CreateFila2(pfilaAptoMedia) != 0)
		exit(-1);

	pfilaAptoBaixa = &filaAptoBaixa;
	if (CreateFila2(pfilaAptoBaixa) != 0)
		exit(-1);

	pfilaBloqueado = &filaBloqueado;
	if (CreateFila2(pfilaBloqueado) != 0)
		exit(-1);

	pfilaTerminado = &filaTerminado;
	if (CreateFila2(pfilaTerminado) != 0)
		exit(-1);

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
	runningThread = mainThread;
	getcontext(&(mainThread->context));
};

/**
 * @brief  Criação de uma thread
 * @note   
 * @param void *(*start)(void *): ponteiro para função que a thread executará
 * @param *arg: parâmetro a ser usado por essa função
 * @param prio: prioridade da thread, 0: alta, 1: média, 2: baixa
 * @retval inteiro positivo caso tenha conseguido criar a thread, -1 caso contrário
 */
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
	newThread->context.uc_stack.ss_flags = 0;
	newThread->isWaitingMe = -1;

	//cria um contexto, passas a função start (parametro da ccreate), é 1 parametro pra ela que tá em arg
	makecontext(&(newThread->context), (void (*)(void))start, 1, arg);

	//append na fila de aptos
	switch (prio)
	{
	case BAIXA_PRIORIDADE:
		if (AppendFila2(pfilaAptoBaixa, (void *)newThread) != 0)
			exit(-1);
		else
			FirstFila2(pfilaAptoBaixa);
		break;
	case MEDIA_PRIORIDADE:
		if (AppendFila2(pfilaAptoMedia, (void *)newThread) != 0)
			exit(-1);
		else
			FirstFila2(pfilaAptoMedia);
		break;
	case ALTA_PRIORIDADE:
		if (AppendFila2(pfilaAptoAlta, (void *)newThread) != 0)
			exit(-1);
		else
			FirstFila2(pfilaAptoAlta);
		break;
	}

	return newThread->tid;
}

/**
 * @brief  Troca a prioridade da thread em execução
 * @note   
 * @param  tid: NULL 
 * @param  prio: nova prioridade da thread em execução
 * @retval 0 se conseguiu, -1 caso contrário
 */
int csetprio(int tid, int prio)
{
    if (!initialized)
		initQueues(); // inicializa as filas (todas)

	if (tid != (int)NULL || prio > BAIXA_PRIORIDADE || prio < ALTA_PRIORIDADE)
		return -1;
	else
	{
		runningThread->prio = prio;
		return 0;
	}
}

/**
 * @brief  Cedência voluntária, muda o estado da thread em execução para apto
 * @note   
 * @retval 0 caso conseguiu, -1 caso contrário
 */
int cyield(void)
{
    if (!initialized)
		initQueues(); // inicializa as filas (todas)

	//muda o estado para apto e coloca em uma das filas

	runningThread->state = PROCST_APTO;

	// salva o contexto na thread

	if (getcontext(&(runningThread->context)) == -1)
	{
		return -1;
	}

	//se a thread chamou essa função agora (isto é, não está retornando sua execução) o scheduler é chamado
	if (runningThread->state == PROCST_APTO)
		scheduler();
    else {
        return 0;
    }

    return 0;
}
/**
 * @brief  Sincronização de término, bloqueia a thread em execução até o termino da thread com id informado
 * @note   
 * @param  tid: id da thread a ser esperada
 * @retval 0 caso sucesso, -1 caso não encontrou a thread que deseja esperar, -2 se já tem alguem esperando pela thread que deseja esperar
 */
int cjoin(int tid)
{
    if (!initialized)
		initQueues(); // inicializa as filas (todas)

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
			runningThread->state = PROCST_BLOQ;
			// salva o contexto da thread que ta rodando (ela vai ser bloqueada)
			getcontext(&(runningThread->context));
			//quando a execução dela voltar ela volta aqui, mas ja passou pelo scheduler
			if (runningThread->state == PROCST_BLOQ)
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

/**
 * @brief  
 * @note   
 * @param  *sem: 
 * @param  count: 
 * @retval 
 */
int csem_init(csem_t *sem, int count)
{
    if (!initialized)
		initQueues(); // inicializa as filas (todas)

    sem->count = count;

    sem->fila = (PFILA2) malloc(sizeof(PFILA2));

    if(CreateFila2(sem->fila) != 0)
        return -1;
    else
        FirstFila2(sem->fila);

    return 0;
}

/**
 * @brief  
 * @note   
 * @param  *sem: 
 * @retval 
 */
int cwait(csem_t *sem)
{
    if (!initialized)
		initQueues(); // inicializa as filas (todas)

    if(sem == NULL)
    {
        return ERROR_NULL_PARAM;
    }

    int status = 0;

    if(sem->count <= 0)
    {
        runningThread->state = PROCST_BLOQ;
        status = AppendFila2(sem->fila, runningThread);
        getcontext(&(runningThread->context));

        if (status != 0)
        {
            return ERROR_CANT_APPEND_THREAD;
        }


        if(runningThread->state == PROCST_BLOQ)
            scheduler();
    }
    sem->count--;
    return SUCCESS;
}

/**
 * @brief  
 * @note   
 * @param  *sem: 
 * @retval 
 */
int csignal(csem_t *sem)
{
    if (!initialized)
		initQueues(); // inicializa as filas (todas)
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
                exit(-1);
                return ERROR_APPEND_THREAD;
            }
            else
                FirstFila2(pfilaAptoBaixa);
            break;
        case MEDIA_PRIORIDADE:
            if (AppendFila2(pfilaAptoMedia, (void *)ThreadFromSem) != 0)
            {
                exit(-1);
                return ERROR_APPEND_THREAD;
            }
            else
                FirstFila2(pfilaAptoMedia);
            break;
        case ALTA_PRIORIDADE:
            if (AppendFila2(pfilaAptoAlta, (void *)ThreadFromSem) != 0)
            {
                exit(-1);
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

/**
 * @brief  Identificação dos integrantes do grupo
 * @note   
 * @param  *name: ponteiro para string a ser preenchida com nome dos integrantes do grupo
 * @param  size: tamanho da string no ponteiro name
 * @retval 
 */
int cidentify(char *name, int size)
{
    if (!initialized)
		initQueues(); // inicializa as filas (todas)

	if (size < MINIMUM_STRING_SIZE)
	{
		return ERROR_MINIMUM_SIZE_NOT_ENOUGH;
	}

	strncpy(name, "Afonso Ferrer - 252856\nDiego Dimer - 287690\nEduardo Paim - 277322", size);
	return SUCCESS;
}

