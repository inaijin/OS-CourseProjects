#include <unistd.h> 
#include <sys/socket.h> 
#include <netinet/in.h> 
#include <string.h> 
#include <arpa/inet.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/time.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <time.h>
#include <errno.h>
#include <stdbool.h>
#include "UDP.h"
#include "TCP.h"
#include "cJSON.h"

char NAME[1024];
int serverFD;
int PORT;
int SENDPORT;
char REQUEST[1024];
int AMMOUNT;

#define MAX_INGREDIENTS 100
#define MAX_DISHES 100

typedef struct {
    char name[50];
    int quantity;
} Ingredient;

typedef struct {
    char name[50];
    Ingredient ingredients[MAX_INGREDIENTS];
    int ingredient_count;
} Dish;

typedef struct {
    Dish dishes[MAX_DISHES];
    int dish_count;
} Menu;

Menu menu;

char* read_file(const char* filename) {
    FILE *file = fopen(filename, "rb");
    if (!file) {
        fprintf(stderr, "Unable to open file %s\n", filename);
        return NULL;
    }

    fseek(file, 0, SEEK_END);
    long length = ftell(file);
    fseek(file, 0, SEEK_SET);

    char *data = (char*)malloc(length + 1);
    if (!data) {
        fprintf(stderr, "Memory allocation failed\n");
        fclose(file);
        return NULL;
    }

    fread(data, 1, length, file);
    data[length] = '\0';
    fclose(file);

    return data;
}

void parse_json_dish(const char* json_data) {
    cJSON *json = cJSON_Parse(json_data);
    if (!json) {
        fprintf(stderr, "Error parsing JSON: %s\n", cJSON_GetErrorPtr());
        return;
    }

    cJSON *dish;
    cJSON_ArrayForEach(dish, json) {
        const char *dish_name = dish->string;
        printf("Dish: %s\n", dish_name);

        cJSON *ingredient;
        cJSON_ArrayForEach(ingredient, dish) {
            const char *ingredient_name = ingredient->string;
            int quantity = ingredient->valueint;
            printf("  %s: %d\n", ingredient_name, quantity);
        }
        printf("\n");
    }

    cJSON_Delete(json);
}

void parse_json_ingredient(const char* json_data, Menu* menu) {
    cJSON *json = cJSON_Parse(json_data);
    if (!json) {
        fprintf(stderr, "Error parsing JSON: %s\n", cJSON_GetErrorPtr());
        return;
    }

    menu->dish_count = 0;
    cJSON *dish;
    cJSON_ArrayForEach(dish, json) {
        if (menu->dish_count >= MAX_DISHES) break;

        const char *dish_name = dish->string;
        strncpy(menu->dishes[menu->dish_count].name, dish_name, sizeof(menu->dishes[menu->dish_count].name) - 1);
        menu->dishes[menu->dish_count].name[sizeof(menu->dishes[menu->dish_count].name) - 1] = '\0';
        menu->dishes[menu->dish_count].ingredient_count = 0;

        cJSON *ingredient;
        cJSON_ArrayForEach(ingredient, dish) {
            if (menu->dishes[menu->dish_count].ingredient_count >= MAX_INGREDIENTS) break;

            const char *ingredient_name = ingredient->string;
            int quantity = 0;

            strncpy(menu->dishes[menu->dish_count].ingredients[menu->dishes[menu->dish_count].ingredient_count].name, ingredient_name, sizeof(menu->dishes[menu->dish_count].ingredients[menu->dishes[menu->dish_count].ingredient_count].name) - 1);
            menu->dishes[menu->dish_count].ingredients[menu->dishes[menu->dish_count].ingredient_count].name[sizeof(menu->dishes[menu->dish_count].ingredients[menu->dishes[menu->dish_count].ingredient_count].name) - 1] = '\0';
            menu->dishes[menu->dish_count].ingredients[menu->dishes[menu->dish_count].ingredient_count].quantity = quantity;
            menu->dishes[menu->dish_count].ingredient_count++;
        }
        menu->dish_count++;
    }

    cJSON_Delete(json);
}

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

void openRestaurant(int sock, struct sockaddr_in *bc_address) {

    char buffer[1024];
    strcpy(buffer, "Restaurant Opened : ");
    strcat(buffer, NAME);
    logg(buffer, 0);
    strcat(buffer, "MSGOPENR");
    sendto(sock, buffer, strlen(buffer), 0, (struct sockaddr *)bc_address, sizeof(*bc_address));
    memset(buffer, 0, 1024);
    strcat(buffer, NAME);
    logg("Restaurat Registered : ", 0);
    logg(buffer, 0);
    memset(buffer, 0, 1024);

}

