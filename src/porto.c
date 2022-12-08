#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>

#include <sys/types.h>
#include "../config1.h"
#include "../utils/sem_utility.h"
#include "../utils/support.h"
#include "../utils/vettoriInt.h"

#include "./porto.h"

//TODO: rifare initporto
Port initPort(int disponibility) {
    Port p = (Port)malloc(sizeof(struct port));
    srand(time(NULL));

    p->requests = distribute(disponibility, SO_MERCI);
    p->supplies = distribute(disponibility, SO_MERCI);

    return p;
}

void freePort(Port p) {
    intFreeList(p->requests);
    intFreeList(p->supplies);
    free(p);
}




void waitForStart() {
    int semid = useSem(MASTKEY, NULL);
    mutex(semid, WAITZERO, NULL);
}

int main(int argc, char const* argv[]) {

    int disponibility = atoi(argv[1]);
    int idx = atoi(argv[2]);

    Port p = initPort(disponibility);


    waitForStart();
    //*START

    printf("Porto:\n");
    printf("Ciao, sono il porto con quantità %d\n", atoi(argv[1]));

    intStampaLista(p->requests);
    intStampaLista(p->supplies);
    printf("______________________________________________\n");

    return 0;

}


