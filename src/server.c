#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <string.h>
#include "map.h"
#include "dbtransactions.h"
#include <stdlib.h>
#include <sqlite3.h>


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

        if(send(client, &aliment_de_trimis, sizeof(Aliment), 0 <= 0)){
            perror("[server] Eroare la trimiterea alimentului catre nevoias\n");
            exit(errno);
        }

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
    readData(db, &depozit);
    


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