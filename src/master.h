#ifndef MASTER_H
#define MASTER_H
#include "./porto.h"
#include "./nave.h"
#include "../config1.h"
#define SYNC 0
#define ASYNC 1
/* funzione che genera navi */
void genera_navi();

/* funzione che genera porti */
void genera_porti(int risorse, int n_porti);


/* codice che verrà eseguito dal processo master (configurazione esclusa)*/
void codiceMaster(int startSimulationSemID, int portsShmid, int shipsShmid, int reservePrintSem,  int waitconfigSemID, int msgRefillerID, int waitEndDaySemID);

/* Separa logicamente ciò che deve fare il master e tutti le creazioni e le cancellazioni delle varie risorse IPC
Infatti in questa funzione vengono allocate le risorse IPC, viene eseguito il codice del master
e infine vengono deallocate
Ovviamente il master ha accesso a tutti gli id perchè gli sono passati come parametro
Questa scelta è dovuta per facilitare la lettura del codice e per rimuovere dalla logica del master tutto ciò che usato solo per il setting */
void mySettedMain(void (*codiceMaster)(int startSimulationSemID, int portsShmid, int shipsShmid, int reservePrintSem, int waitconfigSemID, int msgRefillerID, int waitEndDaySemID));

/*
    esegue una waitzero di un semaforo inizializzato a SO_PORTI + SO_NAVI e aspetta quando tutti quanti i processi hanno eseguito una LOCK so quel semaforo,
    un porto o una nave esegue una lock su quel semaforo quando hanno finito la configurazione
    il motivo di questa funzione è che non è detto che nel momento in cui il master darà il via ai processi per essere eseguiti, questi ultimi
    avranno finito di configurarsi, magari alcuni avranno finito e partiranno subito il loro ciclo di vita, gli altri no
*/
void aspettaConfigs(int waitConfigSemID);


void refillPorts(int opt, int msgRefillerID, int quantitaAlGiorno, int giorno);

void expirePortsGoods(int day);
void expireShipGoods();


#endif
