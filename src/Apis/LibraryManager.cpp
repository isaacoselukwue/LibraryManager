#include <iostream>
#include <string>
#include <sstream>

#include "../Interfaces/LibraryManager.hpp"

LibraryManager::LibraryManager() : books(), users(), transactions() {}

std::string LibraryManager::GetCurrentMenu(const Session& session) {
    switch (session.currentMenu) {
        case MenuType::INITIAL:
            return "Enter a command (1=Login, 2=Register, exit=Exit): ";
            
        case MenuType::MAIN:
            return GetMainMenu(session.user.Type);
            
        case MenuType::USER_MANAGEMENT:
            return "\nUser Management Commands:\n"
                   "11. Activate User\n"
                   "12. Deactivate User\n"
                   "13. Delete User\n"
                   "14. Change to Admin\n"
                   "15. Change to User\n"
                   "16. View User Transactions\n"
                   "17. View All Transactions\n"
                   "18. Hard Delete User\n"
                   "0. Back to Main Menu\n"
                   "Enter command number: ";
            
        default:
            return "";
    }
}

std::string LibraryManager::ProcessCommand(int clientId, const std::string& command) {
    if (sessions.find(clientId) == sessions.end()) {
        sessions[clientId] = Session{};
    }
    
    auto& session = sessions[clientId];
    
    if (session.state == SessionState::INITIAL) {
        if (command == "1") {
            session.state = SessionState::LOGIN_EMAIL;
            return "Enter email:";
        } else if (command == "2") {
            session.state = SessionState::REGISTER_FIRST_NAME;
            return "Enter first name:";
        } else if (command == "exit") {
            ClearSession(clientId);
            return "Goodbye!";
        }
        return "Invalid command.\n" + GetCurrentMenu(session);
    }
    
    if (!session.isAuthenticated) {
        if (session.state == SessionState::LOGIN_EMAIL || 
            session.state == SessionState::LOGIN_PASSWORD) {
            return HandleLogin(clientId, command);
        } else {
            return HandleRegistration(clientId, command);
        }
    }
    
    switch (session.state) {
        case SessionState::WAITING_SEARCH_TERM:
            session.state = SessionState::AUTHENTICATED;
            return HandleBookSearch(command);
            
        case SessionState::WAITING_BOOK_ID:
            session.state = SessionState::AUTHENTICATED;
            if (session.lastCommand == UserCommand::BORROW_BOOK) {
                return HandleBorrowBook(clientId, command);
            } else if (session.lastCommand == UserCommand::RETURN_BOOK) {
                return HandleReturnBook(clientId, command);
            }
            else if (session.lastCommand == UserCommand::REMOVE_BOOK) {
                return HandleRemoveBook(command);
            }
            return "Invalid state";
            
        case SessionState::WAITING_BOOK_NAME:
        case SessionState::WAITING_BOOK_ISBN:
        case SessionState::WAITING_BOOK_AUTHOR:
        case SessionState::WAITING_BOOK_PUBLISHER:
        case SessionState::WAITING_BOOK_COPIES:
            return HandleAddBook(clientId, command);
            
        case SessionState::WAITING_CATEGORY_NAME:
        case SessionState::WAITING_CATEGORY_DESCRIPTION:
            return HandleAddCategory(clientId, command);

        case SessionState::WAITING_USER_ID:
            //if (session.state == SessionState::WAITING_USER_ID) {
            switch (session.lastCommand) {
                case UserCommand::ACTIVATE_USER:
                    return HandleUserStatusChange(clientId, command, UserStatus::UserStatus_ACTIVE);
                case UserCommand::DEACTIVATE_USER:
                    return HandleUserStatusChange(clientId, command, UserStatus::UserStatus_INACTIVE);
                case UserCommand::DELETE_USER:
                    return HandleUserStatusChange(clientId, command, UserStatus::UserStatus_DELETED);
                case UserCommand::CHANGE_TO_ADMIN:
                    return HandleUserTypeChange(clientId, command, UserType::UserType_ADMIN);
                case UserCommand::CHANGE_TO_USER:
                    return HandleUserTypeChange(clientId, command, UserType::UserType_USERS);
                case UserCommand::VIEW_USER_TRANSACTIONS:
                    return HandleViewUserTransactions(clientId, command);
                case UserCommand::VIEW_ALL_TRANSACTIONS:
                    return HandleViewAllTransactions();
                case UserCommand::HARD_DELETE_USER:
                    return HandleHardDeleteUser(clientId, command);
                case UserCommand::HARD_DELETE_USER_CONFIRMED:
                    return HandleHardDeleteUserConfirmed(clientId, command);
                default:
                    return "Invalid command";
                    }
                //}
            
            
            case SessionState::WAITING_NEW_PASSWORD:
                return HandleChangePassword(clientId, command);

        case SessionState::AUTHENTICATED:
            try {
                int cmd = std::stoi(command);
                session.lastCommand = static_cast<UserCommand>(cmd);
                switch (session.lastCommand) {
                    case UserCommand::SEARCH_BOOKS:
                        session.state = SessionState::WAITING_SEARCH_TERM;
                        return "Enter search term:";
                        
                    case UserCommand::BORROW_BOOK:
                        session.state = SessionState::WAITING_BOOK_ID;
                        return "Enter book ID to borrow:";
                        
                    case UserCommand::RETURN_BOOK:
                        session.state = SessionState::WAITING_BOOK_ID;
                        return "Enter book ID to return:";
                        
                    case UserCommand::VIEW_BORROWED:
                        return ViewBorrowedBooks(clientId);
                        
                    case UserCommand::VIEW_RETURNED:
                        return ViewReturnedBooks(clientId);
                        
                    case UserCommand::ADD_BOOK:
                        if (session.user.Type != UserType::UserType_ADMIN) {
                            return "Access denied. Admin privileges required.";
                        }
                        session.state = SessionState::WAITING_BOOK_NAME;
                        return "Enter book name:";
                        
                    case UserCommand::REMOVE_BOOK:
                        if (session.user.Type != UserType::UserType_ADMIN) {
                            return "Access denied. Admin privileges required.";
                        }
                        session.state = SessionState::WAITING_BOOK_ID;
                        return "Enter book ID to remove:";
                        
                    case UserCommand::ADD_CATEGORY:
                        if (session.user.Type != UserType::UserType_ADMIN) {
                            return "Access denied. Admin privileges required.";
                        }
                        session.state = SessionState::WAITING_CATEGORY_NAME;
                        return "Enter category name:";
                        
                    case UserCommand::MANAGE_USERS:
                        if (session.user.Type != UserType::UserType_ADMIN) {
                            return "Access denied. Admin privileges required.";
                        }
                        return HandleManageUsers(clientId);

                    case UserCommand::ACTIVATE_USER:
                        if (session.user.Type != UserType::UserType_ADMIN) {
                            return "Access denied. Admin privileges required.";
                        }
                        session.state = SessionState::WAITING_USER_ID;
                        return "Enter user ID to activate:";
                        
                    case UserCommand::DEACTIVATE_USER:
                        if (session.user.Type != UserType::UserType_ADMIN) {
                            return "Access denied. Admin privileges required.";
                        }
                        session.state = SessionState::WAITING_USER_ID;
                        return "Enter user ID to deactivate:";
                        
                    case UserCommand::DELETE_USER:
                        if (session.user.Type != UserType::UserType_ADMIN) {
                            return "Access denied. Admin privileges required.";
                        }
                        session.state = SessionState::WAITING_USER_ID;
                        return "Enter user ID to delete:";
                    
                    case UserCommand::CHANGE_TO_ADMIN:
                        if (session.user.Type != UserType::UserType_ADMIN) {
                            return "Access denied. Admin privileges required.";
                        }
                        session.state = SessionState::WAITING_USER_ID;
                        return "Enter user ID to change to Admin:";

                    case UserCommand::CHANGE_TO_USER:
                        if (session.user.Type != UserType::UserType_ADMIN) {
                            return "Access denied. Admin privileges required.";
                        }
                        session.state = SessionState::WAITING_USER_ID;
                        return "Enter user ID to change to User:";

                    case UserCommand::VIEW_USER_TRANSACTIONS:
                        if (session.user.Type != UserType::UserType_ADMIN) {
                            return "Access denied. Admin privileges required.";
                        }
                        session.state = SessionState::WAITING_USER_ID;
                        return "Enter user ID to view transactions:";

                    case UserCommand::VIEW_ALL_TRANSACTIONS:
                        if (session.user.Type != UserType::UserType_ADMIN) {
                            return "Access denied. Admin privileges required.";
                        }
                        return HandleViewAllTransactions();

                    case UserCommand::HARD_DELETE_USER:
                        if (session.user.Type != UserType::UserType_ADMIN) {
                            return "Access denied. Admin privileges required.";
                        }
                        session.state = SessionState::WAITING_USER_ID;
                        return "Enter user ID to hard delete (permanent):";

                    case UserCommand::CHANGE_PASSWORD:
                        session.state = SessionState::WAITING_NEW_PASSWORD;
                        return "Enter new password:";

                    case UserCommand::LOGOUT:
                        ClearSession(clientId);
                        return "Logged out successfully.";
                        
                    default:
                        return GetMainMenu(session.user.Type);
                }
            } catch (...) {
                return GetMainMenu(session.user.Type);
            }
            
            
        default:
            return "Invalid state";
    }
}

