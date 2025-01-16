#ifndef LIBRARY_MANAGER_HPP
#define LIBRARY_MANAGER_HPP

#include <string>
#include <unordered_map>
#include "Common.hpp"
#include "../Interfaces/Books.hpp"
#include "../Interfaces/Users.hpp"
#include "../Interfaces/Transactions.hpp"

struct Session {
    SessionState state{SessionState::INITIAL};
    UserCommand lastCommand{UserCommand::NONE};
    std::string email{};
    std::string password{};
    std::string firstName{};
    std::string lastName{};
    std::string address{};
    std::string phoneNumber{};
    std::string bookName{};
    bool isAuthenticated{false};
    UserDto user{};
    std::string bookIsbn{};
    std::string bookAuthor{};
    std::string bookPublisher{};
    int bookCopies{0};
    std::vector<CategoryDto> bookCategories{};
    std::string categoryName{};
    std::string categoryDescription{};
    MenuType currentMenu{MenuType::INITIAL};
};

class LibraryManager {
private:
    Books books;
    Categories categories;
    Users users;
    Transactions transactions;
    UserDto currentUser;
    bool isLoggedIn = false;
    std::unordered_map<int, Session> sessions;
    bool ValidatePassword(const std::string& password);

public:
    LibraryManager();
    std::string ProcessCommand(int clientId, const std::string& command);
    std::string HandleLogin(int clientId, const std::string& input);
    std::string HandleRegistration(int clientId, const std::string& input);
    std::string HandleBookSearch(const std::string& searchTerm);
    std::string HandleBorrowBook(int clientId, const std::string& bookId);
    std::string HandleReturnBook(int clientId, const std::string& bookId);
    std::string HandleAddBook(int clientId, const std::string& bookDetails);
    std::string HandleRemoveBook(const std::string& bookId);
    std::string HandleAddCategory(int clientId, const std::string& categoryName);
    std::string ViewBorrowedBooks(int clientId);
    std::string ViewReturnedBooks(int clientId);
    std::string HandleManageUsers();
    std::string HandleUserStatusChange(int clientId, const std::string& userId, UserStatus newStatus);
    std::string HandleUserTypeChange(int clientId, const std::string& userId, UserType newType);
    std::string HandleViewUserTransactions(int clientId, const std::string& userId);
    std::string HandleHardDeleteUser(int clientId, const std::string& userId);
    std::string HandleHardDeleteUserConfirmed(int clientId, const std::string& userId);
    std::string HandleChangePassword(int clientId, const std::string& newPassword);
    std::string HandleViewAllTransactions();
    void ClearSession(int clientId);
    std::string GetMainMenu(UserType type);
    std::string GetCurrentMenu(const Session& session);
};

#endif