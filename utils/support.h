#ifndef SUPPORT_H
#define SUPPORT_H
#define MILLIS_MULT 1000000
#include "./vettoriInt.h"

//distribuisce casualmente un quantità "quantity" in "parts" parti
intList* distribute(int quantity, int parts);

//serve per stampare a schermo i dati di una struttura senza essere interrotti da una stampa di un altro processo
void reservePrint(void (*printer)(void* obj, int idx), void* obj, int idx);

//handler del segnale USR1 che quitta
void quitSignalHandler(int sig);


//Separa logicamente ciò che deve fare il master e tutti le creazioni e le cancellazioni delle varie risorse IPC
//Infatti in questa funzione vengono allocate le risorse IPC, viene eseguito il codice del master
//e infine vengono deallocate
//Ovviamente il master ha accesso a tutti gli id perchè gli sono passati come parametro
//Questa scelta è dovuta per facilitare la lettura del codice e per rimuovere dalla logica del master tutto ciò che usato solo per il setting
void mySettedMain(void (*codiceMaster)(int semid, int portsShmid, int shipsShmid, int reservePrintSem));


#endif