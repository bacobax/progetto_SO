#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/types.h>
#include "../config1.h"
#include "../utils/msg_utility.h"
#include "../utils/support.h"
#include "../utils/vettoriInt.h"

#include "./porto.h"
#include "../utils/supplies.h"



void codicePorto(Port porto, int myQueueID, int shipsQueueID) {
    int quantity;
    mex* messaggioRicevuto;
    int res;
    int tipoTrovato;
    int dataScadenzaTrovata;
    char text[MEXBSIZE];
    waitForStart();

    /* START */

    
  
    while (1) {
      
        /*
            E' importante che sia sincrona la gestione del messaggio ricevuto
            perchè prima di poterne ricevere un altro il porto deve poter aver aggiornato le sue disponibilità
        */

        
        messaggioRicevuto = msgRecv(myQueueID, 0, errorHandler, NULL, SYNC);
        sscanf(messaggioRicevuto->mtext, "%d", &quantity);
        
        printf("Ricevuto messaggio da nave %d con quantità %d\n", messaggioRicevuto->mtype - 1, quantity);
        res = trovaTipoEScadenza(&(porto->supplies), &tipoTrovato, &dataScadenzaTrovata, quantity);
        printf("Ho trovato il tipo %d con data di scadenza %d\n", tipoTrovato, dataScadenzaTrovata);
        if (res == -1) {
            msgSend(shipsQueueID, "x", messaggioRicevuto->mtype, errorHandler);
            
        }
        else {
            sprintf(text, "%d %d", tipoTrovato, dataScadenzaTrovata);
            msgSend(shipsQueueID, text, messaggioRicevuto->mtype, errorHandler);
            
        }
    }


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


