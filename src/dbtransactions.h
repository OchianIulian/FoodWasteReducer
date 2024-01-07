#ifndef DBTRANSACTIONS_H
#define DBTRANSACTIONS_H

#include <sqlite3.h>  // Include necessary headers
#include "map.h"

sqlite3 * connect_to_database(sqlite3 *db);
void insertOrUpdateData(sqlite3 *db, const char *key, int value);
void readData(sqlite3 *db, Map *map);

#endif