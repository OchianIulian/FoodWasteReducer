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

/*portul*/
int port;

typedef struct {
    int id;
    char nume[50];
    int cantity;
} Aliment;

int setup_server(char *server_address){
    int sd;//descriptorul de socket
    struct sockaddr_in server;//structura pentru server

    /*ultimul parametru e 0, ca valoare pentru protocol, sa fie ales automat de sistem*/
    if((sd = socket(AF_INET, SOCK_STREAM, 0)) == -1){
        perror("eroare la crearea socketului");
        return errno;
    }

    /*umplem structura folosita pentru realizarea conexiunii cu serverul*/
    /*familia socketului*/
    server.sin_family = AF_INET;
    /*portul*/
    server.sin_port = htons(port);
    /*adresa IP a serverului*/
    server.sin_addr.s_addr = inet_addr(server_address);

    /*ne conectam la server*/
    if(connect(sd, (struct sockaddr*)&server, sizeof(struct sockaddr)) == -1){
        perror("[client]Eroare la connect\n");
        return errno;
    }
    return sd;
}

int main(int argc, char *argv[]){
    int sd;
    char msg[100];//mesajul de trimis

    /*verificam daca exista 3 argumente in apelare*/
    if(argc!=3){
        printf("Sintaxa: %s <adresa_server> <port>\n", argv[0]);
        return errno;
    }

    /*stabilim portul*/
    port = atoi(argv[2]);
    sd = setup_server(argv[1]);
    

    Aliment alimentSurplus;
    alimentSurplus.id = 1;//vine de la donator
    printf("Introdu numele produsului: ");
    fflush(stdout);
    scanf("%49s", alimentSurplus.nume);
    printf("Introdu cantitatea pentru %s:", alimentSurplus.nume);
    fflush(stdout);
    scanf("%d", &alimentSurplus.cantity);


    /*trimitem mesajul la server*/
    //ultimul paramentru e setat pe 0 pentru operatia de trimitere standard
    if(send(sd, &alimentSurplus, sizeof(Aliment), 0) <=0){
        perror("[client] Eroare la write\n");
        return errno;
    }

    close(sd);

}