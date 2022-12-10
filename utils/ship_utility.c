#include "../src/nave.h"
#include "../utils/loadShip.h"
#include "../config1.h"


int checkCapacity(loadShip* load){
    if(load->weightLoad == 0) return 0;
    return load->weightLoad;
}

int availableCapacity(loadShip* load){
    int currentCapacity = checkCapacity(loadShip* load)
    if(currentCapacity < 0) return -1;
    return (SO_CAPACITY - currentCapacity);
}

double generateCord(){
    // TO-DO implementare metodo che genere le coordinate
}

Products* generateArrayOfProducts(loadShip* list){

}

/*
 2 shm: 1 per il dump contenente il numero di navi e ogni posizione nell'array dump contiene
        le informazioni relative al dump di quella nave

        2 seconda shm per accedere alle merci dei porti BISOGNA SAPERE L'INDICI DEL PORTO CON CUI
        SCAMBIARE LE MERCI

*/
