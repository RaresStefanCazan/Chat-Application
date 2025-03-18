#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/time.h>
#include <errno.h>
#include <bits/pthreadtypes.h>
#include <pthread.h>


#define PORT 2728
#define MAX_CLIENTS 10
#define MAX_MSG_LEN 256
#define MAX_FILENAME_LEN 520


void *gestionare_client(void *arg);
void trimite_istoric(int client_socket, const char* filename);

struct Message {
        int message_number;
    char command[20]; 
    char sender[20];
    char receiver[20];
    char message[MAX_MSG_LEN];
    char timestamp[30];
};
struct UnseenMessage {
    char sender[20];
    char message[MAX_MSG_LEN];
    struct UnseenMessage *next;
};

struct Client {
    int socket;
    char username[20];
    struct UnseenMessage *unseen_messages;
};

struct Client clients[MAX_CLIENTS];
int client_count = 0;



void save_mesaj_fisier(struct Message *message);

void gestionare_client_nou(int server_socket);

void gestionare_mesaj_client(int client_index, struct Message *received_message) {
    //printf("Server: Mesaj primit de la %s catre %s: %s\n", 
          // received_message->sender, received_message->receiver, received_message->message);
                printf("Comanda primita: %s\n", received_message->command);
    int receiver_index = -1;
    for (int i = 0; i < client_count; i++) {
        if (strcmp(clients[i].username, received_message->receiver) == 0) {
            receiver_index = i;
            break;
        }
    }
    
    if (strcmp(received_message->command, "read_unseen") == 0) {
    char filename[50];
    snprintf(filename, sizeof(filename), "%s_unseen.txt", received_message->sender);

    FILE *file = fopen(filename, "r");
    if (!file) {
        // daca nu exista nu s mesaje necitite
        char msg[] = "Nu există mesaje necitite.\n";
        write(clients[client_index].socket, msg, strlen(msg));
        return;
    }

    char line[MAX_MSG_LEN];
    while (fgets(line, sizeof(line), file) != NULL) {
        // trimite fiecare mesaj inapoi la client
        write(clients[client_index].socket, line, strlen(line));
    }
    fclose(file);

    
    remove(filename);
}


    if (strcmp(received_message->command, "send") == 0) {
        printf("Server: Procesare comanda 'send'\n");
        if (receiver_index != -1 && clients[receiver_index].socket != -1) {
            
            // daca userul e online
            write(clients[receiver_index].socket, received_message, sizeof(struct Message));
        } else if (receiver_index != -1) {

            // daca e offline creez fisierul
        char unseen_file[50];

        snprintf(unseen_file, sizeof(unseen_file), "%s_unseen.txt", clients[receiver_index].username);
        FILE *file = fopen(unseen_file, "a");

        if (file) {
            fprintf(file, "%s: %s\n", received_message->sender, received_message->message);
            fclose(file);
        }
    }
    save_mesaj_fisier(received_message);

    } else if (strcmp(received_message->command, "history") == 0) {
        printf("Server: Procesare comanda 'history' pentru %s si %s\n", received_message->sender, received_message->receiver);

        char filename[MAX_FILENAME_LEN];
        const char *first_user, *second_user;

            // ordonez alfabetic userii
        if (strcmp(received_message->sender, received_message->receiver) < 0) {

    first_user = received_message->sender;
    second_user = received_message->receiver;
        } else {

    first_user = received_message->receiver;
    second_user = received_message->sender;
                }

snprintf(filename, sizeof(filename), "%s_%s.txt", first_user, second_user);

        trimite_istoric(clients[client_index].socket, filename);
    }
if (strcmp(received_message->command, "reply") == 0) {
    
    char filename[MAX_FILENAME_LEN];
            const char *first_user, *second_user;

    //ordonarea
    if (strcmp(received_message->sender, received_message->receiver) < 0) {
        
    first_user = received_message->sender;
    second_user = received_message->receiver;
} else {
    first_user = received_message->receiver;
    second_user = received_message->sender;
}

snprintf(filename, sizeof(filename), "%s_%s.txt", first_user, second_user);

    printf("Numarul mesajului: %d\n", received_message->message_number);
    printf("Sender: %s\n", received_message->sender);
    printf("Receiver: %s\n", received_message->receiver);

    // adaug replyul
    FILE *file = fopen(filename, "a+");

    if (!file) {
        perror("Eroare la deschiderea fișierului pentru adăugarea reply-ului");
        return;
    }

    
    fprintf(file, "[reply] %s a raspuns la mesajul numarul %d cu: \"%s\"\n",  received_message->sender, received_message->message_number, received_message->message);
    fclose(file);
}

if (strcmp(received_message->command, "users_online") == 0) {
    
    char online_users[MAX_MSG_LEN] = "Online users: \n";

    for (int i = 0; i < client_count; i++) {

        //clients[i].socket retine userii on
        if (clients[i].socket != -1) {
            strcat(online_users, clients[i].username);
            strcat(online_users, "\n");
        }
    }
    write(clients[client_index].socket, online_users, strlen(online_users));
}


}




