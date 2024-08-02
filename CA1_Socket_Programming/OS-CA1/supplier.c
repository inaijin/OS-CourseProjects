#include <unistd.h> 
#include <sys/socket.h> 
#include <netinet/in.h> 
#include <string.h> 
#include <arpa/inet.h>
#include <stdlib.h>
#include <sys/time.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include <time.h>
#include <errno.h>
#include "UDP.h"
#include "TCP.h"

int PORT;

char NAME[1024];
int serverFD;
int SENDPORT;

void logg(char logInfo[], int isItSpecial) {

    int fileFD;
    fileFD = open("log.txt", O_APPEND | O_RDWR);
    if(isItSpecial == 1)
        write(fileFD, logInfo, strlen(logInfo) - 1);
    else
        write(fileFD, logInfo, strlen(logInfo));
    close(fileFD);

}

int sockKindChecker(char buffer[1024], char type[1024]) {
    char *msgPtr = strstr(buffer, "MSG");
    if (msgPtr == NULL) {
        return 0;
    }

    msgPtr += 3;

    if (strcmp(msgPtr, type) == 0) {
        return 1;
    }

    return 0;
}

void extracter(char buffer[1024], char result[1024]) {
    char *msgPtr = strstr(buffer, "MSG");
    if (msgPtr == NULL) {
        result[0] = '\0';
        return;
    }

    size_t length = msgPtr - buffer;

    strncpy(result, buffer, length);

    result[length] = '\0';
}

void sendRequest(int choice, int port) {

    int fd = connectServer(port);
    char buff[1024] = {0};

    if(choice == 1)
        strcpy(buff, "Supplier Accepted !\n");
    else if(choice == 0)
        strcpy(buff, "Time Out !!\n");
    else
        strcpy(buff, "Supplier Denied !\n");

    strcat(buff, "MSGRS");

    send(fd, buff, strlen(buff), 0);
    memset(buff, 0, 1024);

    close(fd);

}

void sliceString(const char *input, int start, int end, char *output) {

    int length = strlen(input);
    
    if (start < 0 || start >= length || end < 0 || end >= length || start > end) {
        strcpy(output, "");
    } else {
        strncpy(output, input + start, end - start + 1);
        output[end - start + 1] = '\0';
    }

}

void alarmHandeler() {

    sendRequest(0, SENDPORT);
    SENDPORT = -1;

}

void requestHandeler(char input[1024]) {

    if(sockKindChecker(input, "NOTUNQR") == 1 || sockKindChecker(input, "NOTUNQC") == 1) {
        write(1, "Your Username Is Not Unique !!\n", 32);
        exit(0);
    }

    else if(sockKindChecker(input, "RIR") == 1) {
        char portt[1024] = {0};
        char inputt[1024];
        memset(inputt, 0, 1024);
        extracter(input, inputt);
        sliceString(inputt, 0, strlen(inputt) - 1, portt);
        SENDPORT = atoi(portt);
        signal(SIGALRM, alarmHandeler);
        siginterrupt(SIGALRM, 1);
        write(1, "Incoming Request (Y/N) : ", 26);
        char ans[1024] = {0};
        alarm(10);
        read(0, ans, 1024);
        alarm(0);
        if(SENDPORT != -1) {
            if(strcmp(ans, "y\n") == 0)
                sendRequest(1, atoi(portt));
            else if(strcmp(ans, "n\n") == 0)
                sendRequest(-1, atoi(portt));
        } else if(SENDPORT == -1) {
            write(1, "\nTime Runout !!\n", 16);
        }
    }

}

void sendInfo(int port) {

    int fd = connectServer(port);
    char buff[1024] = {0};

    strcpy(buff, NAME);
    strcat(buff, ".");
    char porttt[1024] = {0};
    int portt = PORT;
    sprintf(porttt, "%d", portt);
    strcat(buff, porttt);

    strcat(buff, "MSGINFOS");

    send(fd, buff, strlen(buff), 0);
    memset(buff, 0, 1024);

}

void sendUCRAns(int port, int bool) {

    int fd = connectServer(port);
    char buff[1024] = {0};
    strcpy(buff, "MSGNOTUNQS");

    send(fd, buff, strlen(buff), 0);
    memset(buff, 0, 1024);

    close(fd);

}

