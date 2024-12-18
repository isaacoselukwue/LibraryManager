#include <iostream>
#include <string>

#include "../Interfaces/LibraryManager.hpp"

class LibraryManager {
    private:
        Books books;
        Users users;
        Transactions transactions;
        UserDto currentUser;
        bool isLoggedIn = false;

        void ShowMainMenu() {
            while (true) {
                std::cout << "\nLibrary Management System\n";
                std::cout << "1. Login\n";
                std::cout << "2. Register\n";
                std::cout << "3. Exit\n";
                std::cout << "Choice: ";
                
                int choice;
                std::cin >> choice;
                
                switch (choice) {
                    case 1: HandleLogin(); break;
                    case 2: HandleRegistration(); break;
                    case 3: return;
                    default: std::cout << "Invalid choice\n";
                }
            }
        }

        void HandleLogin() {
            std::string email, password;
            std::cout << "Email: ";
            std::cin >> email;
            std::cout << "Password: ";
            std::cin >> password;

            std::string result = users.Login(email, password);
            if (result == "success") {
                currentUser = users.GetUserByEmail(email);
                isLoggedIn = true;
                ShowUserMenu();
            } else {
                std::cout << result << std::endl;
            }
        }

        void HandleRegistration() {
            UserDto newUser;
            std::string password;
    
            std::cout << "First Name: ";
            std::cin.ignore();
            std::getline(std::cin, newUser.FirstName);
            
            std::cout << "Last Name: ";
            std::getline(std::cin, newUser.LastName);
            
            std::cout << "Address: ";
            std::getline(std::cin, newUser.Address);
            
            std::cout << "Phone Number: ";
            std::getline(std::cin, newUser.PhoneNumber);
            
            std::cout << "Email: ";
            std::getline(std::cin, newUser.Email);
            
            std::cout << "Password: ";
            std::getline(std::cin, password);
            
            newUser.Status = UserStatus::UserStatus_ACTIVE;
            newUser.Type = UserType::UserType_USERS;
            newUser.PasswordHash = password;
            
            if (users.AddUser(newUser)) {
                std::cout << "Registration successful!\n";
            } else {
                std::cout << "Registration failed. Email might already exist.\n";
            }
        }

        void ShowUserMenu() {
            while (isLoggedIn) {
                std::cout << "\nWelcome " << currentUser.FirstName << "!\n";
                std::cout << "1. Search Books\n";
                std::cout << "2. Borrow Book\n";
                std::cout << "3. Return Book\n";
                std::cout << "4. View My Borrowed Books\n";
                std::cout << "5. Logout\n";
                if (currentUser.Type == UserType::UserType_ADMIN) {
                    std::cout << "6. Add Book\n";
                    std::cout << "7. Remove Book\n";
                    std::cout << "8. Manage Users\n";
                }
                std::cout << "Choice: ";
                
                int choice;
                std::cin >> choice;
                
                switch (choice) {
                    case 1: HandleBookSearch(); break;
                    case 2: HandleBorrowBook(); break;
                    case 3: HandleReturnBook(); break;
                    case 4: ViewBorrowedBooks(); break;
                    case 5: Logout(); return;
                    case 6: 
                        if (currentUser.Type == UserType::UserType_ADMIN)
                            HandleAddBook();
                        break;
                    case 7:
                        if (currentUser.Type == UserType::UserType_ADMIN)
                            HandleRemoveBook();
                        break;
                    case 8:
                        if (currentUser.Type == UserType::UserType_ADMIN)
                            HandleManageUsers();
                        break;
                    default: std::cout << "Invalid choice\n";
                }
            }
        }

        void HandleBookSearch() {
            std::string query;
            std::cout << "Enter search term: ";
            std::cin.ignore();
            std::getline(std::cin, query);
            
            auto results = books.SearchBooks(query);
            if (results.empty()) {
                std::cout << "No books found.\n";
                return;
            }
            
            for (const auto& result : results) {
                std::cout << "\nID: " << result.book.BookId
                        << "\nTitle: " << result.book.Name
                        << "\nAuthor: " << result.book.Author
                        << "\nAvailable Copies: " << result.book.NoOfCopies
                        << "\n---------------\n";
            }
        }

        void HandleBorrowBook() {
            int bookId;
            std::cout << "Enter Book ID: ";
            std::cin >> bookId;
            
            auto book = books.GetBooksById(bookId);
            if (book.BookId == 0) {
                std::cout << "Book not found.\n";
                return;
            }
            
            if (book.NoOfCopies <= 0) {
                std::cout << "No copies available.\n";
                return;
            }
            
            TransactionsDto transaction;
            transaction.UserId = currentUser.UserId;
            transaction.BookId = book.BookId;
            transaction.Status = BorrowStatus::BorrowStatus_BORROWED;
            transaction.BorrowDate = std::time(nullptr);
            transaction.DueDate = transaction.BorrowDate + (14 * 24 * 60 * 60); // 14 days
            
            if (transactions.AddTransaction(transaction) == "success") {
                books.RemoveBookCopies(bookId, 1);
                std::cout << "Book borrowed successfully!\n";
            } else {
                std::cout << "Failed to borrow book.\n";
            }
        }

