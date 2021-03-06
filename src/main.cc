#include "jack.h"
#include "gsm.h"
#include "iax.h"
#include <unistd.h>
#include <signal.h>
#include <pthread.h>

void sighandler(int sig)
{
    if (sig == SIGINT)
    {
	printf("\n");
        exit(0);
    }
}

int main(int argc, char **argv)
{
    Jack jack;
    IAX iax(&jack);

    signal(SIGINT, sighandler);
    pthread_create(&iax.thread, 0, protocol_thread_func, &iax);

    if (argc > 1)
	iax.call("2224","JS Bach",argv[1]);
    else
	iax.call("2224","JS Bach","guest@fugal.net/0");

    while (1) { sleep(1); }
    return 0;
}