std::string LibraryManager::GetMainMenu(UserType userType) {
    std::stringstream ss;
    ss << "\nAvailable commands:\n"
       << "1. Search Books\n"
       << "2. Borrow Book\n"
       << "3. Return Book\n"
       << "4. View Borrowed Books\n"
       << "5. View Returned Books\n";
    
    if (userType == UserType::UserType_ADMIN) {
        ss << "6. Add Book\n"
           << "7. Remove Book\n"
           << "8. Add Category\n"
           << "9. Manage Users\n";
    }
    
    ss << "10. Logout\n"
        << "20. Change Password\n"
        << "Enter command number: ";
    
    return ss.str();
}

std::string LibraryManager::HandleLogin(int clientId, const std::string& input) {
    auto& session = sessions[clientId];
    
    if (session.state == SessionState::LOGIN_EMAIL) {
        session.email = input;
        session.state = SessionState::LOGIN_PASSWORD;
        return "Enter password:";
    }
    
    if (session.state == SessionState::LOGIN_PASSWORD) {
        session.password = input;
        std::string result = users.Login(session.email, session.password);
        if (result == "success") {
            session.isAuthenticated = true;
            session.user = users.GetUserByEmail(session.email);
            session.state = SessionState::AUTHENTICATED;
            
            auto borrowedBooks = users.GetBorrowedBooks(session.user.UserId);
            std::stringstream ss;
            ss << "Welcome " << session.user.FirstName << " " << session.user.LastName << "!\n";
            ss << "You currently have " << borrowedBooks.size() << " book(s) borrowed.\n\n";
            ss << GetMainMenu(session.user.Type);
            return ss.str();
        }
        session.state = SessionState::INITIAL;
        return "Login failed. " + result;
    }
    
    return "Invalid login state";
}

