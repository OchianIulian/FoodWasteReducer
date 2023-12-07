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

int main(int argc, char *argv[]){
    int sd;//descriptorul de socket
    struct sockaddr_in server;//structura pentru server
    char msg[100];//mesajul de trimis

    /*verificam daca exista 3 argumente in apelare*/
    if(argc!=3){
        printf("Sintaxa: %s <adresa_server> <port>\n", argv[0]);
        return errno;
    }

    /*stabilim portul*/
    port = atoi(argv[2]);

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
    server.sin_addr.s_addr = inet_addr(argv[1]);

    /*ne conectam la server*/
    if(connect(sd, (struct sockaddr*)&server, sizeof(struct sockaddr)) == -1){
        perror("[client]Eroare la connect\n");
        return errno;
    }

    /*citirea mesajului*/
    bzero(msg, 100);
    printf("[client]Introduceti un nume:  ");
    fflush(stdout);
    read(0, msg, 100);

    /*trimitem mesajul la server*/
    //ultimul paramentru e setat pe 0 pentru operatia de trimitere standard
    if(send(sd, msg, 100, 0) <=0){
        perror("[client] Eroare la write\n");
        return errno;
    }

    /*citirea mesajului*/
    bzero(msg, 100);
    if(recv(sd, msg, 100, 0) < 0){
        perror("[nevoias]eroare la recv()\n");
        return errno;
    }

    printf("[nevoias]Mesajul primit este: %s\n", msg);

    close(sd);

}