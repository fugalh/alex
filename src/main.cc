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
    pthread_t iax_thread;
    Jack jack("alex");
    GSMCoder gsmc;
    IAXClient iax(&jack, &gsmc);

    pthread_create(&iax_thread, 0, iax_event_loop, &iax);

    if (argc > 1)
	iax.call("2224","JS Bach",argv[1]);
    else
	iax.call("2224","JS Bach","test:foobar@fugal.net/s");

    pthread_join(iax_thread, 0);
    return 0;
}
