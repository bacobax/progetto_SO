#ifndef LOAD_SHIP_H
#define LOAD_SHIP_H

struct productNode_ { /* nodo utilizzato nella lista */
    int id_product;   /* identificativo del nodo nella lista */
    int product_type; /* tipo di merce nella lista */
    int weight;
    int expirationTime;
    struct productNode_* next;
};
typedef struct productNode_* Product;

struct load { /* lista implementata per il carico della nave */
    Product first;
    Product last;
    int length;
    int weightLoad;
};
typedef struct load* loadShip;

loadShip initLoadShip();

void addProduct(loadShip list, Product p);

Product findProduct(loadShip list, int product_type);

int getProductId(loadShip list, int product_type);

void removeProduct(loadShip list, int idProduct);

void printLoadShip(loadShip list);

void freeLoadShip(loadShip list);

#endif
