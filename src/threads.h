#ifndef ALEX_THREADS_H
#define ALEX_THREADS_H

#include <semaphore.h>

extern pthread_mutex_t iax_event_queue_mut;
extern sem_t event_sem;

#endif
