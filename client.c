#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>  
#include <sys/socket.h>
#include <sys/time.h>   
#include <sys/types.h>  
#include <unistd.h>

#define PORT 2728
#define MAX_MSG_LEN 256

void check_and_display_server_messages();

struct Message {
        int message_number;
    char command[20];
    char sender[20];
    char receiver[20];
    char message[MAX_MSG_LEN];
    char timestamp[30];
};

void read_line(char *buffer, size_t size) {
    if (fgets(buffer, size, stdin) == NULL) {
        perror("Eroare la citirea input-ului");
        exit(EXIT_FAILURE);
    }
    buffer[strcspn(buffer, "\n")] = 0;  // Elimina caracterul newline daca exista
}

void afiseaza_meniu() {
    printf("\nMeniu:\n");
    printf("1. Trimite mesaj\n");
    printf("2. Vezi istoricul conversatiei\n");
    printf("3. Iesire\n");
    printf("4. Citeste mesaje necitite\n");
    printf("5. Reply la un mesaj\n");
    printf("6. Users online\n");

    printf("Alege o optiune: ");
        fflush(stdout); // ma asigur ca tot texul e afisat
}

int main() {
    int client_socket;
    struct sockaddr_in server_addr;
    char history_buffer[MAX_MSG_LEN];
    fd_set read_fds;    // descriptori de fisiere
    struct timeval tv;  // structura de timp
    int retval;         // aici returneaza select

    // socket-ul clientului
    client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket == -1) {
        perror("Eroare la crearea socket-ului");
        exit(EXIT_FAILURE);
    }

    // se configureaza adresa serverului
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    if (inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr) <= 0) {
        perror("Adresa invalida");
        exit(EXIT_FAILURE);
    }

    // conectare la server
    if (connect(client_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Eroare la conectare la server");
        exit(EXIT_FAILURE);
    }

    // Trimite user-ul la server
        int afiseazaMeniu = 1; // Flag pentru afișarea meniului
    char username[20];
    printf("Scrie username: ");
    read_line(username, sizeof(username));
    send(client_socket, username, sizeof(username), 0);
    check_and_display_server_messages(client_socket);
       

    while (1) {
       
        int end_of_history_received = 0;  // resetare flag pentru fiecare comanda noua
         if (afiseazaMeniu) {
            afiseaza_meniu();
            afiseazaMeniu = 0;
        }

        FD_ZERO(&read_fds);
    FD_SET(client_socket, &read_fds);
    FD_SET(STDIN_FILENO, &read_fds); // asculta si stdin

    tv.tv_sec = 0;
        tv.tv_usec = 100000; // asteapta


    retval = select(client_socket + 1, &read_fds, NULL, NULL, NULL);
    if (retval == -1) {
        perror("Eroare la select");
        exit(EXIT_FAILURE);
    }

if (FD_ISSET(client_socket, &read_fds)) {
    struct Message incoming_message;
    ssize_t bytes_received = recv(client_socket, &incoming_message, sizeof(struct Message), 0);
    if (bytes_received <= 0) {
        // Eroare la primirea mesajului sau serverul a inchis conexiunea
        printf("Eroare la primirea mesajului sau serverul a închis conexiunea.\n");
        close(client_socket);
        exit(EXIT_FAILURE);
    }
            afiseazaMeniu = 1; // flagul pentru a afisa meniul dupa procesarea mesajelor

    // Afiseaza mesajul primit
    printf("\nMesaj nou de la %s: %s\n", incoming_message.sender, incoming_message.message);
}
    if (FD_ISSET(STDIN_FILENO, &read_fds)) {
        // Aici procesezi inputul utilizatorului
    if (afiseazaMeniu) {
                afiseaza_meniu();
                afiseazaMeniu = 0; // nu se mai afiseaza meniul
            }
    int optiune = -1;
    if (scanf("%d", &optiune) <= 0) {
               getchar(); // daca nu s a introdus numar, bufferul se curata
                afiseazaMeniu = 1; // utilizatorul nu a introdus o optiune valida, afișeaza meniul din nou
                continue;
            }
    getchar(); 

    struct Message message;
    strcpy(message.sender, username); // Seteaza sender-ul cu username-ul citit

    afiseazaMeniu =1;
       
    

        switch (optiune) {
            case 1:  // Trimite mesaj
                strcpy(message.command, "send");
                printf("Scrie user-ul destinatarului: ");
                read_line(message.receiver, sizeof(message.receiver));
                printf("Scrie mesajul: ");
                read_line(message.message, sizeof(message.message));
                // Trimite mesajul la server
                ssize_t bytes_sent = send(client_socket, &message, sizeof(struct Message), 0);
                if (bytes_sent < 0 || bytes_sent != sizeof(struct Message)) {
                    perror("Eroare la trimiterea mesajului la server");
                    exit(EXIT_FAILURE);
                }
                break;

            case 2:  // Vezi istoricul conversatiei
                strcpy(message.command, "history");
                printf("Client: Solicitare istoric pentru: ");
                read_line(message.receiver, sizeof(message.receiver));
                printf("Client: Istoric solicitat cu %s\n", message.receiver);

                memset(history_buffer, 0, sizeof(history_buffer));

                send(client_socket, &message, sizeof(struct Message), 0);

                printf("Client: Istoricul conversatiei pentru %s:\n", message.receiver);
                end_of_history_received = 0;

                while (!end_of_history_received) {
                    FD_ZERO(&read_fds);
                    FD_SET(client_socket, &read_fds);

                    tv.tv_sec = 0;
                    tv.tv_usec = 500000;

                    retval = select(client_socket + 1, &read_fds, NULL, NULL, &tv);
                    if (retval == -1) {
                        perror("Client: Eroare la select");
                        break;
                    } else if (retval) {
                        char temp_buffer[MAX_MSG_LEN];
                        ssize_t bytes_received = recv(client_socket, temp_buffer, sizeof(temp_buffer) - 1, 0);
                        if (bytes_received <= 0) {
                            printf("Client: Eroare sau conexiune închisă.\n");
                            continue;  // Continua sa astepte mesaje pana la END_OF_HISTORY
                        }
                        temp_buffer[bytes_received] = '\0';

                        if (strstr(temp_buffer, "END_OF_HISTORY\n") != NULL) {
                            end_of_history_received = 1;
                            *strstr(temp_buffer, "END_OF_HISTORY\n") = '\0';
                        }

                        printf("%s", temp_buffer);  // afiseaza fiecare parte a mesajului
                    }
                }

                break;

            case 3:  // Iesire
                printf("Iesire din program.\n");
                close(client_socket);
                return 0;

                case 4: // Citeste mesaje necitite
        strcpy(message.command, "read_unseen");
        send(client_socket, &message, sizeof(struct Message), 0);
        printf("Mesajele necitite:\n");

            int mesajePrimite = 0; // Definirea variabilei înaintea buclei while

            while (1) {
                FD_ZERO(&read_fds);
                FD_SET(client_socket, &read_fds);

                tv.tv_sec = 3; // Așteaptă 5 secunde pentru a primi toate mesajele
                tv.tv_usec = 0;

                retval = select(client_socket + 1, &read_fds, NULL, NULL, &tv);
                if (retval == -1) {
                    perror("Eroare la select în client");
                    break;
                } else if (retval) {

                    char unseen_msg[MAX_MSG_LEN];
                    ssize_t bytes_received = recv(client_socket, unseen_msg, sizeof(unseen_msg) - 1, 0);
                    if (bytes_received <= 0) {
                        break; // Niciun mesaj primit sau eroare
                    }
                    unseen_msg[bytes_received] = '\0';
            printf("%s", unseen_msg);
            mesajePrimite = 1;
                } else {
                    // daca nu s a primit mesaj, se iese din bucla
            if (mesajePrimite == 0) {
                printf("Nu există mesaje necitite.\n");
            }
                    break; 
                }
            }
            afiseazaMeniu = 1;
        break;

           case 5: // Reply la un mesaj
           
    strcpy(message.command, "reply");
    printf("Scrie user-ul destinatarului pentru reply: ");
    read_line(message.receiver, sizeof(message.receiver));
    printf("Numarul mesajului la care raspunzi: ");
    scanf("%d", &message.message_number); 
    getchar();
    printf("Scrie mesajul tau: ");
    read_line(message.message, sizeof(message.message));
   
    send(client_socket, &message, sizeof(struct Message), 0);
    
    break;

    case 6:  

    strcpy(message.command, "users_online");
    send(client_socket, &message, sizeof(struct Message), 0);

        char online_users[MAX_MSG_LEN];
            memset(online_users, 0, sizeof(online_users));

    int bytes_received = recv(client_socket, online_users, sizeof(online_users), 0);
    if (bytes_received <= 0) {
        perror("Failed to receive online users list");
        exit(EXIT_FAILURE);
    }
    printf("%s", online_users);

        break;

            default:
                printf("Optiune invalida. Te rog sa alegi din nou.\n");
                continue;
        }

        
    }
    }
    // inchid socket
    close(client_socket);
    return 0;
}

void check_and_display_server_messages(int client_socket) {
    fd_set read_fds;
    struct timeval tv;
    int retval;
    char server_message[MAX_MSG_LEN];

    FD_ZERO(&read_fds);
    FD_SET(client_socket, &read_fds);

    // timp de asteptare 0.5 sec
    tv.tv_sec = 0;
    tv.tv_usec = 500000;

    retval = select(client_socket + 1, &read_fds, NULL, NULL, &tv);
    if (retval == -1) {
        perror("Client: Eroare la select");
        return;
    } else if (retval) {
        ssize_t bytes_received = recv(client_socket, server_message, sizeof(server_message) - 1, 0);
        if (bytes_received <= 0) {
            printf("Client: Eroare sau conexiune închisă.\n");
            return;
        }
        server_message[bytes_received] = '\0';
        printf("%s", server_message); // afiseaza mesajul de la server
    }
}
