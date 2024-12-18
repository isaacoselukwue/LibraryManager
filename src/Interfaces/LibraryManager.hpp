#ifndef LIBRARY_MANAGER_HPP
#define LIBRARY_MANAGER_HPP

#include <string>
#include "../Interfaces/Books.hpp"
#include "../Interfaces/Users.hpp"
#include "../Interfaces/Transactions.hpp"

class LibraryManager {
private:
    Books books;
    Users users;
    Transactions transactions;
    UserDto currentUser;
    bool isLoggedIn = false;

public:
    LibraryManager();
    // std::string ProcessCommand(const std::string& command, const std::string& data);
    // std::string HandleLogin(const std::string& email, const std::string& password);
    // std::string HandleRegistration(const std::string& firstName, const std::string& lastName, const std::string& address, const std::string& phoneNumber, const std::string& email, const std::string& password);
};

#endif