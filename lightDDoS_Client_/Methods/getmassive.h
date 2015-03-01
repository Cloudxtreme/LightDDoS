#ifndef GETMASSIVE_H
#define GETMASSIVE_H
#include "unp.h"
#include <pthread.h>
#include <unistd.h>
#include <syscall.h>
#define MYPORT 80
void * getMassivePthread(void * argv);
void * getMassive(void * argv);
#endif // GETMASSIVE_H
