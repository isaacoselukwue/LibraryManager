#ifndef USER_TESTS_HPP
#define USER_TESTS_HPP

#include <cassert>
#include <filesystem>
#include "../../Interfaces/Users.hpp"
#include "../../Utils/HashUtils.hpp"

namespace fs = std::filesystem;

class UserTests {
private:
    const std::string TEST_DIR = "./resources/test/database";
    const std::string TEST_FILE = TEST_DIR + "/test_users.json";

    void SetUp() {
        fs::create_directories(TEST_DIR);
        std::ofstream file(TEST_FILE);
        file << "[]";
        file.close();
        std::cout << "Test file created at: " << TEST_FILE << std::endl;
    }

    void TearDown() {
        if (fs::exists(TEST_FILE)) {
            fs::remove(TEST_FILE);
        }
    }

    void TestAddUser() {
        Users users(TEST_FILE);
        UserDto user{};
        user.UserId = 1;
        user.FirstName = "John";
        user.LastName = "Doe";
        user.Email = "john.doe@test.com";
        user.PasswordHash = "password123";
        user.Type = UserType::UserType_USERS;
        user.Status = UserStatus::UserStatus_ACTIVE;
        user.CreatedBy = "System";
        
        std::cout << "Adding user with ID: " << user.UserId << std::endl;
        bool result = users.AddUser(user);
        assert(result && "Add user failed");
        
        auto savedUser = users.GetUserById(1);
        assert(savedUser.Email == "john.doe@test.com" && "User data mismatch");
    }

    void TestLogin() {
        std::cout << "Setting up login test...\n";
        Users users(TEST_FILE);
        
        // Setup test user
        UserDto user{};
        user.UserId = 2;
        user.Email = "test10@test.com";
        user.FirstName = "Test";
        user.LastName = "User";
        user.Status = UserStatus::UserStatus_ACTIVE;
        std::string password = "password123";
        user.PasswordHash = password;
        user.CreatedDate = std::time(nullptr);
        user.AccessCount = 0;
        
        std::cout << "Adding test user with hash: " << user.PasswordHash << std::endl;
        bool addResult = users.AddUser(user);
        assert(addResult && "Failed to add test user");
        
        // Test successful login
        std::cout << "Attempting login...\n";
        std::string result = users.Login(user.Email, password);
        std::cout << "Login result: " << result << std::endl;
        assert(result == "success" && "Login should succeed");

        std::cout << "Login with correct pwd successful.\n";

        // Test wrong password
        result = users.Login(user.Email, "wrongpass");
        assert(result.find("Invalid email or password") != std::string::npos && "Wrong password should fail");
        
        // Test access count increment
        for(int i = 0; i < 4; i++) {
            users.Login(user.Email, "wrongpass");
        }
        result = users.Login(user.Email, "wrongpass");
        assert(result.find("Account locked") != std::string::npos && "Account should be locked after 5 attempts");
        
        // Test successful login resets access count
        auto currentUser = users.GetUserByEmail(user.Email);
        currentUser.AccessCount = 0;
        users.UpdateUser(currentUser);
        std::cout << "Current users details: " << currentUser.PasswordHash << std::endl;
        result = users.Login(user.Email, password);
        assert(result == "success" && "Login should succeed after reset");
        
        // Test inactive account
        user.Status = UserStatus::UserStatus_INACTIVE;
        users.UpdateUser(user);
        result = users.Login(user.Email, password);
        assert(result.find("Account is inactive") != std::string::npos && "Inactive account should not login");
        
        // Test deleted account
        user.Status = UserStatus::UserStatus_DELETED;
        users.UpdateUser(user);
        result = users.Login(user.Email, password);
        assert(result.find("Account has been deleted") != std::string::npos && "Deleted account should not login");
        
        currentUser = users.GetUserByEmail(user.Email);
        currentUser.Status = UserStatus::UserStatus_ACTIVE;
        users.UpdateUser(currentUser);

        std::string wrongPassword = "wrongpass";
        std::cout << "\nTesting wrong password:" << std::endl;
        std::cout << "Wrong password: " << wrongPassword << std::endl;
        std::cout << "Expected hash: " << Utils::CreateSaltedHash(user.Email, wrongPassword) << std::endl;
        
        result = users.Login(user.Email, wrongPassword);
        std::cout << "Login result with wrong password: " << result << std::endl;
        assert(result.find("Invalid email or password") != std::string::npos && "Wrong password should fail");
    }

    void TestGetUser() {
        Users users(TEST_FILE);
        auto user = users.GetUserById(1);
        assert(user.UserId == 1 && "Get user failed");
    }

    void TestUpdateUser() {
        Users users(TEST_FILE);
        auto user = users.GetUserById(1);
        user.FirstName = "Jane";
        bool result = users.UpdateUser(user);
        assert(result && "Update user failed");
        
        auto updatedUser = users.GetUserById(1);
        assert(updatedUser.FirstName == "Jane" && "User update mismatch");
    }

    void TestUpdatePassword() {
        Users users(TEST_FILE);
        bool result = users.UpdatePassword(1, "newpassword123");
        assert(result && "Password update failed");
    }

    void TestSoftDelete() {
        Users users(TEST_FILE);
        bool result = users.SoftDeleteUser(1, "Admin");
        assert(result && "Soft delete failed");
        
        auto user = users.GetUserById(1);
        assert(user.Status == UserStatus::UserStatus_DELETED && "User status not updated");
    }

    void TestBookOperations() {
        Users users(TEST_FILE);
        bool result = users.AddBorrowedBook(1, "BOOK-001");
        assert(result && "Add borrowed book failed");
        
        auto borrowed = users.GetBorrowedBooks(1);
        assert(!borrowed.empty() && borrowed[0] == "BOOK-001" && "Borrowed books mismatch");
        
        result = users.AddReturnedBook(1, "BOOK-001");
        assert(result && "Return book failed");
        
        auto returned = users.GetReturnedBooks(1);
        assert(!returned.empty() && returned[0] == "BOOK-001" && "Returned books mismatch");
    }

public:
    void RunAllTests() {
        try {
            SetUp();
            std::cout << "Running user tests...\n";
            TestAddUser();
            TestGetUser();
            TestUpdateUser();
            TestUpdatePassword();
            TestBookOperations();
            TestLogin();
            TestSoftDelete();
            // TearDown();
            std::cout << "All user tests passed!\n";
        }
        catch (const std::exception& e) {
            std::cerr << "Test failed: " << e.what() << std::endl;
            TearDown();
            throw;
        }
    }
};

#endif