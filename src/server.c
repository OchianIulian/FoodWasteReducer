#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdlib.h>
#include <sqlite3.h>
#include "map.h"
#include "dbtransactions.h"


/*Portul folosit*/
#define PORT 2024
/*Cate tipuri de alimente poate stoca depozitul*/
#define MAX_AlIMENTS 1000;

/*codul de eroare returnat de anumite apeluri*/
extern int errno;

typedef struct {
    int id;/*1 pentru donator 0  pentru nevoias*/
    char nume[50];
    int cantity;
} Aliment;


/*depozitul de alimente*/
Map depozit;

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
void readData(sqlite3 *db){
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
        sprintf(depozit.keys[depozit.size], "%s", key);
        depozit.values[depozit.size] = value;
        depozit.size++;
    }

    sqlite3_finalize(stmt);
}

/*end database config*/

/* functie de convertire a adresei IP a clientului in sir de caractere */
char * conv_addr (struct sockaddr_in address)
{
  static char str[25];
  char port[7];

  /* adresa IP a clientului */
  strcpy (str, inet_ntoa (address.sin_addr));	
  /* portul utilizat de client */
  bzero (port, 7);
  /*sprintf actioneaza ca un printf doar ca outputul il copiaza in a bufferul 'port'*/
  sprintf (port, ":%d", ntohs (address.sin_port));	
  strcat (str, port);
  return (str);
}

/* realizeaza primirea si retrimiterea unui mesaj unui client */
int transactions(int client, char *msg, sqlite3 *db)
{
    Aliment receivedAliment;    
    memset(&receivedAliment, 0, sizeof(Aliment));
        
    /*s-a realizat conexiunea, se asteapta mesajul*/
    /*ne asiguram ca bufferul nu contine nimic*/
    bzero(msg, 100);
    printf("[server]asteptam alimentul...\n");
    fflush(stdout);

    /*citirea citim mesajul*/
    if(recv(client, &receivedAliment, sizeof(Aliment), 0)<=0){//ultimul parametru este setat pe 0 pentru o operațiune de recepție standard.
        perror("[server]Eroare la read() de la client");
        fflush(stdout);
        return -1;
    }

    if(receivedAliment.id == 0){
        printf("Se face o donatie catre un nevoias\n");
        /*se creaza structura de trimis catre nevoias*/
        Aliment aliment_de_trimis;
        aliment_de_trimis.id = 2;//de trimis
        if(getValue(&depozit, receivedAliment.nume) < receivedAliment.cantity){
            aliment_de_trimis.cantity = getValue(&depozit, receivedAliment.nume);
        } else {
            aliment_de_trimis.cantity = receivedAliment.cantity;
        }
        strcpy(aliment_de_trimis.nume, receivedAliment.nume);

        printf("Aliment de trimis: %s, %d\n", aliment_de_trimis.nume, aliment_de_trimis.cantity);


        /*se sterg elementele din depozitul local*/
        delete_values(&depozit, receivedAliment.nume, receivedAliment.cantity);
    } else {
        printf("S-a primit o donatie puternica\n");
        /*se adauga elementele in depozit*/
        add_items(&depozit, receivedAliment.nume, receivedAliment.cantity);
    }

    printf("Alimentul a fost receptionat...: Nume=%s; Cantitate=%d\n",
    receivedAliment.nume, receivedAliment.cantity);

    /*dupa ce s-au facut tranzactiile inseram si in baza de date*/
    insertOrUpdateData(db,receivedAliment.nume, getValue(&depozit, receivedAliment.nume));
    printf("S-au scris in baza de date: %s, %d;\n", receivedAliment.nume, getValue(&depozit, receivedAliment.nume));
    return 1;
}

