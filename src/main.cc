#include "jack.h"
#include "gsm.h"
#include "iax.h"
#include <unistd.h>
#include <signal.h>
#include <pthread.h>

pthread_mutex_t stopmutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t stopcond = PTHREAD_COND_INITIALIZER;
void sighandler(int sig)
{
    if (sig == SIGINT)
	pthread_cond_signal(&stopcond);
}

int main(int argc, char **argv)
{
    pthread_t iax_event_thread;
    Jack jack("alex");
    GSMCoder gsmc;
    IAXClient iax(&jack, &gsmc);

    iax.call("2224","JS Bach","hans:m00se@fugal.net/s");
    sleep(10);
    iax.hangup("guten abend");

    pthread_create(&iax_event_thread, 0, iax_event_thread_func, &iax);
    pthread_join(iax_event_thread, 0);
    return 0;
}
