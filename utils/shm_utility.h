#ifndef SHM_UTILITY_H

#define SHM_UTILITY_H

#define SHMERRGET 7
#define SHMERRAT  8
#define SHMERRDT  9
#define SHMERRCTL 10

#include <sys/types.h>


int createShm(int key, size_t shmSize, void (*errorHandler)(int err));

int useShm(int key, size_t shmSize, void (*errorHandler)(int err));

void* getShmAddress(int shmid, int flag, void (*errorHandler)(int err));

void shmDetach(void* addrToRemove, void (*errorHandler)(int err));

void removeShm(int shmid, void (*errorHandler)(int err));

#endif
