#include <cthread.h>
#include <stdio.h>
#include <stdlib.h>

int	id0, id1, id2, id3;
csem_t* sem;

void* func2(void *arg)
{
    printf("Eu sou a thread ID3\n");
}

void* func3(void *arg)
{
    printf("Eu sou a thread ID4\n");
}

void* func0(void *arg)
{
    printf("Eu sou a thread ID1 pedindo o recurso\n");
    cwait(sem);
    printf("ID1 desbloqueada!\n");
    csignal(sem);
}

void* func1(void *arg)
{
    printf("Eu sou a thread ID2 pedindo o recurso\n");
    cwait(sem);
    printf("ID2 desbloqueada!\n");
    csignal(sem);

}

int main(int argc, char *argv[])
{


    int i=0;
    sem = malloc(sizeof(csem_t));
    csem_init(sem, 1);

    id0 = ccreate(func0, (void *)&i, 0);
    id1 = ccreate(func1, (void *)&i, 1);

    printf("Eu sou a main pegando o recurso\n");
    cwait(sem);
    printf("Main com o recurso yieldando\n");
    cyield(); // rodar ID1 e ID2
    printf("Main liberando o recurso\n");
    csignal(sem);
    cyield(); // rodar ID1
    cyield(); // rodar ID2

    printf("Eu sou a main voltando para terminar o programa\n");
    cjoin(id0); //a main termina o programa, mas quero que as threads rodem por receberem o recurso (serem desbloqueadas)
    cjoin(id1);
}