void uniqueCheckSend(int sock, struct sockaddr_in *bc_address, char name[1024]) {

    char buffer[1024];
    memset(buffer, 0, 1024);
    char portt[1024];
    memset(portt, 0, 1024);

    int port = PORT;
    strcpy(buffer, name);
    strcat(buffer, ".");
    sprintf(portt, "%d", port);
    strcat(buffer, portt);
    strcat(buffer, "MSGUNQCHKS");
    sendto(sock, buffer, strlen(buffer), 0, (struct sockaddr *)bc_address, sizeof(*bc_address));

}

void uniqueCheckRecv(char input[1024]) {
    char inputt[1024];
    memset(inputt, 0, 1024);
    extracter(input, inputt);

    char sendedName[1024] = {0};
    char sendedPort[1024] = {0};
    for (int i = 0; i < strlen(inputt); i++) {
        if (inputt[i] == '.') {
            sliceString(inputt, i + 1, strlen(inputt) - 1, sendedPort);
            sliceString(inputt, 0, i - 2, sendedName);
        }
    }
    strcat(sendedName, "\n");
    int sendeddport = atoi(sendedPort);
    if(strcmp(sendedName, NAME) == 0 && sendeddport != PORT)
        sendUCRAns(sendeddport, 1);
}

void inputHandeler(char input[1024]) {
    write(1, input, strlen(input));
}

void sockHandeler(char buffer[1024]) {

    if(sockKindChecker(buffer, "NAMEC") == 1) {
        char result[1024];
        memset(result, 0, 1024);
        extracter(buffer, result);
        write(STDOUT_FILENO, "Welcome ", 9);
        write(STDOUT_FILENO, result, strlen(result) - 1);
        write(STDOUT_FILENO, " As Customer\n", 14);
    }

    else if(sockKindChecker(buffer, "NAMER") == 1) {
        char result[1024];
        memset(result, 0, 1024);
        extracter(buffer, result);
        write(STDOUT_FILENO, "Welcome ", 9);
        write(STDOUT_FILENO, result, strlen(result) - 1);
        write(STDOUT_FILENO, " As Restaurant\n", 16);
    }

    else if(sockKindChecker(buffer, "UNQCHKR") == 1 || sockKindChecker(buffer, "UNQCHKC") == 1) {
        uniqueCheckRecv(buffer);
    }

    else if(sockKindChecker(buffer, "SHOWSR") == 1) {
        char inputt[1024];
        memset(inputt, 0, 1024);
        extracter(buffer, inputt);
        int port = atoi(inputt);
        sendInfo(port);
    }

}

int checkNameUniqueness(int tcp_server_fd, int timeout_seconds) {
    fd_set readfds;
    struct timeval timeout;
    timeout.tv_sec = timeout_seconds;
    timeout.tv_usec = 0;

    FD_ZERO(&readfds);
    FD_SET(tcp_server_fd, &readfds);
    int max_sd = tcp_server_fd;

    int activity = select(max_sd + 1, &readfds, NULL, NULL, &timeout);
    if (activity < 0 && errno != EINTR) {
        perror("select error");
        return 0;
    }

    if (activity == 0) {
        return 1;
    }

    int client_fd = acceptClient(tcp_server_fd);

    char buffer[1024] = {0};
    int bytes_received = recv(client_fd, buffer, sizeof(buffer) - 1, 0);
    if (bytes_received < 0) {
        perror("recv");
        close(client_fd);
        return 0;
    }

    buffer[bytes_received] = '\0';

    int is_unique = 1;
    if (sockKindChecker(buffer, "NOTUNQC") == 1 || sockKindChecker(buffer, "NOTUNQR") == 1) {
        write(1, "Your Username Is Not Unique !!\n", 32);
        is_unique = 0;
    }

    close(client_fd);
    return is_unique;
}

