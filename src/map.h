#ifndef MAP_H
#define MAP_H

#define MAX_SIZE 100

typedef struct  {
    int size;
    char keys[MAX_SIZE][100];
    int values[MAX_SIZE];
} Map;


/*definim functiile*/
/*returneaza indexul elementului cu cheia key in cele 2 tablouri din structura*/
int getIndex(Map *map,char key[]);
/*insereaza un element nou in map*/
void insert(Map *map,char key[], int value);
/*adauga elemente in values la un produs deja existent
*daca un element nu exista, se adauga in map, iar valoarea sa va fi cea specificata in value*/
void add_items(Map *map,char key[], int value);
/*scade un numar egal cu value din values din map*/
void delete_values(Map *map,char key[], int value);
/*sterge un element din lista*/
void delete_key(Map *map,char key[]);
/*returneaza valoarea unui element cu cheia key*/
int get(Map *map,char key[]);
/*afiseaza elementele map ului*/
void printMap(Map *map);

#endif 