void closeRestaurant(int sock, struct sockaddr_in *bc_address) {

    char buffer[1024];
    strcpy(buffer, "Restaurant Closed : ");
    strcat(buffer, NAME);
    logg(buffer, 0);
    strcat(buffer, "MSGRCLOSED");
    sendto(sock, buffer, strlen(buffer), 0, (struct sockaddr *)bc_address, sizeof(*bc_address));

    while(1) {
        memset(buffer, 0, 1024);
        read(STDIN_FILENO, buffer, 1024);
        if(strcmp(buffer, "start working\n") == 0) {
            openRestaurant(sock, bc_address);
            break;
        }
    }

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
    strcat(buffer, "MSGUNQCHKR");
    sendto(sock, buffer, strlen(buffer), 0, (struct sockaddr *)bc_address, sizeof(*bc_address));

}

void trim_newline(char *str) {
    char *pos;
    if ((pos = strchr(str, '\n')) != NULL) {
        *pos = '\0';
    }
}

void update_ingredient_quantity(Menu *menu, const char *ingredient_name, int amount) {
    char trimmed_name[50];
    strncpy(trimmed_name, ingredient_name, sizeof(trimmed_name) - 1);
    trimmed_name[sizeof(trimmed_name) - 1] = '\0';
    trim_newline(trimmed_name);

    for (int i = 0; i < menu->dish_count; i++) {
        for (int j = 0; j < menu->dishes[i].ingredient_count; j++) {
            if (strcmp(menu->dishes[i].ingredients[j].name, trimmed_name) == 0) {
                menu->dishes[i].ingredients[j].quantity += amount;
            }
        }
    }
}

void sendRequestA(int choice, int port) {

    int fd = connectServer(port);
    char buff[1024] = {0};

    if(choice == 1)
        strcpy(buff, "Restaurant Accepted !\n");
    else if(choice == 0)
        strcpy(buff, "Time Out !!\n");
    else
        strcpy(buff, "Restaurant Denied !\n");

    strcat(buff, "MSGSC");

    logg(buff, 1);

    send(fd, buff, strlen(buff), 0);
    memset(buff, 0, 1024);

    close(fd);

}

void alarmHandeler() {

    sendRequestA(0, SENDPORT);
    SENDPORT = -1;

}

void requestHandeler(char input[1024]) {

    if(sockKindChecker(input, "NOTUNQS") == 1 || sockKindChecker(input, "NOTUNQC") == 1) {
        write(1, "Your Username Is Not Unique !!\n", 32);
        exit(0);
    }

    if(sockKindChecker(input, "INFOS") == 1) {
        char inputt[1024];
        memset(inputt, 0, 1024);
        extracter(input, inputt);
        char sendedName[1024];
        char sendedPort[1024];
        for (int i = 0; i < strlen(inputt); i++) {
            if (inputt[i] == '.') {
                sliceString(inputt, i + 1, strlen(inputt) - 1, sendedPort);
                sliceString(inputt, 0, i, sendedName);
            }
        }
        write(STDOUT_FILENO, sendedName, strlen(sendedName) - 2);
        write(1, " : ", 4);
        write(STDOUT_FILENO, sendedPort, strlen(sendedPort));
        write(1, "\n", 2);
    }

    else if(sockKindChecker(input, "RS") == 1) {
        char inputt[1024];
        memset(inputt, 0, 1024);
        extracter(input, inputt);
        if(strcmp(inputt, "Supplier Accepted !\n") == 0) {
            update_ingredient_quantity(&menu, REQUEST, AMMOUNT);
        }
        write(1, inputt, strlen(inputt));
    }

    else if(sockKindChecker(input, "FOODR") == 1) {
        char portt[1024] = {0};
        char inputt[1024];
        memset(inputt, 0, 1024);
        extracter(input, inputt);
        sliceString(inputt, 0, strlen(inputt) - 1, portt);
        SENDPORT = atoi(portt);
        signal(SIGALRM, alarmHandeler);
        siginterrupt(SIGALRM, 1);
        write(1, "Incoming Order (Y/N) : ", 24);
        char anssss[1024] = {0};
        alarm(10);
        read(0, anssss, 1024);
        alarm(0);
        if(SENDPORT != -1) {
            if(strcmp(anssss, "y\n") == 0)
                sendRequestA(1, atoi(portt));
            else if(strcmp(anssss, "n\n") == 0)
                sendRequestA(-1, atoi(portt));
        } else if(SENDPORT == -1) {
            write(1, "\nTime Runout !!\n", 16);
        }
    }

}

void checkSuppliersResponse(int tcp_server_fd, int timeout_seconds) {
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
        return;
    }

    if (activity == 0) {
        return;
    }

    int client_fd = acceptClient(tcp_server_fd);

    char buffer[1024] = {0};
    int bytes_received = recv(client_fd, buffer, sizeof(buffer) - 1, 0);
    if (bytes_received < 0) {
        perror("recv");
        close(client_fd);
        return;
    }

    buffer[bytes_received] = '\0';

    requestHandeler(buffer);

    close(client_fd);
}

