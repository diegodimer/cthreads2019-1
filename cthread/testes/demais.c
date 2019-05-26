#include <cthread.h>
#include <stdio.h>
#include <stdlib.h>

int	id1, id2, id3, id4;
csem_t* sem;


void* func1(void *arg)
{
    cyield();
    printf("Eu sou a thread ID1 %d\n", (int)arg);
}

void* func2(void *arg)
{
    cyield();
    printf("Eu sou a thread ID2 %d\n", (int)arg);
}

void* func3(void *arg)
{
    cyield();
    printf("Eu sou a thread ID3 %d\n", (int)arg);
}


void* func4(void *arg)
{
    csetprio((int)NULL, 0); // muda prioridade para alta
    cjoin(3); // id4 se bloqueia até id3 terminar
    printf("Eu sou a thread ID4 %d\n", (int)arg);
}

int main(int argc, char *argv[])
{
    id1 = ccreate(func1, (void *)1    , 2); // baixa prioridade
    id2 = ccreate(func2, (void *)id1+1, 2); // baixa prioridade
    id3 = ccreate(func3, (void *)id2+1, 2); // baixa prioridade
    id4 = ccreate(func4, (void *)id3+1, 1); // media prioridade

    printf("Eu sou a main apos criacao das threads\n");

    cjoin(id4); // se bloqueia até id4 terminar, id4 vai rodar por ter prioridade média

    printf("Eu sou a main voltando para terminar o programa\n");
}