void initSupplier(int sock, struct sockaddr_in *bc_address, int tcp_server_fd) {
    char buffer[1024];
    memset(buffer, 0, sizeof(buffer));

    while (1) {
        write(1, "Enter Your Supply Shop: ", 24);
        read(0, buffer, 1024);

        uniqueCheckSend(sock, bc_address, buffer);

        if (checkNameUniqueness(tcp_server_fd, 1)) {
            strcpy(NAME, buffer);
            strcat(buffer, "MSGNAMES");
            sendto(sock, buffer, strlen(buffer), 0, (struct sockaddr *)bc_address, sizeof(*bc_address));
            logg("Supply Shop Registered: ", 0);
            logg(buffer, 1);
            break;
        } else {
            write(1, "Name already taken, please enter a different name.\n", 52);
        }

        memset(buffer, 0, sizeof(buffer));
    }
}

void Supplier(int port) {
    int udp_sock, tcp_server_fd, new_socket, client_socket[4], activity, i, valread, sd;
    int max_sd;
    struct sockaddr_in bc_address, broadcast_address;
    char buffer[1024];

    for (i = 0; i < 4; i++) {
        client_socket[i] = 0;
    }

    udp_sock = setupBroadCast(port, &bc_address);

    broadcast_address.sin_family = AF_INET;
    broadcast_address.sin_port = htons(port);
    broadcast_address.sin_addr.s_addr = inet_addr("255.255.255.255");

    srand(time(0));
    PORT = rand() % 65536;
    tcp_server_fd = setupServer(PORT);

    fd_set readfds;

    initSupplier(udp_sock, &broadcast_address, tcp_server_fd);

    while (1) {
        FD_ZERO(&readfds);

        FD_SET(udp_sock, &readfds);
        max_sd = udp_sock;

        FD_SET(tcp_server_fd, &readfds);
        if (tcp_server_fd > max_sd)
            max_sd = tcp_server_fd;

        for (i = 0; i < 4; i++) {
            sd = client_socket[i];

            if (sd > 0)
                FD_SET(sd, &readfds);

            if (sd > max_sd)
                max_sd = sd;
        }

        FD_SET(STDIN_FILENO, &readfds);
        if (STDIN_FILENO > max_sd)
            max_sd = STDIN_FILENO;

        activity = select(max_sd + 1, &readfds, NULL, NULL, NULL);

        if ((activity < 0) && (errno != EINTR)) {
            perror("select error");
        }

        if (FD_ISSET(udp_sock, &readfds)) {
            memset(buffer, 0, sizeof(buffer));
            int recv_bytes = recv(udp_sock, buffer, sizeof(buffer), 0);
            if (recv_bytes < 0) {
                perror("recv failed");
                close(udp_sock);
                exit(EXIT_FAILURE);
            }
            sockHandeler(buffer);
        }

        if (FD_ISSET(tcp_server_fd, &readfds)) {
            new_socket = accept(tcp_server_fd, NULL, NULL);
            if (new_socket < 0) {
                perror("accept");
                exit(EXIT_FAILURE);
            }

            for (i = 0; i < 4; i++) {
                if (client_socket[i] == 0) {
                    client_socket[i] = new_socket;
                    break;
                }
            }
        }

        if (FD_ISSET(STDIN_FILENO, &readfds)) {
            memset(buffer, 0, sizeof(buffer));
            int read_bytes = read(STDIN_FILENO, buffer, sizeof(buffer));
            if (read_bytes > 0) {
                if (sendto(udp_sock, buffer, read_bytes, 0, (struct sockaddr *)&broadcast_address, sizeof(broadcast_address)) < 0) {
                    perror("sendto failed");
                    close(udp_sock);
                    exit(EXIT_FAILURE);
                }
                inputHandeler(buffer);
            }
        }

        for (i = 0; i < 4; i++) {
            sd = client_socket[i];

            if (FD_ISSET(sd, &readfds)) {
                memset(buffer, 0, sizeof(buffer));
                valread = read(sd, buffer, sizeof(buffer));
                if (valread == 0) {
                    close(sd);
                    client_socket[i] = 0;
                } else {
                    buffer[valread] = '\0';
                    requestHandeler(buffer);
                    send(sd, buffer, strlen(buffer), 0);
                }
            }
        }
    }

    close(udp_sock);
    close(tcp_server_fd);
}

int main(int argc, char const *argv[]) {

    int port = atoi(argv[1]);

    Supplier(port);

    return 0;

}