void sendRequest(int tcp) {

    char supplierPort[1024] = {0}, ammount[1024] = {0}, ingre[1024] = {0};

    write(1, "Port Of Supplier : ", 20);
    read(0, supplierPort, 1024);
    write(1, "Ingredient : ", 14);
    read(0, ingre, 1024);
    write(1, "How Much : ", 12);
    read(0, ammount, 1024);

    strcpy(REQUEST, ingre);
    AMMOUNT = atoi(ammount);

    int fd = connectServer(atoi(supplierPort));
    char portt[1024] = {0};
    int port = PORT;
    sprintf(portt, "%d", port);
    strcpy(ingre, portt);
    logg("Restaurant Requested : ", 0);
    logg(ingre, 1);
    strcat(ingre, "MSGRIR");
    send(fd, ingre, strlen(ingre), 0);
    close(fd);
    checkSuppliersResponse(tcp, 10);

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
    if (sockKindChecker(buffer, "NOTUNQS") == 1 || sockKindChecker(buffer, "NOTUNQC") == 1) {
        write(1, "Your Username Is Not Unique !!\n", 32);
        is_unique = 0;
    }

    close(client_fd);
    return is_unique;
}

void supplierViewer(int tcp_server_fd, int timeout_seconds) {
    int client_socket[4] = {0};
    fd_set readfds;
    struct timeval timeout;
    timeout.tv_sec = timeout_seconds;
    timeout.tv_usec = 0;
    char buffer[1024];
    int max_sd, activity, new_socket, valread, sd;

    while (1) {
        FD_ZERO(&readfds);
        FD_SET(tcp_server_fd, &readfds);
        max_sd = tcp_server_fd;

        for (int i = 0; i < 4; i++) {
            sd = client_socket[i];
            if (sd > 0) {
                FD_SET(sd, &readfds);
            }
            if (sd > max_sd) {
                max_sd = sd;
            }
        }

        activity = select(max_sd + 1, &readfds, NULL, NULL, &timeout);

        if ((activity < 0) && (errno != EINTR)) {
            perror("select error");
        }

        if (FD_ISSET(tcp_server_fd, &readfds)) {
            new_socket = acceptClient(tcp_server_fd);
            if (new_socket < 0) {
                perror("accept");
                exit(EXIT_FAILURE);
            }

            for (int i = 0; i < 4; i++) {
                if (client_socket[i] == 0) {
                    client_socket[i] = new_socket;
                    break;
                }
            }
        }

        for (int i = 0; i < 4; i++) {
            sd = client_socket[i];
            if (FD_ISSET(sd, &readfds)) {
                valread = recv(sd, buffer, sizeof(buffer) - 1, 0);
                if (valread == 0) {
                    close(sd);
                    client_socket[i] = 0;
                } else {
                    buffer[valread] = '\0';
                    requestHandeler(buffer);
                }
            }
        }

        if (activity == 0) {
            return;
        }
    }
}

int empty_file(const char *filename) {
    FILE *file = fopen(filename, "w");

    if (file == NULL) {
        perror("Error opening file");
        return 1;
    }

    if (fclose(file) != 0) {
        perror("Error closing file");
        return 2;
    }

    return 0;
}

void initRestaurant(int sock, struct sockaddr_in *bc_address, int tcp_server_fd) {
    char buffer[1024];
    memset(buffer, 0, sizeof(buffer));

    const char *filename = "recipes.json";
    char *json_data = read_file(filename);

    parse_json_ingredient(json_data, &menu);
    free(json_data);

    const char *log_filename = "log.txt";

    empty_file(log_filename);

    while (1) {
        write(1, "Enter Your Restaurant Name: ", 29);
        read(0, buffer, 1024);

        uniqueCheckSend(sock, bc_address, buffer);

        if (checkNameUniqueness(tcp_server_fd, 1)) {
            strcpy(NAME, buffer);
            strcat(buffer, "MSGNAMER");
            sendto(sock, buffer, strlen(buffer), 0, (struct sockaddr *)bc_address, sizeof(*bc_address));
            logg("Restaurant Registered: ", 0);
            logg(buffer, 1);
            break;
        } else {
            write(1, "Name already taken, please enter a different name.\n", 52);
        }

        memset(buffer, 0, sizeof(buffer));
    }

    openRestaurant(sock, bc_address);
}

