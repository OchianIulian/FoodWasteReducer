#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/*Portul folosit*/
#define PORT 2024

/*codul de eroare returnat de anumite apeluri*/
extern int errno;

typedef struct {
    int id;/*1 pentru donator 0  pentru nevoias*/
    char nume[50];
    int cantity;
} Aliment;


int main(){
    struct sockaddr_in server;/*structura folosita de server*/
    struct sockaddr_in from;/*structura clientului*/
    struct sockaddr_in to;
    char msg[100];/*mesajul primit de la client*/
    char msgrasp[100]="";/*mesaj de raspuns pentru client*/
    int sd;/*socket descriptor*/


    /*crearea unui socket*/
    if((sd = socket(AF_INET, SOCK_STREAM, 0)) == -1){
        perror("Eroare la crearea socketului\n");
        return errno;
    }

    /*pregatirea structurilor de date si initializarea lor cu 0*/
    bzero(&server, sizeof(server));
    bzero(&from, sizeof(from));

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

    /*servim in mod iterativ clientii*/
    while (1)
    {
        int client;
        int length = sizeof(from);

        printf("[server]Asteptam la portul %d...", PORT);
        fflush(stdout);

        /*acceptam un client ()*/
        client = accept(sd, (struct sockaddr*)&from, &length);

        /*eroare la acceptarea unui client*/
        if(client<0){
            perror("[server]Eroare la accept()");
            fflush(stdout);
            continue;
        }

        Aliment receivedAliment;    
        memset(&receivedAliment, 0, sizeof(Aliment));
        
        /*s-a realizat conexiunea, se asteapta mesajul*/
        /*ne asiguram ca bufferul nu contine nimic*/
        bzero(msg, 100);
        printf("[server]asteptam mesajul...");
        fflush(stdout);

        /*citirea citim mesajul*/
        if(recv(client, &receivedAliment, sizeof(Aliment), 0)<=0){//ultimul parametru este setat pe 0 pentru o operațiune de recepție standard.
            perror("[server]Eroare la read() de la client");
            fflush(stdout);
            continue;
        }

        printf("Alimentul a fost receptionat...: Nume=%s; Cantitate=%d\n",
                 receivedAliment.nume, receivedAliment.cantity);
        //send(client, msg, strlen(msg), 0);
        close(client);
    }
    


    return 0;
}