void load_users() {

    // deschide users.txt
    FILE *file = fopen("users.txt", "r");
    if (!file) {
        perror("Eroare la deschiderea fisierului");
        return;
    }

    // citeste userii si ii adauga ca on
    while (fscanf(file, "%s", clients[client_count].username) == 1) {
        clients[client_count].socket = -1;  // daca socket=-1 e offline
        client_count++;
    }

    
    fclose(file);
}



void save_mesaj_fisier(struct Message *message) {
    char filename[MAX_FILENAME_LEN];
    char sender[MAX_MSG_LEN];
    char receiver[MAX_MSG_LEN];

    strcpy(sender, message->sender);
    strcpy(receiver, message->receiver);

    //ordonarea
    if (strcmp(sender, receiver) > 0) {
        char temp[MAX_MSG_LEN];
        strcpy(temp, sender);
        strcpy(sender, receiver);
        strcpy(receiver, temp);
    }

    snprintf(filename, sizeof(filename), "%s_%s.txt", sender, receiver);

    int message_count = 0;

    FILE *file_read = fopen(filename, "r");
    
    if (file_read) {

        char line[MAX_MSG_LEN];
        while (fgets(line, sizeof(line), file_read) != NULL) {
            message_count++;
        }
        fclose(file_read);
    }

    // se adauga mesajul cu index
    FILE *file_write = fopen(filename, "a");
    if (!file_write) {
        perror("Eroare la deschiderea fișierului pentru scriere");
        return;
    }

    fprintf(file_write, "[%d] %s: %s\n", message_count + 1, message->sender, message->message);
    fclose(file_write);
}

void trimite_istoric(int client_socket, const char* filename) {

    FILE *file = fopen(filename, "r");
    char history_buffer[MAX_MSG_LEN];

    if (file != NULL) {

        while (fgets(history_buffer, MAX_MSG_LEN, file) != NULL) {
            write(client_socket, history_buffer, strlen(history_buffer));
        }
        fclose(file);

    } else {

        char* msg = "Istoricul conversatiei nu este disponibil.\n";
        write(client_socket, msg, strlen(msg));
    }

    char* end_msg = "END_OF_HISTORY\n";
    write(client_socket, end_msg, strlen(end_msg));
}


