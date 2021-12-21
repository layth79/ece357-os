#ifndef _SEM_H
#define _SEM_H

#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <stddef.h>
#include <sys/mman.h>
#include "spinlock.h"

int my_procnum; //the current process number
int hcount; //how many times the handler was invoked

struct proc //each process gets its own pid and procnum
{
    int pid; 
    int procnum; 
};

struct barray //for checking if things are asleep 
{
    struct proc item; 
    int isAsleep; 
};

struct sem
{
    volatile char lock;
    int count;
    int asleep[6]; //count how many times each proc sleeps
    int awake[6]; //count how many times each proc wakes up
    struct barray items[N_PROC];
    int wlength; //how many things are waiting right now 
};

void sem_init(struct sem *s, int count);
int sem_try(struct sem *s);
void sem_wait(struct sem *s);
void sem_inc(struct sem *s);
#endif
