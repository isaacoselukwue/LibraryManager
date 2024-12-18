#include "LibraryServer.hpp"
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <iostream>

LibraryServer::LibraryServer(int port) {
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket < 0) {
        throw std::runtime_error("Failed to create socket");
    }
    
    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(port);
    
    if (bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        throw std::runtime_error("Failed to bind socket");
    }
    
    running = false;
}

LibraryServer::~LibraryServer() {
    Stop();
}

void LibraryServer::Start() {
    running = true;
    listen(serverSocket, 5);
    
    while (running) {
        sockaddr_in clientAddr{};
        socklen_t clientLen = sizeof(clientAddr);
        int clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddr, &clientLen);
        
        if (clientSocket >= 0) {
            clientThreads.emplace_back(&LibraryServer::HandleClient, this, clientSocket);
        }
    }
}

void LibraryServer::Stop() {
    running = false;
    close(serverSocket);
    for (auto& thread : clientThreads) {
        if (thread.joinable()) {
            thread.join();
        }
    }
}

void LibraryServer::HandleClient(int clientSocket) {
    char buffer[1024];
    while (running) {
        ssize_t bytesRead = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
        if (bytesRead <= 0) break;
        
        buffer[bytesRead] = '\0';
        std::string request(buffer);
        ProcessRequest(clientSocket, request);
    }
    close(clientSocket);
}

void LibraryServer::ProcessRequest(int clientSocket, const std::string& request) {
    std::cout << "Received request: " << request << std::endl;
    std::string response = "Server received: " + request;
    //libraryManager.
    send(clientSocket, response.c_str(), response.length(), 0);
}