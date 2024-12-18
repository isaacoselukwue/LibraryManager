#include <iostream>

#include "../Interfaces/Books.hpp"
#include "../Interfaces/Categories.hpp"
#include "../Interfaces/Users.hpp"
#include "../Network/LibraryClient.hpp"
#include "../Network/LibraryServer.hpp"
#include "../Tests/UnitTests/BookTests.hpp"
#include "../Tests/UnitTests/CategoryTests.hpp"
#include "../Tests/UnitTests/UserTests.hpp"
#include "../Tests/UnitTests/TransactionTests.hpp"

void RunUnitTests() {
    BookTests bookTests;
    CategoryTests categoryTests;
    UserTests userTests;
    TransactionTests transactionTests;
    
    std::cout << "Running Unit Tests...\n\n";

    std::cout << "\nCategory Tests:\n";
    categoryTests.RunAllTests();
    
    std::cout << "\nBook Tests:\n";
    bookTests.RunAllTests();

    std::cout << "\nUser Tests:\n";
    userTests.RunAllTests();

    std::cout << "\nTransaction Tests:\n";
    transactionTests.RunAllTests();
}

int main(int argc, char* argv[])
{
    if (argc > 1 && std::string(argv[1]) == "1") {
        RunUnitTests();
        return 0;
    }

    std::cout << "Library Management System\n";

    if (argc > 1 && std::string(argv[1]) == "server") {
        try {
            LibraryServer server(8080);
            server.Start();
        } catch (const std::exception& e) {
            std::cerr << "Server error: " << e.what() << std::endl;
            return 1;
        }
    }
    else if (argc > 1 && std::string(argv[1]) == "client") {
        try {
            LibraryClient client("127.0.0.1", 8080);
            if (client.ConnectToServer()) {
                std::string request;
                while (true) {
                    std::cout << "Enter a message to send to the server (or 'exit' to disconnect): ";
                    std::getline(std::cin, request);
                    if (request == "exit") {
                        client.CloseConnection();
                        break;
                    }
                    client.SendRequestToServer(request);
                    std::string response = client.ReceiveData();
                    std::cout << "Response from server: " << response << std::endl;
                }
            }
        } catch (const std::exception& e) {
            std::cerr << "Client error: " << e.what() << std::endl;
            return 1;
        }
    }
    return 0;
}