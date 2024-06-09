/*


Rezumat: Am creeat fisierul client aferent serverului . Functii : stats , uptime si cmd<comanda>.



*/

#include <stdio.h> // biblioteca I/O
#include <stdlib.h> // standard pt functii
#include <string.h> // pentru siruri de caractere
#include <unistd.h> // standard 
#include <arpa/inet.h> // pentru adrese 
//def adresa IP
#define SERVER_IP "127.0.0.1" 
//def portului
#define SERVER_PORT 8888 


// functie trimitere info la server
void send_command(int sockfd, const char *command) {
    struct sockaddr_in server_addr; //structura server
    char buffer[1024]; // structura buffer mesaje

    // initializare structura server
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr);

    // trimitere comanda la server
    sendto(sockfd, command, strlen(command), 0, (struct sockaddr *)&server_addr, sizeof(server_addr));

    // afisare raspuns
    ssize_t recv_len = recvfrom(sockfd, buffer, 1024 - 1, 0, NULL, NULL);
    if (recv_len > 0) {
        buffer[recv_len] = '\0';
        printf("Server response: %s\n", buffer);
    }
}

int main() {
    int sockfd; // socketul clientului
    struct sockaddr_in server_addr; // structura de adresă a serverului

    // crearea socketului
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    //loop de comenzi
    while (1) {
        char command[1024]; // comanda primita de la user
        printf(">>> "); // 
        fgets(command, 1024, stdin); //citire comanda
        command[strcspn(command, "\n")] = '\0'; // eliminare newline

        // pentru iesire
        if (strcmp(command, "quit") == 0) {
            break;
        }

        // trimitere comanda la server
        send_command(sockfd, command);
    }

    // inchidere socket
    close(sockfd);

    return 0;
}
/*
Compilare:
user@DESKTOP-EPTB3M2:~$ gcc -o client1 client1.c
user@DESKTOP-EPTB3M2:~$ ./client1
>>> stats
Server response: CPU usage: 0.18% user, 0.13% sys, 99.69% idle
Memory usage: 1777260 kB used, 5165880 kB free, 6148200 kB available
>>> uptime
Server response: Server is UP for 0d 00h00min18sec
>>> uptime
Server response: Server is UP for 0d 00h00min21sec
>>> cmd:ls
Server response: Documents
a
c
client1
client1.c
monitor.sh
monitor_final.sh
program
program.c
program_proba
program_proba.c
server1
server1.c
subiect
subiect.c

>>> quit
user@DESKTOP-EPTB3M2:~$

*/