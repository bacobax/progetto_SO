typedef struct productNode_ {
    int id;
    int weight;
    int expirationTime;
    struct productNode_* next;
} Product;

typedef struct load{
    Product* first;
    Product* last;
    int length;
    int weightLoad;
} loadShip;

loadShip* initLoadShip();

void addProduct(loadShip* list, Product* p);

Product* findProduct(loadShip* list, int idProduct);

void removeProduct(loadShip* list, int idProduct);

void printLoadShip(loadShip* list);

void freeLoadShip(loadShip* list);