std::string LibraryManager::HandleRegistration(int clientId, const std::string& input) {
    auto& session = sessions[clientId];
    
    switch (session.state) {
        case SessionState::REGISTER_FIRST_NAME:
            session.firstName = input;
            session.state = SessionState::REGISTER_LAST_NAME;
            return "Enter last name:";
            
        case SessionState::REGISTER_LAST_NAME:
            session.lastName = input;
            session.state = SessionState::REGISTER_ADDRESS;
            return "Enter address:";
            
        case SessionState::REGISTER_ADDRESS:
            session.address = input;
            session.state = SessionState::REGISTER_PHONE;
            return "Enter phone number:";
            
        case SessionState::REGISTER_PHONE:
            session.phoneNumber = input;
            session.state = SessionState::REGISTER_EMAIL;
            return "Enter email:";
            
        case SessionState::REGISTER_EMAIL:
            session.email = input;
            session.state = SessionState::REGISTER_PASSWORD;
            return "Enter password:";
            
        case SessionState::REGISTER_PASSWORD: {
            session.password = input;

            if (!ValidatePassword(session.password)) {
                session.state = SessionState::REGISTER_PASSWORD;
                return "Password must be at least 8 characters long.";
            }

            UserDto newUser;
            newUser.FirstName = session.firstName;
            newUser.LastName = session.lastName;
            newUser.Address = session.address;
            newUser.PhoneNumber = session.phoneNumber;
            newUser.Email = session.email;
            newUser.PasswordHash = session.password;
            newUser.Status = UserStatus::UserStatus_ACTIVE;
            newUser.Type = UserType::UserType_USERS;
            
            if (users.AddUser(newUser)) {
                session.isAuthenticated = true;
                session.state = SessionState::AUTHENTICATED;
                return "Registration successful! Welcome " + session.firstName;
            }
            
            session.state = SessionState::INITIAL;
            return "Registration failed. Email might already exist.";
        }
        
        default:
            session.state = SessionState::INITIAL;
            return "Invalid registration state";
    }
}

