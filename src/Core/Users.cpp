#include <nlohmann/json.hpp>
#include <filesystem>
#include <fstream>
#include <sys/file.h>
#include <fcntl.h>
#include <unistd.h>
#include <functional>
#include <iomanip>
#include <sstream>

#include "../Interfaces/Users.hpp"
#include "../Utils/HashUtils.hpp"

using json = nlohmann::json;
namespace fs = std::filesystem;

Users::Users() {
    filename = "./resources/database/users.json";
    fs::create_directories("./resources/database");
    if (!fs::exists(filename)) {
        std::ofstream file(filename);
        file << "[]";
        file.close();
    }
}

Users::Users(const std::string& fname) : filename(fname) {}

Users::~Users() {}

std::string Users::Login(const std::string& email, const std::string& password) {
    try {
        int fd = open(filename.c_str(), O_RDWR);
        if (fd == -1) return "System error: Unable to access database";
        
        flock(fd, LOCK_EX);
        auto users = LoadFromFile();
        
        auto it = std::find_if(users.begin(), users.end(),
            [&email](const UserDto& u) { return u.Email == email; });
            
        if (it == users.end()) {
            flock(fd, LOCK_UN);
            close(fd);
            return "Invalid email or password";
        }
        
        if (it->Status == UserStatus::UserStatus_DELETED) {
            flock(fd, LOCK_UN);
            close(fd);
            return "Account has been deleted";
        }
        
        if (it->Status == UserStatus::UserStatus_INACTIVE) {
            flock(fd, LOCK_UN);
            close(fd);
            return "Account is inactive. Please contact administrator";
        }
        
        if (it->Status == UserStatus::UserStatus_Pending) {
            flock(fd, LOCK_UN);
            close(fd);
            return "Account is pending activation";
        }
        
        
        if (it->AccessCount >= 5) {
            flock(fd, LOCK_UN);
            close(fd);
            return "Account locked. Too many failed attempts. Please reset password";
        }
        
        std::cout << "Login: Creating hash for validation..." << std::endl;
        std::string hashedPassword = Utils::CreateSaltedHash(email, password);
        std::cout << "Login: Generated hash: " << hashedPassword << std::endl;
        std::cout << "Login: Stored hash: " << it->PasswordHash << std::endl;
        if (hashedPassword != it->PasswordHash) {
            it->AccessCount++;
            SaveToFile(users);
            
            flock(fd, LOCK_UN);
            close(fd);
            
            if (it->AccessCount >= 5) {
                return "Account locked. Too many failed attempts. Please reset password";
            }
            return "Invalid email or password. Attempts remaining: " + 
                   std::to_string(5 - it->AccessCount);
        }
        
        
        it->AccessCount = 0;
        it->UpdatedDate = std::time(nullptr);
        SaveToFile(users);
        
        flock(fd, LOCK_UN);
        close(fd);
        return "success";
        
    } catch (...) {
        return "System error: An unexpected error occurred";
    }
}

bool Users::AddUser(UserDto user) {
    try {
        if (EmailExists(user.Email)) return false;
        
        int fd = open(filename.c_str(), O_RDWR);
        if (fd == -1) return false;
        
        flock(fd, LOCK_EX);
        
        auto users = LoadFromFile();
        user.CreatedDate = std::time(nullptr);
        user.UpdatedDate = std::time(nullptr);
        user.PasswordHash = Utils::CreateSaltedHash(user.Email, user.PasswordHash);
        user.AccessCount = 0;
        users.push_back(user);
        SaveToFile(users);
        
        flock(fd, LOCK_UN);
        close(fd);
        return true;
    } catch (...) {
        return false;
    }
}

std::vector<UserDto> Users::GetAllUsers() {
    int fd = open(filename.c_str(), O_RDONLY);
    if (fd == -1) return {};
    
    flock(fd, LOCK_SH);
    auto users = LoadFromFile();
    flock(fd, LOCK_UN);
    close(fd);
    return users;
}

UserDto Users::GetUserById(int id) {
    auto users = GetAllUsers();
    auto it = std::find_if(users.begin(), users.end(),
        [id](const UserDto& user) { return user.UserId == id; });
    return it != users.end() ? *it : UserDto{};
}

UserDto Users::GetUserByEmail(const std::string& email) {
    auto users = GetAllUsers();
    auto it = std::find_if(users.begin(), users.end(),
        [&email](const UserDto& user) { return user.Email == email; });
    return it != users.end() ? *it : UserDto{};
}

std::vector<UserDto> Users::GetUsersByStatus(UserStatus status) {
    std::vector<UserDto> result;
    auto users = GetAllUsers();
    std::copy_if(users.begin(), users.end(), std::back_inserter(result),
        [status](const UserDto& user) { return user.Status == status; });
    return result;
}

bool Users::UpdateUser(const UserDto& user) {
    try {
        int fd = open(filename.c_str(), O_RDWR);
        if (fd == -1) return false;
        
        flock(fd, LOCK_EX);
        auto users = LoadFromFile();
        
        auto it = std::find_if(users.begin(), users.end(),
            [&user](const UserDto& u) { return u.UserId == user.UserId; });
            
        if (it != users.end()) {
            UserDto updatedUser = user;
            updatedUser.UpdatedDate = std::time(nullptr);
            *it = updatedUser;
            SaveToFile(users);
        }
        
        flock(fd, LOCK_UN);
        close(fd);
        return true;
    } catch (...) {
        return false;
    }
}

