#ifndef SUPPLIES_H
#define SUPPLIES_H
#include "../config1.h"

typedef struct supplies {
    int magazine[SO_DAYS][SO_MERCI];
    int expirationTimes[SO_DAYS*SO_MERCI];
}Supplies;

void fillExpirationTime(Supplies* S);

void fillMagazine(Supplies* S, int day, int* supplies);


int getExpirationTime(Supplies S, int tipoMerce, int giornoDistribuzione); /*//TODO: da testare*/

void printSupplies(Supplies S);

void decrementExpTimes(Supplies* S);

void removeExpiredGoods(Supplies* S);

#endif
