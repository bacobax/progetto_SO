#ifndef LOAD_SHIP_H
#define LOAD_SHIP_H
#include "../src/porto.h"
struct productNode_ { /* nodo utilizzato nella lista */
    int product_type; /* tipo di merce nella lista */
    int weight;
    int expirationTime;
    int distributionDay;
    int portID;
    struct productNode_* next;
};
typedef struct productNode_* Product;

struct load { /* lista implementata per il carico della nave */
    Product first;
    Product last;
    int length;
    
};
typedef struct load* loadShip;

loadShip initLoadShip();

Product findProduct(loadShip list, int product_type);

int getProductId(loadShip list, int product_type);


void printLoadShip(loadShip list, FILE* stream);

void freeLoadShip(loadShip list);
Product productAt(loadShip l, int idx);
Product initProduct(int weight, int type, int expTime, int portID, int dd);
int weigthSum(loadShip l);
int getProductExpTime(Product p);

#endif
