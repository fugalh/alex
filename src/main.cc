#include "jack.h"
#include "gsm.h"
#include "iax.h"
#include <unistd.h>

int main(int argc, char **argv)
{
    Jack *jack = new Jack("alex");
    //GSMCoder *gsmc = new GSMCoder();
    //IAXClient *iax = new IAXClient(jack, gsmc);

    //delete iax;
    delete jack;
    //delete gsmc;
    return 0;
}