void LibraryManager::ClearSession(int clientId) {
    sessions.erase(clientId);
}

std::string LibraryManager::HandleBookSearch(const std::string& searchTerm) {
    auto results = books.SearchBooks(searchTerm);
    if (results.empty()) {
        return "No books found.";
    }

    std::stringstream ss;
    ss << "\nSearch Results:\n";
    for (const auto& result : results) {
        ss << "ID: " << result.book.BookId
           << ", Title: " << result.book.Name
           << ", Author: " << result.book.Author
           << ", Copies: " << result.book.NoOfCopies << "\n";
    }
    return ss.str();
}

std::string LibraryManager::HandleBorrowBook(int clientId, const std::string& bookId) {
    auto& session = sessions[clientId];
    try {
        auto book = books.GetBooksById(std::stoi(bookId));
        if (book.BookId == 0) {
            return "Book not found.";
        }
        if (book.NoOfCopies <= 0) {
            return "No copies available.";
        }

        TransactionsDto transaction;
        transaction.UserId = session.user.UserId;
        transaction.BookId = book.BookId;
        transaction.CreatedDate = std::time(nullptr);
        transaction.BorrowDate = std::time(nullptr);
        transaction.Status = BorrowStatus::BorrowStatus_BORROWED;

        if (transactions.AddTransaction(transaction) == "success") {
            books.RemoveBookCopies(book.BookId, 1);
            users.AddBorrowedBook(session.user.UserId, bookId);
            return "Book borrowed successfully.";
        }
    } catch (const std::exception& e) {
        return "Failed to borrow book: " + std::string(e.what());
    }
    return "Failed to borrow book.";
}

std::string LibraryManager::HandleReturnBook(int clientId, const std::string& bookId) {
    auto& session = sessions[clientId];
    try
    {
        auto book = books.GetBooksById(std::stoi(bookId));
        if (book.BookId == 0) {
            return "Book not found.";
        }

        auto userTransactions = transactions.GetTransactionsByUserId(session.user.UserId);
        auto it = std::find_if(userTransactions.begin(), userTransactions.end(),
            [&](const TransactionsDto& t) {
                return t.BookId == book.BookId && 
                    t.Status == BorrowStatus::BorrowStatus_BORROWED;
            });

        if (it == userTransactions.end()) {
            return "You haven't borrowed this book.";
        }

        it->Status = BorrowStatus::BorrowStatus_RETURNED;
        it->ReturnDate = std::time(nullptr);
        it->ActualReturnDate = std::time(nullptr);

        if (transactions.UpdateTransaction(*it) == "success") {
            books.AddBookCopies(book.BookId, 1);
            users.AddReturnedBook(session.user.UserId, std::to_string(book.BookId));
            return "Book returned successfully.";
        }
    }
    catch(const std::exception& e)
    {
        return "Failed to return book: " + std::string(e.what());
    }
    
    return "Failed to return book.";
}

