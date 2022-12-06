#ifndef SHM_UTILITY_H

#define SHM_UTILITY_H

#define SHMERRGET 7
#define SHMERRAT  8
#define SHMERRDT  9
#define SHMERRCTL 10


int createShm(int key, /* incompleto: tipo di dato con cui creare la shm , */int shmSize);

/* tipo dato */ getShmAddress(int shmid, int flag);

void shmDetach(/*tipo dato*/);
int removeShm(int shmid);

#endif