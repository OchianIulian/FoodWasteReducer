# FoodWasteReducer

FoodWasteReducer este o aplicație client-server proiectată pentru a conecta persoanele nevoiașe cu organizațiile caritabile, având ca scop reducerea deșeurilor alimentare prin redistribuirea hranei excedentare către cei care au nevoie de ea.

Caracteristici
- Arhitectură Client-Server: Implementează un model client-server pentru comunicarea între utilizatorii în nevoie și organizațiile caritabile.
- Listare Alimente: Utilizatorii pot lista alimente excedentare pe care doresc să le doneze sau să le primească.
- Algoritm de Potrivire: Utilizează un algoritm de potrivire pentru a conecta donatorii cu destinatarii in functie de disponibilitatea alimentelor și preferințe.
- Comunicare în Timp Real: Oferă funcționalitate de mesagerie în timp real pentru ca utilizatorii să comunice și să coordoneze donațiile de alimente.

## Rulare proiect(toate comenzile de mai jos trebuie rulate)
### Compilare server.c
```
gcc server.c map.c dbtransactions.c -o server -lsqlite3 
```
### Compilare nevoias.c
```
gcc nevoias.c map.c -o nevoias 
```
### Compilare donator.c
```
gcc donator.c -o donator 
```
### Rulare executabil donator
```
./donator 0 2024
```
### Rulare executabil nevoias
```
./nevoias 0 2024
```
### Rulare executabil organizatie caritabila
```
./donator -oc 0 2024
```
### Rulare executabil server
```
./server
```
