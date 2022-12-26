#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <sys/types.h>
#include "../config1.h"
#include "../utils/msg_utility.h"
#include "../utils/support.h"
#include "../utils/vettoriInt.h"

#include "./porto.h"
#include "../utils/supplies.h"



void codicePorto(Port porto, int myQueueID, int shipsQueueID, int idx) {
    int quantity = -2;
    mex* messaggioRicevuto;
    int res;
    int tipoTrovato;
    int dayTrovato;
    int dataScadenzaTrovata;
    char text[MEXBSIZE];
    int sonostatoScelto;
    waitForStart();

    /* START */
     while (1) {

        /*
            E' importante che sia sincrona la gestione del messaggio ricevuto
            perchè prima di poterne ricevere un altro il porto deve poter aver aggiornato le sue disponibilità
        */

        /*
            prendo il primo messaggio che arriva
        */
        messaggioRicevuto = msgRecv(myQueueID, 0, errorHandler, NULL, SYNC);
        sscanf(messaggioRicevuto->mtext, "%d", &quantity);

        printf("Port %d: Ricevuto messaggio da nave %d con quantità %d\n", getpid(), messaggioRicevuto->mtype - 1, quantity);
        res = trovaTipoEScadenza(&porto->supplies, &tipoTrovato, &dayTrovato, &dataScadenzaTrovata, quantity);

        printf("Port %d: Ho trovato il tipo %d con data di scadenza %d\n", getpid(), tipoTrovato, dataScadenzaTrovata);

        if (res == -1) {
            msgSend(shipsQueueID, "x", idx + 1, errorHandler);
        }
        else {

            porto->supplies.magazine[dayTrovato][tipoTrovato] -= quantity;
            sprintf(text, "%d %d", tipoTrovato, dataScadenzaTrovata);
            msgSend(shipsQueueID, text, idx + 1, errorHandler);
        }

        messaggioRicevuto = msgRecv(myQueueID, messaggioRicevuto->mtype, errorHandler, NULL, SYNC);

        sscanf(messaggioRicevuto->mtext, "%d", &sonostatoScelto);
        printf("Port %d: valore di sonostatoScelto = %d\n", getpid(), sonostatoScelto);
        if (sonostatoScelto == 0 && res != -1) {
            printf("Porto %d, non sono stato scelto anche se avevo trovato della rob\n", getpid());
            porto->supplies.magazine[dayTrovato][tipoTrovato] += quantity;
        }

        if (sonostatoScelto == 1 && res == 1) {
            printf("Porto %d: sono stato scelto\n", getpid());
        }
    }

    // launchGoodsDispatcher(myQueueID, porto, idx, shipsQueueID);


}

int main(int argc, char const* argv[]) {
    int supplyDisponibility;
    int requestDisponibility;
    int idx;

    supplyDisponibility = atoi(argv[1]);
    requestDisponibility = atoi(argv[2]);
    idx = atoi(argv[3]);

    mySettedPort(supplyDisponibility, requestDisponibility, idx, codicePorto);
    

}


