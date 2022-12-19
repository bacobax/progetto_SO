#ifndef CONFIG_1

#define CONFIG_1


#define SO_PORTI     1
#define SO_NAVI      10
#define SO_MERCI     3
#define SO_SIZE      20 /* tonn */
#define SO_MIN_VITA  6 /* tonn */
#define SO_MAX_VITA  10 /* tonn */
#define SO_DAYS      3

/*chiavi dei semafori*/
/* chiave del semaforo del master */
#define MASTKEY       132
#define WAITCONFIGKEY       245
#define BANCHINESEMKY 165
/* chiave del semaforo che permette la reserve print */
#define RESPRINTKEY  124
#define RESPORTSBUFFERS 213

/*chiavi delle shm*/
#define PSHMKEY       342
#define DUMPSHMKEY    3215

/*chiavi delle queue*/
#define REFILLERQUEUE 412




#define SO_FILL      100



/* ciao */
#define SO_SPEED     500
#define SO_LATO      4
#define SO_CAPACITY  10
#define SO_BANCHINE  2
#define SO_LOADSPEED 200 

/* funzione generale che gestisce l'errore */
void errorHandler(int err);

#endif
