#ifndef MASTER_H
#define MASTER_H
#include "./porto.h"
#include "./nave.h"
#include "../config1.h"

/* funzione che genera navi */
void genera_navi();

/* funzione che genera porti */
void genera_porti(int risorse, int n_porti);


/* codice che verrà eseguito dal processo master */
void codiceMaster(int semid, int portsShmid, int shipsShmid, int reservePrintSem);



#endif
