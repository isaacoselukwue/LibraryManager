#ifndef USERS_HPP
#define USERS_HPP

#include <vector>

#include "Common.hpp"

using UserDto = struct UserDto
{
    int UserId;
    std::string FirstName;
    std::string LastName;
    std::string PasswordHash;
    std::string Email;
    UserType Type;
    UserStatus Status;
    std::string CreatedBy;
    std::time_t CreatedDate;
    std::string UpdatedBy;
    std::time_t UpdatedDate;
    std::string Address;
    std::string PhoneNumber;
    int AccessCount;
    std::vector<std::string> BorrowedBooks;
    std::vector<std::string> ReturnedBooks;
};

class Users {
    public:
        Users();
        Users(const std::string& filename);
        ~Users();

        
        bool AddUser(UserDto user);
        std::string Login(const std::string& email, const std::string& password);
        
        std::vector<UserDto> GetAllUsers();
        UserDto GetUserById(int id);
        UserDto GetUserByEmail(const std::string& email);
        std::vector<UserDto> GetUsersByStatus(UserStatus status);
        
        bool UpdateUser(const UserDto& user);
        bool UpdateUserStatus(int userId, UserStatus newStatus, const std::string& updatedBy);
        bool UpdatePassword(int userId, const std::string& newPasswordHash);
          
        bool SoftDeleteUser(int userId, const std::string& deletedBy);
        bool HardDeleteUser(int userId);
        
        bool AddBorrowedBook(int userId, const std::string& bookId);
        bool AddReturnedBook(int userId, const std::string& bookId);
        std::vector<std::string> GetBorrowedBooks(int userId);
        std::vector<std::string> GetReturnedBooks(int userId);

    private:
        std::string filename;
        void SaveToFile(const std::vector<UserDto>& users) const;
        std::vector<UserDto> LoadFromFile() const;
        bool UserExists(int userId) const;
        bool EmailExists(const std::string& email) const;
};

#endif