bool Users::UpdateUserStatus(int userId, UserStatus newStatus, const std::string& updatedBy) {
    try {
        auto user = GetUserById(userId);
        if (user.UserId == 0) return false;
        
        user.Status = newStatus;
        user.UpdatedBy = updatedBy;
        user.UpdatedDate = std::time(nullptr);
        
        return UpdateUser(user);
    } catch (...) {
        return false;
    }
}

bool Users::UpdatePassword(int userId, const std::string& newPassword) {
    try {
        auto user = GetUserById(userId);
        if (user.UserId == 0) return false;
        
        user.PasswordHash = Utils::CreateSaltedHash(user.Email, newPassword);
        user.UpdatedDate = std::time(nullptr);
        
        return UpdateUser(user);
    } catch (...) {
        return false;
    }
}

bool Users::SoftDeleteUser(int userId, const std::string& deletedBy) {
    return UpdateUserStatus(userId, UserStatus::UserStatus_DELETED, deletedBy);
}

bool Users::HardDeleteUser(int userId) {
    try {
        int fd = open(filename.c_str(), O_RDWR);
        if (fd == -1) return false;
        
        flock(fd, LOCK_EX);
        auto users = LoadFromFile();
        
        auto it = std::remove_if(users.begin(), users.end(),
            [userId](const UserDto& user) { return user.UserId == userId; });
        
        if (it != users.end()) {
            users.erase(it, users.end());
            SaveToFile(users);
        }
        
        flock(fd, LOCK_UN);
        close(fd);
        return true;
    } catch (...) {
        return false;
    }
}

bool Users::AddBorrowedBook(int userId, const std::string& bookId) {
    try {
        auto user = GetUserById(userId);
        if (user.UserId == 0) return false;
        
        user.BorrowedBooks.push_back(bookId);
        user.UpdatedDate = std::time(nullptr);
        
        return UpdateUser(user);
    } catch (...) {
        return false;
    }
}

bool Users::AddReturnedBook(int userId, const std::string& bookId) {
    try {
        auto user = GetUserById(userId);
        if (user.UserId == 0) return false;
        
        auto it = std::find(user.BorrowedBooks.begin(), user.BorrowedBooks.end(), bookId);
        if (it != user.BorrowedBooks.end()) {
            user.BorrowedBooks.erase(it);
            user.ReturnedBooks.push_back(bookId);
            user.UpdatedDate = std::time(nullptr);
            return UpdateUser(user);
        }
        return false;
    } catch (...) {
        return false;
    }
}

std::vector<std::string> Users::GetBorrowedBooks(int userId) {
    auto user = GetUserById(userId);
    return user.UserId != 0 ? user.BorrowedBooks : std::vector<std::string>{};
}

std::vector<std::string> Users::GetReturnedBooks(int userId) {
    auto user = GetUserById(userId);
    return user.UserId != 0 ? user.ReturnedBooks : std::vector<std::string>{};
}

void Users::SaveToFile(const std::vector<UserDto>& users) const {
    json j = json::array();
    for (const auto& user : users) {
        json userJson;
        userJson["UserId"] = user.UserId;
        userJson["FirstName"] = user.FirstName;
        userJson["LastName"] = user.LastName;
        userJson["PasswordHash"] = user.PasswordHash;
        userJson["Email"] = user.Email;
        userJson["Type"] = static_cast<int>(user.Type);
        userJson["Status"] = static_cast<int>(user.Status);
        userJson["CreatedBy"] = user.CreatedBy;
        userJson["CreatedDate"] = user.CreatedDate;
        userJson["UpdatedBy"] = user.UpdatedBy;
        userJson["UpdatedDate"] = user.UpdatedDate;
        userJson["Address"] = user.Address;
        userJson["PhoneNumber"] = user.PhoneNumber;
        userJson["BorrowedBooks"] = user.BorrowedBooks;
        userJson["ReturnedBooks"] = user.ReturnedBooks;
        userJson["AccessCount"] = user.AccessCount;
        j.push_back(userJson);
    }
    
    std::ofstream file(filename);
    file << std::setw(4) << j << std::endl;
}

std::vector<UserDto> Users::LoadFromFile() const {
    std::vector<UserDto> users;
    std::ifstream file(filename);
    if (!file.is_open()) return users;
    
    json j;
    file >> j;
    
    for (const auto& userJson : j) {
        UserDto user;
        user.UserId = userJson["UserId"];
        user.FirstName = userJson["FirstName"];
        user.LastName = userJson["LastName"];
        user.PasswordHash = userJson["PasswordHash"];
        user.Email = userJson["Email"];
        user.Type = static_cast<UserType>(userJson["Type"]);
        user.Status = static_cast<UserStatus>(userJson["Status"]);
        user.CreatedBy = userJson["CreatedBy"];
        user.CreatedDate = userJson["CreatedDate"];
        user.UpdatedBy = userJson["UpdatedBy"];
        user.UpdatedDate = userJson["UpdatedDate"];
        user.Address = userJson["Address"];
        user.PhoneNumber = userJson["PhoneNumber"];
        user.BorrowedBooks = userJson["BorrowedBooks"].get<std::vector<std::string>>();
        user.ReturnedBooks = userJson["ReturnedBooks"].get<std::vector<std::string>>();
        user.AccessCount = userJson.value("AccessCount", 0);
        users.push_back(user);
    }
    
    return users;
}

bool Users::UserExists(int userId) const {
    auto users = LoadFromFile();
    return std::any_of(users.begin(), users.end(),
        [userId](const UserDto& user) { return user.UserId == userId; });
}

bool Users::EmailExists(const std::string& email) const {
    auto users = LoadFromFile();
    return std::any_of(users.begin(), users.end(),
        [&email](const UserDto& user) { return user.Email == email; });
}