std::string LibraryManager::ViewBorrowedBooks(int clientId) {
    auto& session = sessions[clientId];
    auto borrowedBooks = users.GetBorrowedBooks(session.user.UserId);
    
    if (borrowedBooks.empty()) {
        return "No books currently borrowed.";
    }

    std::stringstream ss;
    ss << "\nCurrently Borrowed Books:\n";
    for (const auto& bookId : borrowedBooks) {
        auto book = books.GetBooksById(std::stoi(bookId));
        auto transaction = transactions.GetBorrowedTransactionsByUserAndBookId(session.user.UserId, book.BookId);

        std::time_t dueDate = transaction.DueDate;
        std::tm* tm = std::gmtime(&dueDate);

        ss << "Book ID: " << book.BookId 
           << ", Title: " << book.Name
           << ", Author: " << book.Author
           << ", Due: " << std::put_time(tm, "%Y-%m-%d %H:%M:%S UTC") << "\n";
    }
    return ss.str();
}

std::string LibraryManager::ViewReturnedBooks(int clientId) {
    auto& session = sessions[clientId];
    auto returnedBooks = users.GetReturnedBooks(session.user.UserId);
    
    if (returnedBooks.empty()) {
        return "No books in return history.";
    }

    std::stringstream ss;
    ss << "\nReturn History:\n";
    for (const auto& bookId : returnedBooks) {
        auto book = books.GetBooksById(std::stoi(bookId));
        auto transaction = transactions.GetReturnedTransactionsByUserAndBookId(session.user.UserId, book.BookId);

        std::time_t returnDate = transaction.ReturnDate;
        std::tm* tm = std::gmtime(&returnDate);

        ss << "Book ID: " << book.BookId 
           << ", Title: " << book.Name
           << ", Author: " << book.Author
           << ", Returned: " << std::put_time(tm, "%Y-%m-%d %H:%M:%S UTC") << "\n";
    }
    return ss.str();
}

// Admin Methods
std::string LibraryManager::HandleAddBook(int clientId, const std::string& input) {
    auto& session = sessions[clientId];

    switch (session.state) {
        case SessionState::WAITING_BOOK_NAME:
            session.bookName = input;
            session.state = SessionState::WAITING_BOOK_ISBN;
            return "Enter ISBN:";

        case SessionState::WAITING_BOOK_ISBN:
            session.bookIsbn = input;
            session.state = SessionState::WAITING_BOOK_AUTHOR;
            return "Enter author:";

        case SessionState::WAITING_BOOK_AUTHOR:
            session.bookAuthor = input;
            session.state = SessionState::WAITING_BOOK_PUBLISHER;
            return "Enter publisher:";

        case SessionState::WAITING_BOOK_PUBLISHER:
            session.bookPublisher = input;
            session.state = SessionState::WAITING_BOOK_COPIES;
            return "Enter number of copies:";

        case SessionState::WAITING_BOOK_COPIES:
            try {
                session.bookCopies = std::stoi(input);
                BooksDto newBook;
                newBook.Name = session.bookName;
                newBook.Isbn = session.bookIsbn;
                newBook.Author = session.bookAuthor;
                newBook.Publisher = session.bookPublisher;
                newBook.NoOfCopies = session.bookCopies;
                newBook.Status = BookStatus::BookStatus_ACTIVE;
                
                if (books.AddBook(newBook)) {
                    session.state = SessionState::AUTHENTICATED;
                    return "Book added successfully!";
                }
                
                session.state = SessionState::AUTHENTICATED;
                return "Failed to add book.";
            } catch (const std::exception& e) {
                return "Invalid number of copies. Please enter a number.";
            }

        default:
            session.state = SessionState::AUTHENTICATED;
            return "Invalid state for adding book";
    }
}

