#include "dbtransactions.h"
#include <stdlib.h>
#include <sqlite3.h>
#include <stdio.h>
#include "map.h"


/*database transactions*/
/*Create connection to database*/
sqlite3 * connect_to_database(sqlite3 *db){
    int rc;//return code
    rc = sqlite3_open("depozit.db", &db);
	
	if(rc != SQLITE_OK){
		fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
      	return NULL;
	} else {
		fprintf(stderr, "Opened database successfully\n");
	}

    char *sql = "CREATE TABLE IF NOT EXISTS MapData (Key TEXT PRIMARY KEY NOT NULL, Value INT NOT NULL);";

    rc = sqlite3_exec(db, sql, 0, 0, 0);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Eroare la crearea tabelului: %s\n", sqlite3_errmsg(db));
    } else {
        fprintf(stdout, "Tabelul 'MapData' a fost creat cu succes sau exista deja!\n");
    }
	return db;
}


void insertOrUpdateData(sqlite3 *db, const char *key, int value){

    if (db == NULL || key == NULL) {
        fprintf(stderr, "Invalid parameters or NULL database connection.\n");
        return;
    }

    char sql[100];//comanda sql 
    sqlite3_stmt *stmt;//e folosit pentru a executa o interogare
    int rc;

    //verificam daca exista deja o intrare cu aceeasi cheie
    sprintf(sql, "SELECT Value FROM MapData WHERE Key = '%s';", key);
    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);//asociaza statementului interogarea
    if(rc!=SQLITE_OK){
        fprintf(stderr, "Eroare la pregatirea interogarii SELECT: %s\n", sqlite3_errmsg(db));
        return;
    }

    rc = sqlite3_step(stmt);//executa statement ul
    /*SQLITE_ROW INFORMEAZA CA EXISTA CEL PUTIN O linie de rezultat disponibila pentru prelucrare*/
    if(rc == SQLITE_ROW){
        // Cheia există, putem face UPDATE la valoarea asociată
        sprintf(sql, "UPDATE MapData SET Value = %d WHERE Key = '%s';", value, key);
    } else {
        // Cheia nu există, inserăm o nouă pereche key-value
        sprintf(sql, "INSERT INTO MapData (Key, Value) VALUES ('%s', %d);", key, value);
    }

    sqlite3_finalize(stmt);//elibereaza resursele 

    // Executăm comanda SQL adecvată (UPDATE sau INSERT)
    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if(rc != SQLITE_OK){
        fprintf(stderr, "Eroare la pregatirea interogarii UPDATE/INSERT: %s\n", sqlite3_errmsg(db));
        return;
    }

    rc = sqlite3_step(stmt);
    if(rc != SQLITE_DONE){
        fprintf(stderr, "Eroare la executia interogarii UPDATE/INSERT: %s\n", sqlite3_errmsg(db));
    }
    printf("S-a adaugat produsul in baza de date, totul e ok\n");
    sqlite3_finalize(stmt);
}

// Functie pentru citirea datelor din baza de date si salvarea lor in structura Map
void readData(sqlite3 *db, Map *depozit){
    sqlite3_stmt *stmt;
    const char *sql = "SELECT Key, Value FROM MapData;";

    //Executam interogareaSQL
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        fprintf(stderr, "Eroare la pregatirea interogarii: %s\n", sqlite3_errmsg(db));
        return;
    }

    // Citim rezultatele interogarii si le salvam in structura Map/depozit
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        const unsigned char *key = sqlite3_column_text(stmt, 0);
        int value = sqlite3_column_int(stmt, 1);

        // Salvam datele in structura Map
        sprintf(depozit->keys[depozit->size], "%s", key);
        depozit->values[depozit->size] = value;
        depozit->size++;
    }

    sqlite3_finalize(stmt);
}

/*end database config*/