#include <stdio.h>
#include <stdlib.h>
#include "../src/nave.h"
#include "../src/porto.h"
#include "../utils/loadShip.h"
#include "../config1.h"
#include "./support.h"
#include "../src/dump.h"
#include "../utils/msg_utility.h"
#include "../utils/sem_utility.h"
#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

void newDayHandlerShip(int sig){
    printf("Giorno finito, aggiornare la data di scadenza delle merci"); /* il dump lo farà il master*/

    checkProducts(ship->load);
}

void checkProducts(loadShip load){

    Product p = load->first;

    while(p != NULL){ /* per ogni prodotto nella lista faccio le seguenti operazioni:*/

        p->expirationTime = p->expirationTime - 1; /* decremento di 1 giorno la data di scadenza*/

        if(p->expirationTime == 0){ /* la merce è scaduta*/

            removeProduct(load, p->id_product); /* rimuovo la merce dal carico della nave*/
            
            /* RICORDARSI DI AGGIORNARE STATO MERCE SCADUTA PER IL DUMP DENTRO LA REMOVE PRODUCT*/
        }

        p = p->next;
    }
}


int checkCapacity() {
    if (ship->load->weightLoad == 0) return 0;
    return ship->load->weightLoad;
}

int availableCapacity() {

    int currentCapacity;

    currentCapacity = checkCapacity(ship);
    if (currentCapacity < 0) return -1;
    return (SO_CAPACITY - currentCapacity);
}

double generateCord() {

    double range, div;

    range = (SO_LATO); /* max-min */
    div = RAND_MAX / range;
    return (rand() / div);
}

Ship initShip(int shipID) {

    if (signal(SIGUSR1, quitSignalHandler) == SIG_ERR) {              /* imposto l'handler per la signal SIGUSR1 */
        perror("Error trying to set a signal handler for SIGUSR1");
        exit(EXIT_FAILURE);
    }

    if (signal(SIGALRM, newDayHandlerShip) == SIG_ERR) {              /* imposto l'handler per la signal SIGALARM */
        perror("Error trying to set a signal handler for SIGUSR1");
        exit(EXIT_FAILURE);
    }

    /* inizializziamo la nave */
    ship = (struct ship*)malloc(sizeof(struct ship));
    ship->shipID = shipID;
    ship->x = generateCord();
    ship->y = generateCord();
    ship->capacity = 0;
    /*
        load sarà NULL all'inizio
    */
    ship->load = initLoadShip();

    return ship;
}

void printShip(int id_ship){
    printf("[%d]: Nave\n", id_ship);
    
    printf("coords: [x:%f, y:%f]\n", (ship->x), (ship->y));
    
    printf("ton trasporati:%d\n", availableCapacity(ship));

    printf("carico trasportato:\n");
    printLoadShip(ship->load);
    
    printf("______________________________________________\n");

}



int chooseQuantityToCharge(){
    
}

int callPorts(int quantityToCharge, int* portIDs){

    /*
        La nave manda un messaggio di richiesta a tutti i porti perchè è intenzionata di fare un'operazione di caricamento merci.

        Ogni porto controlla il proprio magazzino e risponde con il tipo di merce e la data di scadenza che si può caricare da esso, ovviamente
        in base alla quantità disponibile per la nave. Se il porto non trova niente manda un messaggio negativo (DA STABILIRE LA SINSTASSI DEL MESSAGGIO).

    */

   /* preperazione costruzione messaggio da inviare per i port*/

    int i = 0;
    int queueID;
    char text[sizeof(quantityToCharge)];
    sprintf(text, "%d", quantityToCharge);

    for(i=0; i<SO_PORTI; i++){
        queueID = useQueue(/* CHIAVI DA DEFINIRE CON BASSI*/, errorHandler);
        msgSend(queueID, text, ship->shipID, errorHandler);
    }

}

int portResponses(struct port_offer* offers){
     
     /*
        Questa funzione ritorna un array con gli id dei porti che hanno
        riposto alla domanda della nave alla richiesta di carico merci
     */

    int i;
    int queueID;
    int ports = 0;
    mex* response;



    /* aspetto che ogni porto mi risponda con una stringa contenente
       l'id della merce che posso caricare e la sua data di scadenza 
    */

    for(i=0; i<SO_PORTI; i++){ 
        queueID = useQueue(/*CHIAVI DA DEFINIRE CON BASSI*/, errorHandler);
        response = msgRecv(queueID, i, errorHandler, NULL, SYNC);

        if(response->mtype != -1){ /* messaggio non negativo dal porto*/
            ports++;

            /*
                spacchetto il messaggio e lo inserisco nell'array offers

                FORMAT DEL MESSAGGIO:
                                        il type identifica il product_type
                                        mtext c'è scritto in formato stringa la scadenza in giorni del prodotto
            */

            offers[i].product_type = response->mtype;
            offers[i].expirationTime = atoi(response->mtext);         
        }
    }

    return ports;
}

int choosePort(struct port_offer* offers){

    /*
        adesso scorro l'array delle offerte ricevute dai porti per scegliere il tipo
        di merce migliore da andare a caricare e ritorno l'indice del porto dove andare
    */
}

void replyToPorts(int portID){

    /*
        la nave risponde a tutti i porti con un messaggio negativo
        tranne per il porto con portID che è quello scelto per andare
        a caricare la merce
    */

    int i;
    int queueID;

    for(i=0; i<SO_PORTI; i++){

        queueID = useQueue(/*CHIAVI DA DEFINIRE CON BASSI*/, errorHandler);

        if(i == portID){
            msgSend(queueID, "ok", ship->shipID, errorHandler);
        } else {
            msgSend(queueID, "negative", -1, errorHandler);
        }
    }
}

void travel(int portID){

    Port p;
    double dt_x, dt_y, spazio, nanosleep_arg;

    int portShmId = useShm(PSHMKEY, SO_PORTI * sizeof(struct port), errorHandler); /* prendo l'id della shm del porto */

    p = ((Port) getShmAddress(portShmId, 0, errorHandler)) + portID; /* prelevo la struttura del porto alla portID-esima posizione nella shm */

    /* imposto la formula per il calcolo della distanza*/
    
    dt_x = p->x - ship->x;
    dt_y = p->y - ship->y;

    spazio = sqrt(pow(dt_x, 2) + pow(dt_y, 2));
    /*
        spazio/SO_SPEED è misurato in giorni (secondi), quindi spazio/SO_SPEED*1000000000 sono il numero di nanosecondi per cui fare la sleep
    */
    nanosecsleep((long)((spazio / SO_SPEED) * NANOS_MULT));


    /* Dopo aver fatto la nanosleep la nave si trova esattamente sulle coordinate del porto
       quindi aggiorniamo le sue coordinate
    */

    ship->x = p->x;
    ship->y = p->y;
}

void accessPort(int portID) {
    int portSHMID;
    Port port;
    int banchineSem;
    portSHMID = useShm(PSHMKEY, sizeof(struct port) * SO_PORTI, errorHandler);

    port = (Port)getShmAddress(portSHMID, 0, errorHandler) + portID;

    banchineSem = useSem(BANCHINESEMKY, errorHandler);

    mutexPro(banchineSem, portID, LOCK, errorHandler);

    /*
     !SEZIONE CRITICA
    */
    
    mutexPro(banchineSem, portID, UNLOCK, errorHandler);

} 