std::string LibraryManager::HandleRemoveBook(const std::string& bookId) {
    if (books.RemoveBook(std::stoi(bookId))) {
        return "Book removed successfully.";
    }
    return "Failed to remove book.";
}

std::string LibraryManager::HandleAddCategory(int clientId, const std::string& input) {
    auto& session = sessions[clientId];
    CategoryDto newCategory;

    switch (session.state) {
        case SessionState::WAITING_CATEGORY_NAME:
            session.categoryName = input;
            session.state = SessionState::WAITING_CATEGORY_DESCRIPTION;
            return "Enter category description:";

        case SessionState::WAITING_CATEGORY_DESCRIPTION:
            session.categoryDescription = input;
            
            newCategory.Name = session.categoryName;
            newCategory.Description = session.categoryDescription;
            
            if (categories.AddCategory(newCategory)) {
                session.state = SessionState::AUTHENTICATED;
                return "Category added successfully!";
            }
            
            session.state = SessionState::AUTHENTICATED;
            return "Failed to add category.";

        default:
            session.state = SessionState::AUTHENTICATED;
            return "Invalid state for adding category";
    }
}

//add methods to manage users details disable account 
std::string LibraryManager::HandleManageUsers(int clientId) {
    auto& session = sessions[clientId];
    auto allUsers = users.GetAllUsers();
    
    std::stringstream ss;
    ss << "\nUser Management:\n";
    ss << "----------------------------------------\n";
    ss << "Status: 1=Active, 2=Inactive, 3=Deleted\n";
    ss << "Type: 0=Admin, 2=User\n\n";
    
    for (const auto& user : allUsers) {
        ss << "ID: " << user.UserId 
           << ", Name: " << user.FirstName << " " << user.LastName
           << ", Email: " << user.Email
           << ", Type: " << static_cast<int>(user.Type)
           << ", Status: " << static_cast<int>(user.Status) << "\n";
    }
    
    ss << "\nCommands:\n"
       << "11. Activate User\n"
       << "12. Deactivate User\n"
       << "13. Delete User\n"
       << "14. Change to Admin\n"
       << "15. Change to User\n"
       << "16. View User Transactions\n"
       << "17. View All Transactions\n"
       << "18. Hard Delete User (Permanent)\n"
       << "0. Back to Main Menu\n";
    
    return ss.str();
}

std::string LibraryManager::HandleUserStatusChange(int clientId, const std::string& userId, UserStatus newStatus) {
    auto& session = sessions[clientId];
    try {
        int id = std::stoi(userId);
        auto user = users.GetUserById(id);
        
        if (user.UserId == 0) {
            session.state = SessionState::AUTHENTICATED;
            return "User not found.";
        }
        
        switch (user.Status) {
            case UserStatus::UserStatus_DELETED:
            session.state = SessionState::AUTHENTICATED;
                return "Cannot modify deleted user account.";
                
            case UserStatus::UserStatus_INACTIVE:
                if (newStatus == UserStatus::UserStatus_DELETED) {
                    break; 
                }
                if (newStatus == UserStatus::UserStatus_INACTIVE) {
                    session.state = SessionState::AUTHENTICATED;
                    return "User account is already inactive.";
                }
                break;
                
            case UserStatus::UserStatus_ACTIVE:
                if (newStatus == UserStatus::UserStatus_ACTIVE) {
                    session.state = SessionState::AUTHENTICATED;
                    return "User account is already active.";
                }
                break;

            default:
                break;
        }
        
        user.Status = newStatus;
        user.UpdatedDate = std::time(nullptr);
        
        if (users.UpdateUser(user)) {
            session.state = SessionState::AUTHENTICATED;
            std::string status;
            switch (newStatus) {
                case UserStatus::UserStatus_ACTIVE:
                    status = "activated";
                    break;
                case UserStatus::UserStatus_INACTIVE:
                    status = "deactivated";
                    break;
                case UserStatus::UserStatus_DELETED:
                    status = "deleted";
                    break;

                default:
                    status = "unknown";
                    break;
            }
            return "User account " + status + " successfully.";
        }
        session.state = SessionState::AUTHENTICATED;
        return "Failed to update user status.";
    } catch (...) {
        session.state = SessionState::AUTHENTICATED;
        return "Invalid user ID.";
    }
}

