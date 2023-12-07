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

int main(int argc, char*argv[]){
    int sd;/*socket descriptor*/
    struct sockaddr_in server;/*structura folosita pentru conectarea cu serverul*/
    char msg_rec[100];/*mesajul primit*/

    /*exista toate argumentele in linia de comanda?*/
    if(argc != 3){
        printf("sintaxa: %s <adresa_server> <port>\n", argv[0]);
        return errno;
    }

    /*stabilim portul*/
    port  = atoi(argv[2]);
    printf("inainte de socket()\n");
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
    server.sin_addr.s_addr = inet_addr(argv[1]);

    printf("inainte de connect\n");

    /*ne conectam la server*/
    if(connect(sd, (struct sockaddr *)&server, sizeof(struct sockaddr)) == -1){
        perror("[nevoias]eroare la connect\n");
        return errno;
    }

    // char msg[100];
    // bzero(msg, 100);
    /* Message to be sent to the server after connection */
    char msg_to_server[] = "Hello from nevoias";

    /* Send a message to the server after connecting */
    if (send(sd, msg_to_server, sizeof(msg_to_server), 0) <= 0) {
        perror("[nevoias] Error sending message to server\n");
        return errno;
    }

    
    /**/
    printf("inainte de recv\n");
    /*citirea mesajului*/
    bzero(msg_rec, 100);
    if(recv(sd, msg_rec, 100, 0) < 0){
        perror("[nevoias]eroare la recv()\n");
        return errno;
    }

    printf("[nevoias]Mesajul primit este: %s\n", msg_rec);

    /*inchidem conexiunea*/
    close(sd);

    return 0;
}