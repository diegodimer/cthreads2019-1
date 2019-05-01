#include <cthread.h>
#include <stdio.h>
#include <stdlib.h>

int	id1, id2, id3, id4;
csem_t* sem;


void* func1(void *arg)
{
    int i;
    for(i=0; i<(int)arg;i++){
        printf("%d ", (int)i);
        cyield();
    }
}

void* func2(void *arg)
{
    int i;
    for(i=0; i<(int)arg;i++){
        printf("%d ", (int)arg - i);
        cyield();
    }}

void* func3(void *arg)
{
    cyield();
    printf("Eu sou a thread ID3 %d\n", (int)arg);
}


void* func4(void *arg)
{
    csetprio((int)NULL, 0); // muda prioridade para alta
    cyield();
    printf("Eu sou a thread ID4 %d\n", (int)arg);
}

int main(int argc, char *argv[])
{
    printf("Inteiro 5\n");
//    scanf("%d", &d);
    id1 = ccreate(func1, (void *)5, 0); // media prioridade
    id2 = ccreate(func2, (void *)5, 0); // baixa prioridade

    printf("Eu sou a main apos criacao das threads\n");

    cyield();

    printf("Eu sou a main voltando para terminar o programa\n");
}



