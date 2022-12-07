#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <sys/types.h>
#include "../config1.h"
#include "../utils/sem_utility.h"
#include "../utils/support.h"
#include "../utils/vettoriInt.h"

#include "./porto.h"


//TODO: creare funzione initRequest() e initSupplies() che le genera randomicamente e che ritornano un tipo intList*
//TODO: poi modificare i parametri di initPort() da int* a intList*

Port initPort(int* requests, int* supplies) {
    Port p = (Port)malloc(sizeof(struct port));


    p->requests = intInitFromArray(requests, SO_MERCI);
    p->supplies = intInitFromArray(supplies, SO_MERCI);

    return p;
}

void freePort(Port p) {
    intFreeList(p->requests);
    intFreeList(p->supplies);
    free(p);
}


void crateSharedStruct() {

}

int main(int argc, char const* argv[]) {




    int semid = useSem(MASTKEY, NULL);

    mutex(semid, WAITZERO, NULL);

    printf("Ciao, sono il porto con quantità %d\n", atoi(argv[1]));
    return 0;

}


