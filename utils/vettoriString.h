#ifndef VETTORI_STRING_H

#define VETTORI_STRING_H
#define length(array) (sizeof(array)/sizeof(array[0]))
/* Addizione */
typedef struct stringNode_{
    char* el;
    struct stringNode_* next;
} stringNode;

typedef struct stringList_{
    stringNode* first;
    stringNode* last;
    int length;
}stringList;

void stringPush(stringList* list, char* i);

void stringStampaLista(stringList* lista);

stringList* stringInit();
stringList* stringInitFromArray(char** a, int length);
/* ritorna la stringa trovata, se l'idx è out of bound ritorna NULL */
char* stringElementAt(stringList* l ,int idx);

char* stringFindFirst(stringList* l , int(*f)(char*));

/* ritorna la lista filtrata */
stringList* stringFindAll(stringList* l , int(*f)(char*));

void stringFreeList(stringList* lista);

void stirngRemove(stringList* lista, int idx);
#endif