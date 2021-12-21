#ifndef _SPINLOCK_H
#define _SPINLOCK_H
#define N_PROC 64

#include <stdio.h>
#include <sched.h>
#include <stdlib.h>
#include "tas.h"

int spin_lock(volatile char *lock);
int spin_unlock(volatile char *lock);

#endif