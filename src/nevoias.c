#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <string.h>

/*codul de eroare returnat de anumite apeluri*/
extern int errno;

/*portul de conectare la server*/
int port;

typedef struct {
    int id;
    char nume[50];
    int cantity;
} Aliment;

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

void transactions(int sd){

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

int main(int argc, char*argv[]){
    int sd;/*socket descriptor*/
    char msg_rec[100];/*mesajul primit*/

    /*exista toate argumentele in linia de comanda?*/
    if(argc != 3){
        printf("sintaxa: %s <adresa_server> <port>\n", argv[0]);
        return errno;
    }
    /*stabilim portul*/
    port  = atoi(argv[2]);
    /*facem conexiunea si setam socketul*/
    sd = setup_server(argv[1]);

    transactions(sd);

    /*inchidem conexiunea*/
    close(sd);

    return 0;
}