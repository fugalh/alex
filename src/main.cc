#include "jack.h"
#include "gsm.h"
#include "iax.h"
#include <unistd.h>
#include <signal.h>
#include <pthread.h>

void sighandler(int sig)
{
    if (sig == SIGINT)
        exit(0);
}

int main(int argc, char **argv)
{
    Jack jack;
    IAX iax(&jack);

    if (argc > 1)
	iax.call("2224","JS Bach",argv[1]);
    else
	iax.call("2224","JS Bach","test:foobar@fugal.net/s");

    while (1) { sleep(1); }

    return 0;
}
