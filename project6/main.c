#include "spinlock.h"
#include "sem.h"

int monte(struct sem *from, struct sem *to, int moves)
{
    for(int i = 0; i < moves; i++)
    {
        sem_wait(from);
        sem_inc(to);
    }
    printf("Child %d (pid %d) done, signal handler was invoked %d times\n", my_procnum, getpid(), hcount);
    printf("VCPU %d done\n", my_procnum);
    return 0;
}


int main(int argc, char *argv[])
{
    int pebbles, moves;
    if(argc <= 1)
    {
        printf("ERROR: That was silly! It's like this dumbass: ./shellgame.exe <pebbles> <moves>\n");
        return 1;
    }

    pebbles = atoi(argv[1]);
    moves = atoi(argv[2]);

    struct sem *A = (struct sem*) mmap(NULL, sizeof(int), PROT_READ|PROT_WRITE, MAP_SHARED|MAP_ANONYMOUS, -1, 0);
    struct sem *B = (struct sem*) mmap(NULL, sizeof(int), PROT_READ|PROT_WRITE, MAP_SHARED|MAP_ANONYMOUS, -1, 0);
    struct sem *C = (struct sem*) mmap(NULL, sizeof(int), PROT_READ|PROT_WRITE, MAP_SHARED|MAP_ANONYMOUS, -1, 0);

    sem_init(A, pebbles);
    sem_init(B, pebbles);
    sem_init(C, pebbles);

    pid_t childPids[6];

    for (int i = 0; i < 6; i++) 
    {   
        int ppid; 
        switch (childPids[i] = fork())
        {
            case -1:
                fprintf(stderr, "ERROR: fork failed: %s", strerror(errno));
                break;

            case 0:
                printf("VCPU %d starting, pid %d\n", i, getpid());
                my_procnum = i;
                int exitVal;
                switch (i)
                {
                    case 0: 
                        exitVal = monte(A, B, moves);
                        printf("Child pid %d exited w/ %d\n", getpid(), exitVal);
                        break;
                    case 1:
                        exitVal = monte(A, C, moves);
                        printf("Child pid %d exited w/ %d\n", getpid(), exitVal);
                        break;
                    case 2:
                        exitVal = monte(B, A, moves);
                        printf("Child pid %d exited w/ %d\n", getpid(), exitVal);
                        break;
                    case 3: 
                        exitVal = monte(B, C, moves);
                        printf("Child pid %d exited w/ %d\n", getpid(), exitVal);
                        break;
                    case 4: 
                        exitVal = monte(C, A, moves);
                        printf("Child pid %d exited w/ %d\n", getpid(), exitVal);
                        break;
                    case 5:
                        exitVal = monte(C, B, moves);
                        printf("Child pid %d exited w/ %d\n", getpid(), exitVal);
                        break;
                }
                return 0; 

            case 1: 
                break; 
        } 
    }

    printf("Main process spawned all children, waiting\n");

    int status;
    pid_t pid;

    int n = 6; 
    while (n > 0)
    {
        pid = wait(&status);
        --n;
    }
    
    printf("Sem#\t\tval\t\tSleeps\t\tWakes\n");

    struct sem *semen[] = {A, B, C}; 
    for(int i = 0; i < 3; i++)
    {
        printf("%d\t\t%d\n", i, semen[i]->count);
        for(int j = 0; j < 6; j++)
            printf("VCPU %d\t\t\t\t%d\t\t%d\n", j, semen[i]->asleep[j], semen[i]->awake[j]);

        printf("\n");
    }

    return 0;
}
