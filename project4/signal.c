#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <signal.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>

#define NUM_CHILDREN 1000000

int countFAKE;
int countREAL;

// sends signal to child processes numSend times
void deliveryMan(int PPid, int numSend)
{
    for(int i = 0; i < numSend; i++)
        {kill(PPid, SIGRTMIN); kill(PPid, SIGSEGV);}
}

// handler demonstrating number of fake time signals handled
void handlerFAKE(int signal)
    {++countFAKE;}

// handler demonstrating number of real time signals handled
void handlerREAL(int signal)
    {++countREAL;}

int main(int argc, char **argv)
{
    //set up the necessary structs and such
    struct sigaction saFAKE;
    saFAKE.sa_handler = handlerFAKE;
    sigemptyset(&saFAKE.sa_mask);
    saFAKE.sa_flags = SA_NODEFER;     
    (void)sigaction(SIGSEGV, &saFAKE, NULL);

    struct sigaction saREAL; 
    saREAL.sa_handler = handlerREAL; 
    sigemptyset(&saREAL.sa_mask);
    saREAL.sa_flags = SA_NODEFER;     
    (void)sigaction(SIGRTMIN, &saREAL, NULL);

    pid_t children[NUM_CHILDREN];
    int numSig = 1, numSend = 1; 

    //if arguments are given then use them     
    if (argv[1] && argv[2])
    {
        numSig = atoi(argv[1]);
        numSend = atoi(argv[2]);
    }
    
    //loop through the amount of times we need to send a signal and fork 
    for (int i = 0; i < numSig; i++) 
    {   
        int PPid; 
        switch (children[i] = fork())
        {
            case -1:
                fprintf(stderr, "ERROR: fork failed: %s", strerror(errno));
                break;
            case 0:
                PPid = getppid();
                deliveryMan(PPid, numSend); //send the signal
                return 0; 
            default: 
                break; 
        } 
    }

    int status;
    pid_t pid;
    
    // wait for each child process to exit
    for (int i = numSig; i > 0; i--)
        {pid = wait(&status);}
    
    printf("Fake time signals delivered: %d, fake time signals handled: %d\n", numSig * numSend, countFAKE);
    printf("Real time signals delivered: %d, real time signals handled: %d\n", numSig * numSend, countREAL);

    return 0; 
}


