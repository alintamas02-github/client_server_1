/*


Rezumat: Fisierul pentru server , are 3 atributii : uptime , stats si orice comanda de linux precum ls.


*/




#include <stdio.h> // biblioteca I/O
#include <stdlib.h> // standard pt functii
#include <string.h>// pentru siruri de caractere
#include <unistd.h> // standard 
#include <arpa/inet.h> // pentru adrese 
#include <time.h>  //pentru var de timp
//def port
#define SERVER_PORT 8888
//def maxim size
#define MAX_MSG_SIZE 1024

time_t start_time; /// var pentru start server
//variabile definite pentru stats
long cpu_user, cpu_nice, cpu_sys, cpu_idle;
long total_cpu_time;
double cpu_user_percentage, cpu_sys_percentage, cpu_idle_percentage;
long mem_total, mem_free, mem_available;
long mem_used;
double mem_used_percentage, mem_free_percentage;

// obtinere timpul actual
void get_uptime(char *uptime_str) {
    time_t uptime_seconds = time(NULL) - start_time; //calculare timp
    int days = uptime_seconds / (24 * 3600); //zile
    int hours = (uptime_seconds % (24 * 3600)) / 3600; //ore
    int minutes = (uptime_seconds % 3600) / 60; //minute
    int seconds = uptime_seconds % 60; //secunde
    sprintf(uptime_str, "Server is UP for %dd %02dh%02dmin%02dsec", days, hours, minutes, seconds); //print
}

// functie pentru stats
void get_stats(char *stats_str) {
    FILE *fp;
    char buffer[MAX_MSG_SIZE];

    // statsusul CPU
    fp = fopen("/proc/stat", "r");
    if (fp != NULL) {
        fgets(buffer, sizeof(buffer), fp);
        fclose(fp);

       // CPU info
        sscanf(buffer, "cpu %ld %ld %ld %ld", &cpu_user, &cpu_nice, &cpu_sys, &cpu_idle);
        total_cpu_time = cpu_user + cpu_nice + cpu_sys + cpu_idle;
        cpu_idle_percentage = (double)cpu_idle / total_cpu_time * 100;
        cpu_user_percentage = (double)cpu_user / total_cpu_time * 100;
        cpu_sys_percentage = (double)cpu_sys / total_cpu_time * 100;
    }

    // file pentru citire memoriei
    fp = fopen("/proc/meminfo", "r");
    if (fp != NULL) {
        while (fgets(buffer, sizeof(buffer), fp) != NULL) {
            if (strstr(buffer, "MemTotal:") != NULL) {
                sscanf(buffer, "MemTotal: %ld kB", &mem_total);
            } else if (strstr(buffer, "MemFree:") != NULL) {
                sscanf(buffer, "MemFree: %ld kB", &mem_free);
            } else if (strstr(buffer, "MemAvailable:") != NULL) {
                sscanf(buffer, "MemAvailable: %ld kB", &mem_available);
            }
        }
        fclose(fp);

        // calculeaza memoria folosita
        mem_used = mem_total - mem_free;
        mem_used_percentage = (double)mem_used / mem_total * 100;
        mem_free_percentage = (double)mem_free / mem_total * 100;
    }

     // print stats
    sprintf(stats_str, "CPU usage: %.2f%% user, %.2f%% sys, %.2f%% idle\n"
                        "Memory usage: %ld kB used, %ld kB free, %ld kB available",
            cpu_user_percentage, cpu_sys_percentage, cpu_idle_percentage,
            mem_used, mem_free, mem_available);
}
// functie pentru a executa o comandavoid execute_command(const char *cmd, char *result) {
    FILE *fp;
    char buffer[MAX_MSG_SIZE];
    fp = popen(cmd, "r");
    if (fp != NULL) {
        while (fgets(buffer, sizeof(buffer), fp) != NULL) {
            strcat(result, buffer);
        }
        pclose(fp);
    }
}

int main() {
    int sockfd; //def socket
    struct sockaddr_in server_addr, client_addr;//structura adreselor
    socklen_t client_len = sizeof(client_addr);
    char buffer[MAX_MSG_SIZE];

    // retine momentul de start
    start_time = time(NULL);

    // creare socket
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

   // seteaza adresa serverului
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(SERVER_PORT);

    // gasire socket
    if (bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Socket bind failed");
        exit(EXIT_FAILURE);
    }

    printf("Server running...\n");

    while (1) {
       // primeste mesajul de la client
        ssize_t recv_len = recvfrom(sockfd, buffer, MAX_MSG_SIZE - 1, 0, (struct sockaddr *)&client_addr, &client_len);
        if (recv_len > 0) {
            buffer[recv_len] = '\0';
            printf("Received message from client: %s\n", buffer);

            // requeste la process
            if (strncmp(buffer, "uptime", 6) == 0) {
                char uptime_str[MAX_MSG_SIZE];
                get_uptime(uptime_str);
                sendto(sockfd, uptime_str, strlen(uptime_str), 0, (struct sockaddr *)&client_addr, client_len);
            } else if (strncmp(buffer, "stats", 5) == 0) {
                char stats_str[MAX_MSG_SIZE];
                memset(stats_str, 0, sizeof(stats_str));
                get_stats(stats_str);
                sendto(sockfd, stats_str, strlen(stats_str), 0, (struct sockaddr *)&client_addr, client_len);
            } else if (strncmp(buffer, "cmd:", 4) == 0) {
                char cmd_result[MAX_MSG_SIZE];
                memset(cmd_result, 0, sizeof(cmd_result));
                execute_command(buffer + 4, cmd_result);
                sendto(sockfd, cmd_result, strlen(cmd_result), 0, (struct sockaddr *)&client_addr, client_len);
            }
        }
    }

   // inchidere socket
    close(sockfd);

    return 0;
}

/*
Compilare:

user@DESKTOP-EPTB3M2:~$ gcc -o server1 server1.c
user@DESKTOP-EPTB3M2:~$ ./server1
Server running...
Received message from client: stats
Received message from client: uptime
Received message from client: uptime
Received message from client: cmd:ls
^C
user@DESKTOP-EPTB3M2:~$
*/
