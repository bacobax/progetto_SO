#ifndef SEM_UTILITY_H

#define SEM_UTILITY_H

#define SERRGET 4
#define SERRCTL 5
#define SERROP  6

#define LOCK -1
#define UNLOCK 1
#define WAITZERO 0

/* fa la semget, ma gestisce l'errore di default (se errorHandler = NULL)
// altrimenti si può passare una funzione per gestire l'errore */
int useSem(int key, void (*errorHandler)(int));

/* crea ed inizializza un solo semaforo */
int createSem(int key, int initValue, void (*errorHandler)(int err));

/* rimuove la struttura ipc, quindi rimuove il set di semafori */
void removeSem(int semid, void (*errorHandler)(int err));

/* esegue operazioni sul singolo semaforo con flag a 0
// LOCK decrementa
// UNLOCK incrementa
// WAITZERO aspetta lo 0 */
void mutex(int semid, int op, void (*errorHandler)(int err));

int createMultipleSem(int key, int nSem, int initValue, void (*errorHandler)(int err));

void mutexPro(int semid, int semIdx, int op, void (*errorHandler)(int err));

int getWaitingPxCount(int semid, int idx);

#endif