//This handles the change of user type
std::string LibraryManager::HandleUserTypeChange(int clientId, const std::string& userId, UserType newType) {
    auto& session = sessions[clientId];
    
    try {
        int id = std::stoi(userId);
        
        if (id == session.user.UserId) {
            session.state = SessionState::AUTHENTICATED;
            return "Cannot modify your own user type.";
        }
        
        auto user = users.GetUserById(id);
        if (user.UserId == 0) {
            session.state = SessionState::AUTHENTICATED;
            return "User not found.";
        }
        
        if (newType != UserType::UserType_ADMIN && newType != UserType::UserType_USERS) {
            session.state = SessionState::AUTHENTICATED;
            return "Can only change between Admin and User types.";
        }
        
        if (user.Type == newType) {
            session.state = SessionState::AUTHENTICATED;
            return "User is already of this type.";
        }
        
        if (user.Status == UserStatus::UserStatus_DELETED) {
            session.state = SessionState::AUTHENTICATED;
            return "Cannot modify deleted user account.";
        }
        
        if (user.Status == UserStatus::UserStatus_INACTIVE) {
            session.state = SessionState::AUTHENTICATED;
            return "Cannot modify inactive user account.";
        }
        
        user.Type = newType;
        user.UpdatedDate = std::time(nullptr);
        
        if (users.UpdateUser(user)) {
            session.state = SessionState::AUTHENTICATED;
            std::string type = (newType == UserType::UserType_ADMIN) ? "Administrator" : "User";
            return "User type changed to " + type + " successfully.";
        }
        
        session.state = SessionState::AUTHENTICATED;
        return "Failed to update user type.";
    } catch (...) {
        session.state = SessionState::AUTHENTICATED;
        return "Invalid user ID.";
    }
}

std::string LibraryManager::HandleViewUserTransactions(int clientId, const std::string& userId) {
    auto& session = sessions[clientId];
    try {
        int id = std::stoi(userId);
        auto user = users.GetUserById(id);
        if (user.UserId == 0) {
            session.state = SessionState::AUTHENTICATED;
            return "User not found.";
        }

        auto userTransactions = transactions.GetTransactionsByUserId(id);
        if (userTransactions.empty()) {
            return "No transactions found for user.";
        }

        std::stringstream ss;
        ss << "\nTransactions for " << user.FirstName << " " << user.LastName << ":\n";
        ss << "----------------------------------------\n";
        
        for (const auto& trans : userTransactions) {
            auto book = books.GetBooksById(trans.BookId);
            std::tm* borrowTm = std::gmtime(&trans.BorrowDate);
            std::string status = (trans.Status == BorrowStatus::BorrowStatus_BORROWED) ? "Borrowed" : "Returned";
            
            ss << "Book: " << book.Name 
               << "\nStatus: " << status
               << "\nBorrowed: " << std::put_time(borrowTm, "%Y-%m-%d %H:%M:%S UTC");
            
            if (trans.Status == BorrowStatus::BorrowStatus_RETURNED) {
                std::tm* returnTm = std::gmtime(&trans.ReturnDate);
                ss << "\nReturned: " << std::put_time(returnTm, "%Y-%m-%d %H:%M:%S UTC");
            }
            ss << "\n----------------------------------------\n";
        }
        session.state = SessionState::AUTHENTICATED;
        return ss.str();
    } catch (...) {
        session.state = SessionState::AUTHENTICATED;
        return "Invalid user ID.";
    }
}

