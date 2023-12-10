#include "map.h"
#include <stdio.h>

int main() 
{ 
    Map *map;
    insert(map,"Geeks", 5); 
    insert(map,"GFG", 3); 
    insert(map,"GeeksforGeeks", 7); 
  
    printf("Value of complete Map: \n"); 
    printMap(map); 
  
    printf("\nValue of apple: %d\n", get(map, "GFG")); 
    printf("Index of GeeksforGeeks: %d\n", 
    getIndex(map, "GeeksforGeeks")); 
  
    return 0; 
}