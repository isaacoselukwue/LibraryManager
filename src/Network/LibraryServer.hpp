#ifndef LIBRARY_SERVER_HPP
#define LIBRARY_SERVER_HPP

#include <thread>
#include <vector>
#include <mutex>
#include <unordered_map>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "../Interfaces/LibraryManager.hpp"
#include "../Interfaces/Audits.hpp"
#include "../Utils/AuditLogger.hpp"

class LibraryServer {
private:
    int serverSocket;
    std::vector<std::thread> clientThreads;
    bool running;
    LibraryManager libraryManager;

    Audits audit;
    AuditLogger auditLogger;
    
    void HandleClient(int clientSocket);
    void ProcessRequest(int clientSocket, const std::string& request);
    
public:
    LibraryServer(int port);
    ~LibraryServer();
    void Start();
    void Stop();
};

#endif