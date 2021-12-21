#include "sem.h"

void handler()
    {hcount++;}

void sem_init(struct sem *s, int count) //initialize count and wait length
{
    s->count = count;
    s->wlength = 0; 
}

int sem_try(struct sem *s) // try P 
{
    spin_lock(&(s->lock));
    if(s->count > 0)
    {
        s->count--; 
        spin_unlock(&(s->lock));
        return 1;
    }
    spin_unlock(&(s->lock));
    return 0; 
}

void sem_wait(struct sem *s)
{
    s->items[my_procnum].item.pid = getpid(); //set the PID

    while(!(sem_try(s)))
    {
        spin_lock(&(s->lock));
        signal(SIGUSR1, handler);

        sigset_t set;
        sigemptyset(&set);
        sigaddset(&set,SIGUSR1);
        sigprocmask(SIG_BLOCK, &set, NULL); //set the mask so no wait problem
        
        s->items[my_procnum].item.procnum = my_procnum;
        s->items[my_procnum].isAsleep = 1;
        s->wlength++; 
        spin_unlock(&(s->lock));
        
        sigset_t mask;
        sigemptyset(&mask); 
        
        s->asleep[my_procnum]++; 
        sigsuspend(&mask);
        s->awake[my_procnum]++;

        sigprocmask(SIG_UNBLOCK, &set, NULL);  //unblock the mask 
    }
}

void sem_inc(struct sem *s)
{
    spin_lock(&(s->lock));
    
    s->count++;
    if(s->wlength > 0 && s->count > 0)
    {
        for(int i = 0; i < N_PROC; i++)
        {
            if (s->items[i].isAsleep) 
            { 
                kill(s->items[i].item.pid, SIGUSR1); //send SIGUSR1 to everything on wait
                s->awake[s->items[i].item.procnum]++;
                s->wlength--;
                s->items[i].isAsleep = 0;
            }
        }
    }
    spin_unlock(&(s->lock));
}
