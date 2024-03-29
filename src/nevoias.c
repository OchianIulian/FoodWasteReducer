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
extern int errno;//cu errno poti obtine inofrmatii despre eroarea rezultata

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
    if((sd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP))==-1){
        perror("[nevoias]eroare la socket()\n");
        return errno;
    }

    /*umplem structura folosita pentru conectarea cu serverul*/
    /*stabilim familia*/
    server.sin_family = AF_INET;//protocol pentru versiunea 4 IPv4 (192.168.0.1)
    /*stabilim portul*/
    server.sin_port = htons(port);//converteste din formatul gazda in formatul retea
    /*stabilim id ul*/
    server.sin_addr.s_addr = inet_addr(server_address);//converteste din ascii in binar pt a putea fi folosit in retea

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

    /*Adaugam alimentul la Lista de alimente pe care o vom trimite*/
    ListaAlimente.id = 0;//marcam ca pleaca de la nevoias
    insert(&ListaAlimente, aliment_cerut.nume, aliment_cerut.cantity);


    /*trimitem mesajul la server*/
    //ultimul paramentru e setat pe 0 pentru operatia de trimitere standard
    if(send(sd, &ListaAlimente, sizeof(Map), 0) <=0){
        perror("[client] Eroare la write\n");
        exit(errno);
    }

    /*pregatesc structura sa primeasca alementele cerute*/
    memset(&ListaAlimente,0, sizeof(Map));
    if(recv(sd, &ListaAlimente, sizeof(Map), 0) <= 0){
        perror("[nevoias]eroare la recv\n");
        exit(errno);
    }

    printf("Am primit:\n");
    printMap(&ListaAlimente);

    char *msg = "[nevoias] Multumesc din suflet pentru donatii";
    if(send(sd, msg, strlen(msg), 0) <=0){
        perror("[client] Eroare la write\n");
        exit(errno);
    }
}

void cerere_organizatie(int sd){
    printf("sunt o organizatie\n");
    int nrAliments;//numarul maxim de alimente pe care le poate cere
    printf("Introdu cate produse vrei sa ceri: ");
    fflush(stdout);
    scanf("%d", &nrAliments);
    if(nrAliments > 10){
        printf("Iti putem oferi doar 10 alimente e ok pt tine? Y/N\n");
        char response;
        scanf(" %c", &response);
        if(response == 'Y'){
            printf("E bine poftim\n");
            nrAliments = 10;
        } else {
            printf("Ok la revedere\n");
            exit(errno);
        }
    }

    ListaAlimente.id = 0;//marcam ca pleaca de la nevoias

    int i=1;
    while(nrAliments > 0){
        Aliment aliment_cerut;
        aliment_cerut.id = 0;//vine de la nevoias
        printf("Introdu numele produsului cu id %d: ", i);
        fflush(stdout);
        scanf("%49s", aliment_cerut.nume);
        printf("Introdu cantitatea pentru %s: ", aliment_cerut.nume);
        fflush(stdout);
        scanf("%d", &aliment_cerut.cantity);

        /*Adaugam alimentele in Lista de trimis*/
        insert(&ListaAlimente, aliment_cerut.nume, aliment_cerut.cantity);

        ++i;
        --nrAliments;
    }

    /*trimitem alimentele catre server*/
    //ultimul paramentru e setat pe 0 pentru operatia de trimitere standard
    if(send(sd, &ListaAlimente, sizeof(Map), 0) <=0){
        perror("[client] Eroare la write\n");
        exit(errno);
    }

    /*pregatesc structura sa primeasca alementele cerute*/
    memset(&ListaAlimente,0, sizeof(Map));
    if(recv(sd, &ListaAlimente, sizeof(Map), 0) <= 0){
        perror("[nevoias]eroare la recv\n");
        exit(errno);
    }

    printf("Am primit:\n");
    printMap(&ListaAlimente);

    char *msg = "[oraganizatie] Multumim din suflet pentru donatii";
    if(send(sd, msg, strlen(msg), 0) <=0){
        perror("[client] Eroare la write\n");
        exit(errno);
    }
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