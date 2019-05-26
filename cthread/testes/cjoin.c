#include <cthread.h>
#include <stdio.h>
#include <stdlib.h>

int	id1, id2, id3, id4;
csem_t* sem;


void* func1(void *arg)
{
    printf("Eu sou a thread ID1 %d\n", (int)arg);
}

void* func2(void *arg)
{
    printf("Eu sou a thread ID2 %d\n", (int)arg);
}

void* func3(void *arg)
{
    cjoin(2); // rodaria após a id2 pois tem prioridade alta, então se bloqueia e espera a id2 terminar
    printf("Eu sou a thread ID3 %d\n", (int)arg);
}


void* func4(void *arg)
{
    cjoin(3); // espera a 3 ser desbloqueada e rodar pra rodar (é pra ser a última)
    printf("Eu sou a thread ID4 %d\n", (int)arg);
}

int main(int argc, char *argv[])
{


    id1 = ccreate(func1, (void *)1, 0); // alta prioridade
    id2 = ccreate(func2, (void *)id1+1, 2); // baixa prioridade
    id3 = ccreate(func3, (void *)id2+1, 0); // alta prioridade
    id4 = ccreate(func4, (void *)id3+1, 1); // media prioridade

    printf("Eu sou a main apos criacao das threads, dando join para id1\n");

    cjoin(id4); // main vai esperar a id4 acabar, vai ser bloqueada e vai rodar a id1 (alta prioridade)


    printf("Eu sou a main voltando para terminar o programa\n");
}



