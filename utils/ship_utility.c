#include <stdio.h>
#include <stdlib.h>
#include "../src/nave.h"
#include "../src/porto.h"
#include "../config1.h"
#include "./support.h"
#include "../src/dump.h"
#include "./msg_utility.h"
#include "./sem_utility.h"
#include "./shm_utility.h"
#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>



int availableCapacity(Ship ship)
{
    return (SO_CAPACITY - ship->weight);
}

double generateCord()
{
    double range, div;

    range = (SO_LATO); /* max-min */
    div = RAND_MAX / range;
    return (rand() / div);
}

void initArray(Product* products){
    int i;
    for(i=0; i<SO_CAPACITY; i++){
        products[i].product_type = -1;
        products[i].expirationTime = -1;
        products[i].weight = -1;
    }
}

Ship initShip(int shipID)
{
    Ship ship;
    int shipShmId;
    printf("Nave: sono dentro alla initShip\n");
    if (signal(SIGUSR1, quitSignalHandler) == SIG_ERR)
    { /* imposto l'handler per la signal SIGUSR1 */
        perror("Error trying to set a signal handler for SIGUSR1");
        exit(EXIT_FAILURE);
    }

    /* inizializziamo la nave in shm*/

    printf("Nave: sto per fare useShm in initShip\n");
    shipShmId = useShm(SSHMKEY, sizeof(struct ship) * SO_NAVI, errorHandler);
    printf("Nave: ship useShm fatta\n");

    ship = ((struct ship*) getShmAddress(shipShmId, 0, errorHandler)) + shipID;
    ship->shipID = shipID;
    ship->x = generateCord();
    ship->y = generateCord();
    ship->weight = 0;
    initArray(ship->products); /* inizializzo l'array con tutti i valori a -1*/

    printf("Nave: nave con id:%d inizializzata\n", ship->shipID);

    return ship;
}

void printLoadShip(Product* products){
    int i;
    for(i=0; i<SO_CAPACITY; i++){
        /*
        if(products[i].product_type == -1) break;
        
        */
        printf("\nProduct type:%d, Expiration time: %d, Weight: %d", products[i].product_type, products[i].expirationTime, products[i].weight);
    }
    printf("\n");
}

void printShip(Ship ship)
{

    int resSemID = useSem(RESPRINTKEY, errorHandler);

    mutex(resSemID, LOCK, errorHandler);

    printf("[%d]: Nave\n", ship->shipID);

    printf("coords: [x:%f, y:%f]\n", (ship->x), (ship->y));

    printf("ton trasporati:%d\n", ship->weight);

    printf("carico trasportato:\n");
    printLoadShip(ship->products);

    printf("______________________________________________\n");

    mutex(resSemID, UNLOCK, errorHandler);
}

int addProduct(Ship ship, Product p){
    int i;
    int aviableCap;
    Product *products = ship->products;
    aviableCap = availableCapacity(ship);
    if (aviableCap >= p.weight)
    {
        printf("Nave: c'è abbastanza capienza per un prodotto che pesa %d, capienza: %d\n" , p.weight, aviableCap);
        for (i = 0; i < SO_CAPACITY; i++)
        {
            /*
                inserisco il prodotto nella prima posizione dell'array che trovo in cui
                product_type = 0
                per non inizializzare tutto l'array a -1 suggerisco di far partire tutti i prodotti
                con un product_type da 1.
            */
            if (products[i].product_type == -1)
            {
                products[i].product_type = p.product_type;
                products[i].expirationTime = p.expirationTime;
                products[i].weight = p.weight;
                ship->weight = ship->weight + p.weight;
                break;
            }
        }
    }
    else
    {
        printf("Nave: non c'è abbastanza capienza per un prodotto che pesa %d, capienza: %d\n" , p.weight, aviableCap);

        return -1;
    }
}

int compareProducts(Product p1, Product p2){
    if((p1.product_type == p2.product_type) && (p1.expirationTime == p2.expirationTime) && (p1.weight == p2.weight))
        return 0;
    else
        return -1;    
}

int findProduct(Product* products, Product p){
    int i;
    for(i=0; i<SO_CAPACITY; i++){
        if(compareProducts(products[i], p) == 0){
            return i;
        }
    }
    return -1;
}

int removeProduct(Ship ship, int product_index){
    int i;
    Product* products = ship->products;
    
    if(product_index<0 || product_index>SO_CAPACITY) return -1;

    for(i=0; i<SO_CAPACITY; i++){
        if(i == product_index){
            products[i].product_type = 0;
            products[i].expirationTime = 0;
            products[i].weight = 0;
            return 0;
        }
    }
}

void updateExpTimeShip(Ship ship){
    int i;
    Product* products = ship->products;

    for(i=0; i<SO_CAPACITY; i++){
        if(products[i].product_type == -1) break;

        products[i].expirationTime += -1;

        if(products[i].expirationTime == 0){
            addExpiredGood(products[i].weight, products[i].product_type, SHIP);
            removeProduct(ship, i);
        }
    }
}

/*
void travel(Ship ship, int portID)
{

    Port p;
    double dt_x, dt_y, spazio, nanosleep_arg;

    int portShmId = useShm(PSHMKEY, SO_PORTI * sizeof(struct port), errorHandler);  prendo l'id della shm del porto 

    p = ((Port)getShmAddress(portShmId, 0, errorHandler)) + portID;  prelevo la struttura del porto alla portID-esima posizione nella shm 

     imposto la formula per il calcolo della distanza

    dt_x = p->x - ship->x;
    dt_y = p->y - ship->y;

    spazio = sqrt(pow(dt_x, 2) + pow(dt_y, 2));
    
        spazio/SO_SPEED è misurato in giorni (secondi), quindi spazio/SO_SPEED*1000000000 sono il numero di nanosecondi per cui fare la sleep
    
    nanosecsleep((long)((spazio / SO_SPEED) * NANOS_MULT));

     Dopo aver fatto la nanosleep la nave si trova esattamente sulle coordinate del porto
       quindi aggiorniamo le sue coordinate
    
   
    ship->x = p->x;
    ship->y = p->y;
}
*/