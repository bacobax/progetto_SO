#ifndef CONFIG_1

#define CONFIG_1


#define SO_PORTI    4
#define SO_NAVI     4
#define SO_MERCI     5
#define SO_SIZE      20 /* tonn */
#define SO_MIN_VITA  3 /* tonn */
#define SO_MAX_VITA  7/* tonn */
#define SO_DAYS      7

/*chiavi dei semafori*/
/* chiave del semaforo del master */
#define MASTKEY       132
#define WAITCONFIGKEY       245
#define BANCHINESEMKY 165
/* chiave del semaforo che permette la reserve print */
#define RESPRINTKEY  124
#define WAITENDDAYKEY 3415

#define ERRFILESEMID 3214
#define DAYWORLDSHM 3213
#define WAITENDDAYSHIPSEM 3212

#define WAITPORTRESPONSES 3002
#define WAITFIRSTRESPONSES 3003
#define WAITRMVDUMPKEY 3001
#define RESPORTSBUFFERS 213
#define WREXPTIMESSEM 246

#define SEMSHIPKEY 9090

#define ENDPROGRAMSHM 3216 
#define WAITSHIPSSEM 3217
#define WAITPORTSSEM 3218

/*chiavi delle shm*/
#define PSHMKEY       342
#define SSHMKEY       893
#define DUMPSHMKEY    3215

/*chiavi delle queue*/
#define REFILLERQUEUE 412




#define SO_FILL      10000



/* ciao */
#define SO_SPEED     0.7
#define SO_LATO      4
#define SO_CAPACITY  100
#define SO_BANCHINE  10
#define SO_LOADSPEED 200 

/* funzione generale che gestisce l'errore */
void errorHandler(int err, char* errCtx);

#endif
