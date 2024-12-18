#ifndef LIBRARY_CLIENT_HPP
#define LIBRARY_CLIENT_HPP

#include <string>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

class LibraryClient {
private:
    int clientSocket;
    bool connected;
    
public:
    LibraryClient(const std::string& serverIp, int port);
    ~LibraryClient();
    bool ConnectToServer();
    bool SendRequestToServer(const std::string& request);
    std::string ReceiveData();
    void CloseConnection();
};

#endif