int setup_server(){
    struct sockaddr_in server;/*structura folosita de server*/
    int sd;
    int optval = 1;
    /*creare socket*/
    if((sd = socket(AF_INET, SOCK_STREAM, 0)) == -1){
        perror("Eroare la crearea socketului\n");
        return errno;
    }
    /*setam pentru socket optiunea SO_REUSEADDR*/
    setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));

    /*pregatirea structurilor de date si initializarea lor cu 0*/
    bzero(&server, sizeof(server));

    /*umplem structura folosita de server*/
    /*stabilirea tipului de adresa a socketului/familiei socketului*/
    server.sin_family = AF_INET;
    /*acceptam orice adresa  si asiguram compatibilitatea pe orice arhitectura*/
    server.sin_addr.s_addr = htonl(INADDR_ANY);
    /*setam portul*/
    server.sin_port = htons(PORT);

     /*se atașează un socketul la adresa IP și portul specific*/
    if(bind(sd, (struct sockaddr *)&server, sizeof(struct sockaddr)) == -1){
        perror("[server]eroare la bind()\n");
        printf("Error code: %d\n", errno); 
        return errno;
    }
    /*punem serverul sa asculte daca vin clienti sa se conecteze, maxim 5 odata*/
    if(listen(sd, 5) == -1){
        perror("[server]Eroare la listen\n");
        return errno;
    }

    return sd;
}


int main(){
   
    struct sockaddr_in from;/*structura clientului care scrie*/

    fd_set readfds;/*multimea descriptorilor de citire*/
    fd_set actfds;/*multimea descriptorilor activi*/
    struct timeval tv;/*structura de timp pentru select()*/


    char msg[100];/*mesajul primit de la client*/
    char msgrasp[100]="";/*mesaj de raspuns pentru client*/
    int sd, client;/*socket descriptors*/
    int optval=1;/*optiune folosita pentru setsockopt()
                    1 inseamna ca optiunea e activata*/
    int fd;/*descriptor folosit pentru parcurgerea listelor de descriptori*/

    int nfds;/*numarul maxim de descriptor*/
    int len;/*lungimea structurii sockaddr_in*/

    /*Database*/
    sqlite3 *db;
    db = connect_to_database(db);
    readData(db);
    


    sd = setup_server();
    
    bzero(&from, sizeof(from));

    
    /*completam multimea de descriptori de citire*/
    FD_ZERO(&actfds);/*ne asiguram ca inital multimea e vida*/
    FD_SET(sd, &actfds);/*includem in multime socketul creat*/

    /*se va astepta timp de 1sec*/
    tv.tv_sec = 1;
    tv.tv_usec = 0;

    /*valoarea maxima a descriptorilor folositi*/
    nfds = sd;

    printf ("[server] Asteptam la portul %d...\n", PORT);
    fflush (stdout);

    /*servim in mod concurent clientii*/
    while (1)
    {
        /*ajustam multimea descriptorilor activi (efectiv utilizati)*/
        bcopy((char *)&actfds, (char *) &readfds, sizeof(readfds));//se copiaza readfds in actfds
        /*apelul select()*/
        if(select(nfds+1, &readfds, NULL, NULL, &tv)<0){//tratam cazurile doar de citire momentan
            perror("[server]Eroare la select()\n");
            return errno;
        }
        /*vedem daca e pregatit socketul pentru a-i accepta pe clienti*/
        if(FD_ISSET(sd, &readfds)){
            /*pregatirea structurii client*/
            len = sizeof(from);
            bzero(&from, sizeof(from));
            /*a venit un client, acceptam conexiunea*/
            if((client = accept(sd, (struct sockaddr*)&from, &len))<0){
                perror("[server]eroare la accept()\n");
                continue;;
            }
            /*ajustez vaoarea maxima de socket*/
            if(nfds<client){
                nfds=client;
            }
            /*includem in lista de descriptori activi si acest socket*/
            FD_SET(client, &actfds);
            printf("[server] S-a conectat clientul cu descriptorul %d, de la adresa %s.\n",client, conv_addr (from));
	        fflush (stdout);
        }
        /*vedem daca e pregatit vreun socket client pentru a transmite raspunsul*/
        for(fd = 0; fd<=nfds; ++fd){/*parcurgem multimea de descriptori*/
            /*este un socket de citire pregatit?*/
            if(fd != sd && FD_ISSET(fd, &readfds)){
                if(transactions(fd, msg, db)>0){
                    printf ("[server] S-a deconectat clientul cu descriptorul %d.\n",fd);
		            fflush (stdout);
		            close (fd);		/* inchidem conexiunea cu clientul */
		            FD_CLR (fd, &actfds);/* scoatem si din multime */
                }
            }
        }
    }
    sqlite3_close(db);
    return 0;
}