#ifndef SERVER_H
#define SERVER_H

#include <iostream>
#include <cstring>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define PORT 8080
#define BUFFER_SIZE 1024
class Server
{
public:
    Server();
    ~Server();
    void run();
private:
    void event();
    int fd;
}

#endif