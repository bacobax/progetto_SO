

#ifndef MSG_UTILITY

#define MSG_UTILITY

#define MEXBSIZE 16

typedef struct _mex {
    long mtype;
    char mtext[MEXBSIZE];
} mex;

#define SYNC 0
#define ASYNC 1

#define MERRGET 0
#define MERRCTL 1
#define MERRSND 2
#define MERRRCV 3



#define EX_KEY 132

/* semplificazione della msgsnd con una gestione degli errori di default
   se si setta l'handler a NULL allora è usato quello di default */
void msgSend(int msgqID, char text[MEXBSIZE], long type, void (*errorHandler)(int err));


/* semplificazione della msgrcv con una gestione degli errori di default
se si setta l'handler a NULL allora è usato quello di default
il callback da passare è quello che si vuole che venga eseguito una volta ricevuto il messaggio
questa funzione crea un processo figlio che si occupa di eseguire il callback
mod può essere = SYNC oppure = ASYNC
se è = SYNC, l'handler non è eseguito e ritorna il puntatore al messaggio (dev'essere liberato)
se è = ASYNC, questa funzione crea un processo che getisce indipendentemente la ricezione del messaggio
se è != ASYNC e != SYNC, da errore */
mex* msgRecv(int msgqID, long type, void (*errorHandler)(int err), void (*callback)(long type, char text[MEXBSIZE]), int mod);

/* in caso di successo restituisce l'id della queue
restituisce EEXIST se esiste già
se si presentano altri errori viene eseguito l'handler
se si setta l'handler a NULL allora è usato quello di default */
int createQueue(int key, void (*errorHandler)(int err));


/* ritorna l'id di una queue esistente */
int useQueue(int key, void (*errorHandler)(int err));

/* ritorna il numero di messaggi */
int getMexCount(int id, void (*errorHandler)(int err));

void printQueueState(int id, void (*errorHandler)(int err));

void removeQueue(int id, void (*errorHandler)(int err));

#endif