std::string LibraryManager::HandleViewAllTransactions() {
    auto allTransactions = transactions.GetAllTransactions();
    if (allTransactions.empty()) {
        return "No transactions found.";
    }

    std::stringstream ss;
    ss << "\nAll Library Transactions:\n";
    ss << "----------------------------------------\n";
    
    for (const auto& trans : allTransactions) {
        auto user = users.GetUserById(trans.UserId);
        auto book = books.GetBooksById(trans.BookId);
        std::tm* borrowTm = std::gmtime(&trans.BorrowDate);
        std::string status = (trans.Status == BorrowStatus::BorrowStatus_BORROWED) ? "Borrowed" : "Returned";
        
        ss << "User: " << user.FirstName << " " << user.LastName
           << "\nBook: " << book.Name 
           << "\nStatus: " << status
           << "\nBorrowed: " << std::put_time(borrowTm, "%Y-%m-%d %H:%M:%S UTC");
        
        if (trans.Status == BorrowStatus::BorrowStatus_RETURNED) {
            std::tm* returnTm = std::gmtime(&trans.ReturnDate);
            ss << "\nReturned: " << std::put_time(returnTm, "%Y-%m-%d %H:%M:%S UTC");
        }
        ss << "\n----------------------------------------\n";
    }
    return ss.str();
}

std::string LibraryManager::HandleHardDeleteUser(int clientId, const std::string& userId) {
    auto& session = sessions[clientId];
    try {
        int id = std::stoi(userId);
        auto user = users.GetUserById(id);
        
        if (user.UserId == 0) {
            session.state = SessionState::AUTHENTICATED;
            return "User not found.";
        }
        
        if (id == session.user.UserId) {
            session.state = SessionState::AUTHENTICATED;
            return "Cannot delete your own account.";
        }
        
        session.lastCommand = UserCommand::HARD_DELETE_USER_CONFIRMED;
        session.state = SessionState::WAITING_USER_ID;
        return "WARNING: This will permanently delete the user account and all history.\n"
               "This action cannot be undone!\n"
               "Enter user ID again to confirm deletion:";
    } catch (...) {
        session.state = SessionState::AUTHENTICATED;
        return "Invalid user ID.";
    }
}

std::string LibraryManager::HandleHardDeleteUserConfirmed(int clientId, const std::string& userId) {
    auto& session = sessions[clientId];
    try {
        int id = std::stoi(userId);
        if (users.HardDeleteUser(id)) {
            session.state = SessionState::AUTHENTICATED;
            return "User has been permanently deleted.";
        }
        session.state = SessionState::AUTHENTICATED;
        return "Failed to delete user.";
    } catch (...) {
        session.state = SessionState::AUTHENTICATED;
        return "Invalid user ID.";
    }
}
//add a readme file

std::string LibraryManager::HandleChangePassword(int clientId, const std::string& newPassword) {
    auto& session = sessions[clientId];
    
    if (!ValidatePassword(newPassword)) {
        session.state = SessionState::AUTHENTICATED;
        return "Password must be at least 8 characters long, contain at least one uppercase letter, "
               "one lowercase letter, one number, and no spaces.";
    }
    
    if (users.UpdatePassword(session.user.UserId, newPassword)) {
        session.state = SessionState::AUTHENTICATED;
        return "Password changed successfully.";
    }
    
    session.state = SessionState::AUTHENTICATED;
    return "Failed to change password.";
}

bool LibraryManager::ValidatePassword(const std::string& password) {
    if (password.length() < 8) {
        return false;
    }
    
    bool hasUpper = false;
    bool hasLower = false;
    bool hasNumber = false;
    
    for (char c : password) {
        if (std::isupper(c)) hasUpper = true;
        if (std::islower(c)) hasLower = true;
        if (std::isdigit(c)) hasNumber = true;
        if (std::isspace(c)) return false;
    }
    
    return hasUpper && hasLower && hasNumber;
}