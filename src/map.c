#include "map.h"
#include <stdio.h>

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