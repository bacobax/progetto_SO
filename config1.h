#ifndef CONFIG_1

#define CONFIG_1

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
#define CANDEADSEMKEY 3211


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






#define RESTTIMESHIP 0
#define WITH_MALESTORM 0

#define NAVESCEGLIEMASSIMO 1
#define PORTOSCEGLIEMASSIMO 1
#endif
