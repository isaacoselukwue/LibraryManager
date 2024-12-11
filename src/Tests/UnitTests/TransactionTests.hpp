#ifndef TRANSACTION_TESTS_HPP
#define TRANSACTION_TESTS_HPP

#include <cassert>
#include <filesystem>
#include "../../Interfaces/Transactions.hpp"

namespace fs = std::filesystem;

class TransactionTests {
private:
    const std::string TEST_DIR = "./resources/test/database";
    const std::string TEST_FILE = TEST_DIR + "/test_transactions.json";

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

    void TestAddTransaction() {
        Transactions transactions(TEST_FILE);
        
        TransactionsDto transaction{};
        transaction.UserId = 1;
        transaction.BookId = 1;
        transaction.BorrowDate = std::time(nullptr);
        transaction.DueDate = transaction.BorrowDate + (7 * 24 * 60 * 60); // 7 days
        transaction.Status = BorrowStatus::BorrowStatus_BORROWED;
        
        std::string result = transactions.AddTransaction(transaction);
        std::cout << "Add transaction result: " << result << std::endl;
        assert(result == "success" && "Add transaction failed");
        
        auto savedTransaction = transactions.GetTransactionById(1);
        assert(savedTransaction.UserId == 1 && "Transaction data mismatch");
        
        std::cout << "Add transaction test passed\n";
    }

    void TestUpdateTransaction() {
        Transactions transactions(TEST_FILE);
        auto transaction = transactions.GetTransactionById(1);
        transaction.Status = BorrowStatus::BorrowStatus_RETURNED;
        transaction.ReturnDate = std::time(nullptr);
        transaction.ActualReturnDate = transaction.ReturnDate;
        
        std::string result = transactions.UpdateTransaction(transaction);
        assert(result == "success" && "Update transaction failed");
        
        auto updatedTransaction = transactions.GetTransactionById(1);
        assert(updatedTransaction.Status == BorrowStatus::BorrowStatus_RETURNED && 
               "Transaction status not updated");
        
        std::cout << "Update transaction test passed\n";
    }

    void TestDateQueries() {
        Transactions transactions(TEST_FILE);
        std::time_t now = std::time(nullptr);
        std::time_t tomorrow = now + (24 * 60 * 60);
        
        auto dateResults = transactions.GetTransactionsByDate(now - (24 * 60 * 60), tomorrow);
        assert(!dateResults.empty() && "Date query returned no results");
        
        auto userDateResults = transactions.GetTransactionsByDateAndUserId(
            now - (24 * 60 * 60), tomorrow, 1);
        assert(!userDateResults.empty() && "User date query returned no results");
        
        std::cout << "Date queries test passed\n";
    }

    void TestStatusQueries() {
        Transactions transactions(TEST_FILE);
        
        auto borrowedTransactions = transactions.GetTransactionsByStatus(
            BorrowStatus::BorrowStatus_BORROWED);
        auto returnedTransactions = transactions.GetTransactionsByStatus(
            BorrowStatus::BorrowStatus_RETURNED);
            
        assert((borrowedTransactions.size() + returnedTransactions.size()) > 0 && 
               "No transactions found by status");
               
        std::cout << "Status queries test passed\n";
    }

    void TestUserAndBookQueries() {
        Transactions transactions(TEST_FILE);
        
        auto userTransactions = transactions.GetTransactionsByUserId(1);
        assert(!userTransactions.empty() && "No transactions found for user");
        
        auto bookTransactions = transactions.GetTransactionsByBookId(1);
        assert(!bookTransactions.empty() && "No transactions found for book");
        
        std::cout << "User and book queries test passed\n";
    }

public:
    void RunAllTests() {
        try {
            SetUp();
            std::cout << "Running transaction tests...\n";
            TestAddTransaction();
            TestUpdateTransaction();
            TestDateQueries();
            TestStatusQueries();
            TestUserAndBookQueries();
            // TearDown();
            std::cout << "All transaction tests passed!\n";
        }
        catch (const std::exception& e) {
            std::cerr << "Test failed: " << e.what() << std::endl;
            TearDown();
            throw;
        }
    }
};

#endif