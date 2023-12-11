#include "map.h"
#include <stdio.h>
#include <string.h>


int getIndex(Map *map,char key[]){
    for (int i = 0; i < map->size; i++) { 
        if (strcmp(map->keys[i], key) == 0) { 
            return i; 
        } 
    } 
    return -1; // Key not found 
}
void insert(Map *map,char key[], int value){
    if(getIndex(map, key) == -1){
        strcpy(map->keys[map->size], key); 
        map->values[map->size] = value; 
        map->size++; 
    } else add_items(map, key, value);
}
void add_items(Map *map,char key[], int value){
    if(getIndex(map, key) == -1){
        insert(map, key, value);
    } else {
        map->values[getIndex(map, key)]+=value;
    }
}
void delete_values(Map *map,char key[], int value){
    int index = getIndex(map, key);
    if(index!=-1){
        if(value <= map->values[index]){
            map->values[index]-=value;
            printf("[Done]Tranzactie completa\n");
        } else {
            if(map->values[index] == 0){
                printf("Ne pare rau dar nu mai avem pe stoc elementul %s\n", key);
            } else if(map->values[index]){
                printf("Ne pare rau dar iti putem oferi doar %d din alimentul %s\n", map->values[index], key);
                map->values[index] = 0;
            }
        }
    } else {
        printf("elementul nu exista\n");
    }
}

void delete_key(Map *map, char key[]){
    /*gasim pozitica care trebuie eliminata*/
    int index = getIndex(map, key);
    /*o golim*/
    memset(map->keys[index], '\0', strlen(map->keys[index]));
    /*se muta toate alimentele de dupa valoarea modificata cu o pozitie inapoi*/
    for(int i=index; i<map->size; ++i){
        strcpy(map->keys[i], map->keys[i+1]);
        map->values[i] = map->values[i+1]; 
    }
    /*golim ultima pozitie duplicata*/
    memset(map->keys[map->size-1], '\0', strlen(map->keys[map->size-1]));
    /*decrementam size ul*/
    map->size--;
}

int get(Map *map,char key[]){
    int index = getIndex(map, key); 
    if (index == -1) { // Key not found 
        return -1; 
    } 
    else { // Key found 
        return map->values[index]; 
    } 
}
void printMap(Map *map){
    for (int i = 0; i < map->size; i++) { 
        printf("%s: %d\n", map->keys[i], map->values[i]); 
    } 
}