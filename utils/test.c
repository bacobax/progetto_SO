#include "./msg_utility.h"
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

void msgRecvHandler(long type, char text[MEXBSIZE]) {
    printf("Messaggio di tipo %ld ricevuto: %s\n", type, text);
}

int main(int argc, char const* argv[])
{
    int id = createQueue(EX_KEY, NULL);
    if (id == EEXIST) {
        id = useQueue(EX_KEY, NULL);
        printf("Coda esiste già, la uso. ID: %d\n", id);
    }
    else {
        printf("Coda creata. ID: %d\n", id);
    }

    msgSend(id, "ciao", 1, NULL);
    //msgRecv(id, 1, NULL, msgRecvHandler, ASYNC);

    //  printf("Messaggio di tipo %ld ricevuto: %s\n", m->mtype, m->mtext);
    //* free(m);

    printQueueState(id);

    printf("Numero mex: %d\n", getMexCount(id));
    //removeQueue(id);
    return 0;
}
