#ifndef _TCP_HPP_
#define _TCP_HPP_

int setupServer(int port);
int acceptClient(int server_fd);
int connectServer(int port);

#endif