# FoodWasteReducer

# Compilare server.c
gcc server.c map.c dbtransactions.c -o server -lsqlite3 

# Compilare nevoias.c
gcc nevoias.c map.c -o nevoias 

# Compilare donator.c
gcc donator.c -o donator 

# Rulare executabil donator
./donator 0 2024

# Rulare executabil nevoias
./nevoias 0 2024

# Rulare executabil organizatie caritabila
./donator -oc 0 2024

# Rulare executabil server
./server
