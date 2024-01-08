#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <string.h>
#include <stdbool.h>
#include <arpa/inet.h>
#include "map.h"

/*codul de eroare returnat de anumite apeluri*/
extern int errno;

/*portul de conectare la server*/
int port;

typedef struct {
    int id;
    char nume[50];
    int cantity;
} Aliment;

Map ListaAlimente;

int setup_server(char *server_address){
    int sd;/*socket descriptor*/
    struct sockaddr_in server;/*structura folosita pentru conectarea cu serverul*/ 

        /*cream socketul*/
    /*
        ultimul parametru e 0, ca valoare pentru protocol, sa fie ales automat de sistem
        TCP, UDP, IP, HTTP, SMTP
    */
    if((sd = socket(AF_INET, SOCK_STREAM, 0))==-1){
        perror("[nevoias]eroare la socket()\n");
        return errno;
    }

    /*umplem structura folosita pentru conectarea cu serverul*/
    /*stabilim familia*/
    server.sin_family = AF_INET;
    /*stabilim portul*/
    server.sin_port = htons(port);
    /*stabilim id ul*/
    server.sin_addr.s_addr = inet_addr(server_address);

    /*ne conectam la server*/
    if(connect(sd, (struct sockaddr *)&server, sizeof(struct sockaddr)) == -1){
        perror("[nevoias]eroare la connect\n");
        return errno;
    }

    return sd;
}

void cerere_nevoias(int sd){
    printf("sunt un nevoias\n");
    Aliment aliment_cerut;
    aliment_cerut.id = 0;//vine de la nevoias
    printf("Introdu numele produsului: ");
    fflush(stdout);
    scanf("%49s", aliment_cerut.nume);
    printf("Introdu cantitatea pentru %s:", aliment_cerut.nume);
    fflush(stdout);
    scanf("%d", &aliment_cerut.cantity);


    /*trimitem mesajul la server*/
    //ultimul paramentru e setat pe 0 pentru operatia de trimitere standard
    if(send(sd, &aliment_cerut, sizeof(Aliment), 0) <=0){
        perror("[client] Eroare la write\n");
        exit(errno);
    }

    /*pregatesc structura sa primeasca alementele cerute*/
    memset(&aliment_cerut,0, sizeof(Aliment));
    if(recv(sd, &aliment_cerut, sizeof(Aliment), 0) <= 0){
        perror("[nevoias]eroare la recv\n");
        exit(errno);
    }

    printf("Alimentul primit e:\n");
    printf("%s si cantitatea %d\n", aliment_cerut.nume, aliment_cerut.cantity);
}

void cerere_organizatie(int sd){
    printf("sunt o organizatie\n");
    
}

void transactions(int sd, bool is_oc){

    if(is_oc == true){
        cerere_organizatie(sd);
    } else {
        cerere_nevoias(sd);
    }
}

int main(int argc, char*argv[]){
    int sd;/*socket descriptor*/
    char msg_rec[100];/*mesajul primit*/

    bool is_oc;//verifica daca este organizatie caritabila
    

    /*exista toate argumentele in linia de comanda?*/
    if(argc < 3 || argc > 4){
        printf("sintaxa: %s [-oc] <adresa_server> <port>\n", argv[0]);
        return errno;
    }
    /*stabilim portul*/
    if(argc == 3){
        is_oc = false;
        port  = atoi(argv[2]);
        /*facem conexiunea si setam socketul*/
        sd = setup_server(argv[1]);
    } else if(argc == 4){
        if(strcmp(argv[1], "-oc") != 0){
            printf("sintaxa: %s [-oc] <adresa_server> <port>\n", argv[0]);
            return errno;
        }
        printf("E organizatie caritabila\n");
        is_oc = true;
        port = atoi(argv[3]);
        /*facem conexiunea si setam socketul*/
        sd = setup_server(argv[2]);
    }
    

    transactions(sd, is_oc);

    /*inchidem conexiunea*/
    close(sd);

    return 0;
}