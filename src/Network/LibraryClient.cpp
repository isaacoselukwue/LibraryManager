#include "LibraryClient.hpp"
#include <iostream>
#include <stdexcept>
#include <unistd.h>

LibraryClient::LibraryClient(const std::string& serverIp, int port) {
    clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket < 0) {
        throw std::runtime_error("Failed to create socket");
    }
    
    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = inet_addr(serverIp.c_str());
    serverAddr.sin_port = htons(port);
    
    if (connect(clientSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        throw std::runtime_error("Failed to connect to server");
    }
    
    connected = true;
}

LibraryClient::~LibraryClient() {
    CloseConnection();
}

bool LibraryClient::ConnectToServer() {
    return connected;
}

bool LibraryClient::SendRequestToServer(const std::string& request) {
    ssize_t bytesSent = send(clientSocket, request.c_str(), request.length(), 0);
    return bytesSent == static_cast<ssize_t>(request.length());
}

std::string LibraryClient::ReceiveData() {
    char buffer[1024];
    ssize_t bytesRead = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
    if (bytesRead <= 0) return "";
    
    buffer[bytesRead] = '\0';
    return std::string(buffer);
}

void LibraryClient::CloseConnection() {
    if (connected) {
        close(clientSocket);
        connected = false;
    }
}