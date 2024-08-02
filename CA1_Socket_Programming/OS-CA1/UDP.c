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
#include <errno.h>

int setupBroadCast(int port, struct sockaddr_in *bc_address) {
    int sock, broadcast = 1, opt = 1;

    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("setsockopt(SO_REUSEADDR) failed");
        close(sock);
        exit(EXIT_FAILURE);
    }

    if (setsockopt(sock, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt)) < 0) {
        perror("setsockopt(SO_REUSEPORT) failed");
        close(sock);
        exit(EXIT_FAILURE);
    }

    if (setsockopt(sock, SOL_SOCKET, SO_BROADCAST, &broadcast, sizeof(broadcast)) < 0) {
        perror("setsockopt(SO_BROADCAST) failed");
        close(sock);
        exit(EXIT_FAILURE);
    }

    bc_address->sin_family = AF_INET;
    bc_address->sin_port = htons(port);
    bc_address->sin_addr.s_addr = htonl(INADDR_ANY); // Listen on any address

    if (bind(sock, (struct sockaddr *)bc_address, sizeof(*bc_address)) < 0) {
        perror("bind");
        close(sock);
        exit(EXIT_FAILURE);
    }

    return sock;
}