void sendUCRAns(int port, int boool) {

    int fd = connectServer(port);
    char buff[1024] = {0};

    strcpy(buff, "MSGNOTUNQR");

    send(fd, buff, strlen(buff), 0);
    memset(buff, 0, 1024);

    close(fd);

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

void showSupplier(int sock, struct sockaddr_in *bc_address, int tcp) {

    char buffer[1024] = {0};
    char portt[1024] = {0};
    int port = PORT;
    sprintf(portt, "%d", port);
    strcpy(buffer, portt);
    strcat(buffer, "MSGSHOWSR");
    sendto(sock, buffer, strlen(buffer), 0, (struct sockaddr *)bc_address, sizeof(*bc_address));
    logg("Restaurant Viewed Suppliers\n", 0);
    supplierViewer(tcp, 1);

}

bool is_ingredient_printed(const char* ingredient_name, char printed_ingredients[][50], int printed_count) {
    for (int i = 0; i < printed_count; i++) {
        if (strcmp(printed_ingredients[i], ingredient_name) == 0) {
            return true;
        }
    }
    return false;
}

void add_printed_ingredient(const char* ingredient_name, char printed_ingredients[][50], int* printed_count) {
    strncpy(printed_ingredients[*printed_count], ingredient_name, 50);
    (*printed_count)++;
}

void showIngredients() {
    char printed_ingredients[MAX_INGREDIENTS][50];
    int printed_count = 0;

    for (int i = 0; i < menu.dish_count; i++) {
        for (int j = 0; j < menu.dishes[i].ingredient_count; j++) {
            if (menu.dishes[i].ingredients[j].quantity == 0) {
                continue;
            }
            if (is_ingredient_printed(menu.dishes[i].ingredients[j].name, printed_ingredients, printed_count)) {
                continue;
            }
            printf("%s: %d\n", menu.dishes[i].ingredients[j].name, menu.dishes[i].ingredients[j].quantity);
            add_printed_ingredient(menu.dishes[i].ingredients[j].name, printed_ingredients, &printed_count);
        }
    }
}

void showDishes() {

    const char *filename = "recipes.json";
    char *json_data = read_file(filename);
    if (json_data) {
        parse_json_dish(json_data);
        free(json_data);
    }

    logg("Restaurant Viewed Ingredients\n", 0);

}

void inputHandeler(char input[1024], int sock, struct sockaddr_in *bc_address, int tcp) {

    if(strcmp(input, "break\n") == 0) {
        closeRestaurant(sock, bc_address);
    }

    else if(strcmp(input, "request ingredient\n") == 0) {
        sendRequest(tcp);
    }

    else if(strcmp(input, "show suppliers\n") == 0) {
        showSupplier(sock, bc_address, tcp);
    }

    else if(strcmp(input, "show ingredients\n") == 0) {
        showIngredients();
    }

    else if(strcmp(input, "show dishes\n") == 0) {
        showDishes();
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

    strcat(buff, "MSGINFOR");

    send(fd, buff, strlen(buff), 0);
    memset(buff, 0, 1024);

}

int orderFood(int tcp_server_fd, int timeout_seconds) {
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

    requestHandeler(buffer);

    close(client_fd);
    return 0;
}

void sockHandeler(char buffer[1024], int tcp) {

    if(sockKindChecker(buffer, "NAMEC") == 1) {
        char result[1024];
        memset(result, 0, 1024);
        extracter(buffer, result);
        write(STDOUT_FILENO, "Welcome ", 9);
        write(STDOUT_FILENO, result, strlen(result) - 1);
        write(STDOUT_FILENO, " As Customer\n", 14);
    }

    else if(sockKindChecker(buffer, "NAMES") == 1) {
        char result[1024];
        memset(result, 0, 1024);
        extracter(buffer, result);
        write(STDOUT_FILENO, "Welcome ", 9);
        write(STDOUT_FILENO, result, strlen(result) - 1);
        write(STDOUT_FILENO, " As Supplier\n", 14);
    }

    else if(sockKindChecker(buffer, "UNQCHKC") == 1 || sockKindChecker(buffer, "UNQCHKS") == 1) {
        uniqueCheckRecv(buffer);
    }

    else if(sockKindChecker(buffer, "SHOWRC") == 1) {
        char inputt[1024];
        memset(inputt, 0, 1024);
        extracter(buffer, inputt);
        int port = atoi(inputt);
        sendInfo(port);
    }

    else if(sockKindChecker(buffer, "ORDER") == 1) {
        orderFood(tcp, 10);
    }

}

void Restaurant(int port) {
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

    initRestaurant(udp_sock, &broadcast_address, tcp_server_fd);

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
            sockHandeler(buffer, tcp_server_fd);
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
            }
            inputHandeler(buffer, udp_sock, &broadcast_address, tcp_server_fd);
        }
    }

    close(udp_sock);
    close(tcp_server_fd);

}

int main(int argc, char const *argv[]) {

    int port = atoi(argv[1]);

    Restaurant(port);

    return 0;

}