int main() {
    int server_socket, new_socket;
    struct sockaddr_in server_addr;
    socklen_t addr_size = sizeof(struct sockaddr_in);

    // initializare socket server
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1) {
        perror("Eroare la crearea socket-ului");
        exit(EXIT_FAILURE);
    }

    // se configura adresa serverului
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    // bind-ul la o adresa specifica
    if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        perror("Eroare in binding la adresa");
        exit(EXIT_FAILURE);
    }

    // listen pentru conexiuni
    if (listen(server_socket, 5) == -1) {
        perror("Eroare in listen pentru conexiuni");
        exit(EXIT_FAILURE);
    }

    // incarca userii din fisier
    load_users();

    printf("Server-ul asculta port-ul: %d\n", PORT);

    fd_set read_fds;
    int max_sd, activity;

    while (1) {
        FD_ZERO(&read_fds);
        FD_SET(server_socket, &read_fds);
        max_sd = server_socket;

        for (int i = 0; i < client_count; i++) {
            int sd = clients[i].socket;
            FD_SET(sd, &read_fds);

            if (sd > max_sd) {
                max_sd = sd;
            }
        }

        activity = select(max_sd + 1, &read_fds, NULL, NULL, NULL);

        if ((activity < 0) && (errno != EINTR)) {
            perror("Eroare in select");
        }

        if (FD_ISSET(server_socket, &read_fds)) {
            gestionare_client_nou(server_socket);
        }

        for (int i = 0; i < client_count; i++) {
            int client_sd = clients[i].socket;

            if (FD_ISSET(client_sd, &read_fds)) {
                struct Message received_message;
                int bytes = read(client_sd, &received_message, sizeof(struct Message));

                if (bytes <= 0) {
                    // erori, deconectare
                    printf("User %s deconectat\n", clients[i].username);
                    close(client_sd);
                    clients[i].socket = -1;
                } else {
                    // primeste mesaj
                    gestionare_mesaj_client(i, &received_message);
                }
            }
        }
    }

    close(server_socket);
    return 0;
}

void gestionare_client_nou(int server_socket) {
    struct sockaddr_in client_addr;
    int addr_size = sizeof(client_addr);
    int new_socket = accept(server_socket, (struct sockaddr*)&client_addr, (socklen_t*)&addr_size);
    if (new_socket < 0) {
        perror("Eroare in acceptarea noii conexiuni");
        return;
    }

    char username[20];
    if (recv(new_socket, username, sizeof(username), 0) < 0) {
        perror("Eroare la primirea username-ului de la client");
        close(new_socket);
        return;
    }

    int valid_user = -1;
    for (int i = 0; i < client_count; i++) {
        if (strcmp(username, clients[i].username) == 0) {
            valid_user = i;
            break;
        }
    }

    if (valid_user == -1) {
        printf("Conectare cu username invalid: %s\n", username);
        close(new_socket);
        return;
    }

        if (valid_user != -1) {
    clients[valid_user].socket = new_socket;
    printf("User %s conectat\n", username);

    // verifica si trimite mesajele unseen
    char unseen_file[50];

    snprintf(unseen_file, sizeof(unseen_file), "%s_unseen.txt", username);
    
    FILE *file = fopen(unseen_file, "r");
    if (file) {
         fclose(file); 

        // Trimite notificarea "Ai mesaje noi!"
        char notification[] = "Ai mesaje noi!\n";
        write(new_socket, notification, sizeof(notification));
    }
}

        
    clients[valid_user].socket = new_socket;
    printf("User %s conectat\n", username);

    pthread_t tid;
    int *client_index = malloc(sizeof(int));
    *client_index = valid_user;
    if (pthread_create(&tid, NULL, gestionare_client, (void*)client_index) != 0) {
        perror("Eroare la crearea thread-ului pentru client");
        free(client_index);
        close(new_socket);
        return;
    }
}


void *gestionare_client(void *arg) {
    int client_index = *(int*)arg;
    free(arg); // trebuie eliberata memoria pentru argument

    struct Client *client = &clients[client_index];
    struct Message received_message;

    while (1) {
        int bytes = read(client->socket, &received_message, sizeof(struct Message));
        if (bytes <= 0) {
            printf("User %s deconectat sau eroare\n", client->username);
            close(client->socket);
            client->socket = -1;
            break;
        }

        // aici se proceseaza mesajul
        gestionare_mesaj_client(client_index, &received_message);
    }

    pthread_exit(NULL);
}
