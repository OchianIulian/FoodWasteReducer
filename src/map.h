#ifndef MAP_H
#define MAP_H

#define MAX_SIZE 100

typedef struct  {
    int size;
    char keys[MAX_SIZE][100];
    int values[MAX_SIZE];
} Map;


/*definim functiile*/
int getIndex(Map *map,char key[]);
void insert(Map *map,char key[], int value);
void add_items(Map *map,char key[], int value);
void delete_values(Map *map,char key[], int value);
int get(Map *map,char key[]);
void printMap(Map *map);








#endif 