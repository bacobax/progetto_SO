#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <time.h>
#include "../src/porto.h"
#include "../src/dump.h"
#include "./sem_utility.h"
#include "./shm_utility.h"
#include "./msg_utility.h"
#include "./support.h"
#include "./vettoriInt.h"

void refillerQuitHandler(int sig) {
    printf("refiller: ricevuto segnale di terminazione\n");
    exit(EXIT_SUCCESS);
}


Port initPort(int supplyDisponibility,int requestDisponibility, int pIndex) {

    int portShmId;
    int length;
    Port p;
    /*
        array nel quale verrà spartita casualmente la domanda in SO_MERCI parti
    */
    int* requests;
    /*
        array nel quale verrà spartita casualmente l'offerta in SO_MERCI parti
    */
    int* supplies;
    int i;
    int j;


    portShmId = useShm(PSHMKEY, SO_PORTI * sizeof(struct port), errorHandler);

    p = ((Port)getShmAddress(portShmId, 0, errorHandler)) + pIndex;


    requests = toArray(distribute(requestDisponibility, SO_MERCI), &length);
    supplies = toArray(distribute(supplyDisponibility, SO_MERCI), &length);

    /*
        informo l'area di dump delle risorse nel porto,
        lo faccio prima di azzerare delle offerte, perchè almeno vengono contate anche le merci che il porto non
        può offrire perchè c'è già la domanda

    */

    for (i = 0; i < SO_MERCI; i++) {
        printf("aggiungo %d merce di tipo %d in porto\n" , supplies[i] , i);
        addNotExpiredGood(supplies[i], i, PORT);
    }
    copyArray(p->requests, requests, length);
    fillMagazine(&p->supplies, 0, supplies);

    free(requests);
    free(supplies);

    fillExpirationTime(&p->supplies);
    reservePrint(printPorto, p, pIndex);

    for (i = 0; i < SO_MERCI; i++) {
        int c = rand() % 2;
        if (c == 1) {
            p->requests[i] = 0;
        }
        else if (c == 0) {
            p->supplies.magazine[0][i] = 0;
        }
        else {
            printf("Errore nella rand");
            exit(EXIT_FAILURE);
        }
    }

    /*
    Azzero tutti tipi delle risorse degli altri giorni
    */
    for (i = 1; i < SO_DAYS; i++) {
        for (j = 0; j < SO_MERCI; j++) {
            p->supplies.magazine[i][j] = -1;
        }
    }


    if (pIndex == 0) {
        p->x = 0;
        p->y = 0;
    }
    else if (pIndex == 1) {
        p->x = SO_LATO;
        p->y = 0;
    }
    else if (pIndex == 2) {
        p->x = SO_LATO;
        p->y = SO_LATO;
    }
    else if (pIndex == 3) {
        p->x = 0;
        p->y = SO_LATO;
    }
    else {
        p->x = (double)rand() / (double)(RAND_MAX / (SO_LATO));
        p->y = (double)rand() / (double)(RAND_MAX / (SO_LATO));
    }



    return p;
}



void printPorto(void* p, int idx) {

    int i;
    printf("Porto %d:\n", idx);
    printf("DOMANDE:\n");
    for (i = 0; i < SO_MERCI; i++) {
        printf("%d, \n", ((Port)p)->requests[i]);
    }

    printSupplies(((Port)p)->supplies);

    printf("coords:\n");
    printf("x: %f\n", ((Port)p)->x);
    printf("y: %f\n", ((Port)p)->y);

    printf("______________________________________________\n");

}

/*
    si assume che i messaggi siano sempre scritti con questo 'pattern': giorno|quantita
*/
void mexParse(char* mex, int* intDay, int* intQuantity) {
    int sizeDay;
    int sizeQuantity;
    int i;
    int c;
    int j;
    char* day;
    char* quantity;
    for (i = 0; *(mex + i); i++) {
        if(mex[i]=='|'){
            sizeDay = i;
            break;
        }
    }
    
    c = 0;
    
    for (i = i + 1; i < strlen(mex); i++) {
        c++;
    }
    sizeQuantity = c;
    

    day = malloc(sizeof(char) * sizeDay);
    quantity = malloc(sizeof(char) * sizeQuantity);

    for(i=0; i<sizeDay; i++){
        day[i] = mex[i];
    }
    i++;
    for (j = 0; j < sizeQuantity; j++) {
        quantity[j] = mex[i];
        i++;
    }
    
    *intDay = atoi(day);
    *intQuantity = atoi(quantity);
    free(day);
    free(quantity);
}


/*
    23|32
*/

int filterIdxs(int request) {
    return request != 0;
}