        void HandleReturnBook() {
            auto borrowed = transactions.GetTransactionsByUserId(currentUser.UserId);
            if (borrowed.empty()) {
                std::cout << "No books to return.\n";
                return;
            }
            
            std::cout << "Your borrowed books:\n";
            for (const auto& transaction : borrowed) {
                if (transaction.Status == BorrowStatus::BorrowStatus_BORROWED) {
                    auto book = books.GetBooksById(transaction.BookId);
                    std::cout << "ID: " << transaction.TransactionId
                            << " - " << book.Name << "\n";
                }
            }
            
            int transactionId;
            std::cout << "Enter Transaction ID to return: ";
            std::cin >> transactionId;
            
            auto transaction = transactions.GetTransactionById(transactionId);
            if (transaction.TransactionId == 0) {
                std::cout << "Invalid transaction ID.\n";
                return;
            }
            
            transaction.Status = BorrowStatus::BorrowStatus_RETURNED;
            transaction.ReturnDate = std::time(nullptr);
            transaction.ActualReturnDate = transaction.ReturnDate;
            
            if (transactions.UpdateTransaction(transaction) == "success") {
                books.AddBookCopies(transaction.BookId, 1);
                std::cout << "Book returned successfully!\n";
            } else {
                std::cout << "Failed to return book.\n";
            }
        }

        void ViewBorrowedBooks() {
            auto borrowed = transactions.GetTransactionsByUserId(currentUser.UserId);
            if (borrowed.empty()) {
                std::cout << "No borrowed books.\n";
                return;
            }
            
            for (const auto& transaction : borrowed) {
                if (transaction.Status == BorrowStatus::BorrowStatus_BORROWED) {
                    auto book = books.GetBooksById(transaction.BookId);
                    std::cout << "\nBook: " << book.Name
                            << "\nDue Date: " << std::ctime(&transaction.DueDate)
                            << "---------------\n";
                }
            }
        }


        void HandleAddBook() {
            BooksDto book{};
            std::cout << "Enter Book Title: ";
            std::cin.ignore();
            std::getline(std::cin, book.Name);
            
            std::cout << "Enter ISBN: ";
            std::getline(std::cin, book.Isbn);
            
            std::cout << "Enter Author: ";
            std::getline(std::cin, book.Author);
            
            std::cout << "Enter Publisher: ";
            std::getline(std::cin, book.Publisher);
            
            std::cout << "Enter Number of Copies: ";
            std::cin >> book.NoOfCopies;
            
            book.Status = BookStatus::BookStatus_ACTIVE;
            book.DateCreated = std::time(nullptr);
            book.DateUpdated = book.DateCreated;
            
            if (books.AddBook(book)) {
                std::cout << "Book added successfully!\n";
            } else {
                std::cout << "Failed to add book.\n";
            }
        }

        void HandleRemoveBook() {
            int bookId;
            std::cout << "Enter Book ID to remove: ";
            std::cin >> bookId;
            
            if (books.RemoveBook(bookId)) {
                std::cout << "Book removed successfully!\n";
            } else {
                std::cout << "Failed to remove book.\n";
            }
        }

        void HandleManageUsers() {
            while (true) {
                std::cout << "\nUser Management\n";
                std::cout << "1. List All Users\n";
                std::cout << "2. Activate/Deactivate User\n";
                std::cout << "3. Change User Type\n";
                std::cout << "4. Back\n";
                std::cout << "Choice: ";
                
                int choice;
                std::cin >> choice;
                
                switch (choice) {
                    case 1: {
                        auto allUsers = users.GetAllUsers();
                        for (const auto& user : allUsers) {
                            std::cout << "\nID: " << user.UserId
                                    << "\nName: " << user.FirstName << " " << user.LastName
                                    << "\nEmail: " << user.Email
                                    << "\nStatus: " << static_cast<int>(user.Status)
                                    << "\nType: " << static_cast<int>(user.Type)
                                    << "\n---------------\n";
                        }
                        break;
                    }
                    case 2: {
                        int userId;
                        std::cout << "Enter User ID: ";
                        std::cin >> userId;
                        auto user = users.GetUserById(userId);
                        if (user.UserId == 0) {
                            std::cout << "User not found.\n";
                            break;
                        }
                        user.Status = (user.Status == UserStatus::UserStatus_ACTIVE) ? 
                                    UserStatus::UserStatus_INACTIVE : UserStatus::UserStatus_ACTIVE;
                        if (users.UpdateUser(user)) {
                            std::cout << "User status updated successfully!\n";
                        }
                        break;
                    }
                    case 3: {
                        int userId;
                        std::cout << "Enter User ID: ";
                        std::cin >> userId;
                        auto user = users.GetUserById(userId);
                        if (user.UserId == 0) {
                            std::cout << "User not found.\n";
                            break;
                        }
                        std::cout << "Enter new user type (0=Admin, 1=Staff, 2=User): ";
                        int type;
                        std::cin >> type;
                        user.Type = static_cast<UserType>(type);
                        if (users.UpdateUser(user)) {
                            std::cout << "User type updated successfully!\n";
                        }
                        break;
                    }
                    case 4: return;
                    default: std::cout << "Invalid choice\n";
                }
            }
        }

        void Logout() {
            isLoggedIn = false;
            currentUser = UserDto{};
            std::cout << "Logged out successfully.\n";
        }

    public:
        LibraryManager() {}
        
        void Start() {
            ShowMainMenu();
        }
};

int main() {
    LibraryManager manager;
    manager.Start();
    return 0;
}