/* TODO: risolvere bug che non fa il parse del day del messaggio */
void refill(long type, char* text) {

    
    /*
        correctType varrà l'indice del  porto al quale il type del messaggio riferito farà riferimento,
        correcType = type - 1 perchè quando il master invia una certa quantità ad un certo porto, e vuole usare il type per riferirsi all'indice del porto,
        non può usare type = 0 perchè quel valore è riservato
    */
    int correctType;
    /*
        id della lista di semafori per cui semaforo[i] serve per bloccare scritture contemporanee nelle offerte del porto i
        (quindi se per esempio una nave deve prelevare deve assicurarsi che in quell'istante non ci sia un'operazione di riempimento delle risorse)
    */
    int portBufferSem;

    /*
        il refiller è solo un listener, non può sapere in che giorno ha appena ricevuto le risorse da distribuire,
        quindi il master, ogni giorno invia le risorse al refiller di tutti i porti specificando nel messaggio anche il giorno
        nel quale le sta inviando
    */
    int day;

    /*
        quantity è la quantità giornaliera che il master invia ogni giorno al refiller,
        ricavata dal messaggio
    */
    int quantity;

    /*
        ID della shm dei porti
    */
    int portShmID;
    Port p;

    /*
        array lungo SO_MERCI nel quale la quantità giornaliera sarà casualmente spartita tra le merci
    */
    int* quanties;
    /*
        length è stato dichiarato più per formalismo, perchè la funzione toArray restituisce anche la lunghezza dell'array
        anche se sappiamo che length = SO_MERCI
    */
    int length;
    /*
        lista dinamica che conterrà gli indici delle posizioni dell'array delle offerte nelle quali l'offerta per quel tipo di merce è a 0
        e ovviamente se è a 0 è perchè c'è già la domanda per quel tipo di merce
    */
    intList* listOfIdxs;

    
    int i;
    srand((int)time(NULL) % getpid());

    correctType = (int)(type - 1);
    
    portShmID = useShm(PSHMKEY, sizeof(struct port) * SO_PORTI, errorHandler);
    p = (Port)getShmAddress(portShmID, 0, errorHandler) + correctType;


    /*
        contiene gli indici delle domande = 0
    */
    listOfIdxs = findIdxs(p->requests, SO_MERCI, filterIdxs);
    /*
        printf("Ricevuto il messaggio %s\n", text);
    */


    sscanf(text, "%d|%d", &day, &quantity);

    printf("P %d , devo distribuire %d merci del giorno %d\n", correctType, quantity, day);
    /*
        !questa era stra deprecated
        mexParse(text, &day, &quantity);
    */

    /*
        printf("giorno: %d, quantità: %d\n", day, quantity);
    */
    
    
    quanties = toArray(distribute(quantity, SO_MERCI), &length);

    
    portBufferSem = useSem(RESPORTSBUFFERS, errorHandler);


    mutexPro(portBufferSem, (int)correctType, LOCK, errorHandler);
    /* fillMagazine(&p->supplies, 0, supplies); */

    fillMagazine(&p->supplies, day, quanties);


    /*
        Azzero le offerte del tipo di merce per cui c'è già la domanda
    */

    for (i = 0; i < listOfIdxs->length; i++) {
        p->supplies.magazine[day][*(intElementAt(listOfIdxs, i))] = 0;
    }

    
    mutexPro(portBufferSem, (int)correctType, UNLOCK, errorHandler);

    reservePrint(printPorto, p, correctType);
    /*
        shmDetach(p, errorHandler);
        !non funziona => invalid argument
    */
}

void refillerCode(int idx) {
    /*
        questo è una sorta di listener, che ascolta sempre in attesa di un messaggio per l'idx passatogli come argomento
        (indice del porto proprietario del refiller)
    */
    int refillerID;
    if (signal(SIGUSR1, refillerQuitHandler) == SIG_ERR) {
        perror("Refiller: non riesco a settare il signal handler\n");
        exit(EXIT_FAILURE);
    }

    refillerID = useQueue(REFILLERQUEUE, errorHandler);

    while (1) {
        /*
            idx+1 perchè nella coda di messaggi ci si riferisce all'indice di ogni porto incrementato di 1
            questo perchè type = 0 è riservato
        */
        msgRecv(refillerID, (long)(idx + 1), errorHandler, refill, ASYNC);
    }
}

void launchRefiller(int idx) {
    int pid = fork();

    if (pid == -1) {
        perror("Error launching the refiller");
        exit(EXIT_FAILURE);
    }
    if (pid == 0) {
        refillerCode(idx);
        exit(EXIT_FAILURE);
    }
}
/*
void updaterCode(int idx) {
    int portShmid;
    int rwExpTimesPortSemID;
    
    Port p;
    portShmid = useShm(PSHMKEY, sizeof(struct port) * SO_PORTI, errorHandler);

    p = (Port)getShmAddress(portShmid, 0, errorHandler) + idx;

    rwExpTimesPortSemID = useSem(WREXPTIMESSEM, errorHandler);


    mutexPro(rwExpTimesPortSemID, idx, LOCK, errorHandler);

    decrementExpTimes(&p->supplies);
    
    removeExpiredGoods(&p->supplies);
    
    mutexPro(rwExpTimesPortSemID, idx, UNLOCK, errorHandler);

    

}
*/

/*

void updateExpTimes(int idx) {
    int pid;

    pid = fork();

    if (pid == -1) {
        perror("errore nella fork per il decrementatore");
        exit(EXIT_FAILURE);
    }
    if (pid == 0) {
        updaterCode(idx);
        exit(EXIT_SUCCESS);
    }
    
}
*/