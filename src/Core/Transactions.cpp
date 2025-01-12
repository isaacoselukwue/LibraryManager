#include <nlohmann/json.hpp>
#include <filesystem>
#include <fstream>
#include <sys/file.h>
#include <fcntl.h>
#include <unistd.h>
#include <optional>

#include "../Interfaces/Transactions.hpp"

using json = nlohmann::json;
namespace fs = std::filesystem;

Transactions::Transactions() {
    filename = "./resources/database/transactions.json";
    fs::create_directories("./resources/database");
    if (!fs::exists(filename)) {
        std::ofstream file(filename);
        file << "[]";
        file.close();
    }
}

Transactions::Transactions(const std::string& fname) : filename(fname) {}

Transactions::~Transactions() {}

std::string Transactions::AddTransaction(TransactionsDto transaction) {
    try {
        int fd = open(filename.c_str(), O_RDWR);
        if (fd == -1) return "Error: Unable to access database";
        
        flock(fd, LOCK_EX);
        auto transactions = LoadFromFile();
        
        transaction.TransactionId = GetNextTransactionId();
        transaction.BorrowDate = std::time(nullptr);
        transaction.CreatedDate = std::time(nullptr);
        transaction.DueDate = std::time(nullptr) + (5 * 24 * 60 * 60);
        
        transactions.push_back(transaction);
        SaveToFile(transactions);
        
        flock(fd, LOCK_UN);
        close(fd);
        return "success";
    } catch (...) {
        return "Error: Failed to add transaction";
    }
}

std::string Transactions::UpdateTransaction(TransactionsDto transaction) {
    try {
        int fd = open(filename.c_str(), O_RDWR);
        if (fd == -1) return "Error: Unable to access database";
        
        flock(fd, LOCK_EX);
        auto transactions = LoadFromFile();
        
        auto it = std::find_if(transactions.begin(), transactions.end(),
            [&transaction](const TransactionsDto& t) { 
                return t.TransactionId == transaction.TransactionId; 
            });
            
        if (it != transactions.end()) {
            *it = transaction;
            SaveToFile(transactions);
        }
        
        flock(fd, LOCK_UN);
        close(fd);
        return "success";
    } catch (...) {
        return "Error: Failed to update transaction";
    }
}

std::string Transactions::RemoveTransaction(int transactionId) {
    try {
        int fd = open(filename.c_str(), O_RDWR);
        if (fd == -1) return "Error: Unable to access database";
        
        flock(fd, LOCK_EX);
        auto transactions = LoadFromFile();
        
        auto it = std::remove_if(transactions.begin(), transactions.end(),
            [transactionId](const TransactionsDto& t) { 
                return t.TransactionId == transactionId; 
            });
            
        if (it != transactions.end()) {
            transactions.erase(it, transactions.end());
            SaveToFile(transactions);
        }
        
        flock(fd, LOCK_UN);
        close(fd);
        return "success";
    } catch (...) {
        return "Error: Failed to remove transaction";
    }
}

std::vector<TransactionsDto> Transactions::GetAllTransactions() {
    try {
        return LoadFromFile();
    } catch (...) {
        return std::vector<TransactionsDto>();
    }
}

TransactionsDto Transactions::GetTransactionById(int transactionId) {
    auto transactions = LoadFromFile();
    auto it = std::find_if(transactions.begin(), transactions.end(),
        [transactionId](const TransactionsDto& t) { 
            return t.TransactionId == transactionId; 
        });
    return it != transactions.end() ? *it : TransactionsDto{};
}

std::vector<TransactionsDto> Transactions::GetTransactionsByUserId(int userId) {
    std::vector<TransactionsDto> result;
    auto transactions = LoadFromFile();
    std::copy_if(transactions.begin(), transactions.end(), std::back_inserter(result),
        [userId](const TransactionsDto& t) { return t.UserId == userId; });
    return result;
}

std::vector<TransactionsDto> Transactions::GetTransactionsByStatus(BorrowStatus status) {
    std::vector<TransactionsDto> result;
    auto transactions = LoadFromFile();
    std::copy_if(transactions.begin(), transactions.end(), std::back_inserter(result),
        [status](const TransactionsDto& t) { return t.Status == status; });
    return result;
}

std::vector<TransactionsDto> Transactions::GetTransactionsByBookId(const int& bookId) {
    std::vector<TransactionsDto> result;
    auto transactions = LoadFromFile();
    std::copy_if(transactions.begin(), transactions.end(), std::back_inserter(result),
        [&bookId](const TransactionsDto& t) { return t.BookId == bookId; });
    return result;
}

TransactionsDto Transactions::GetBorrowedTransactionsByUserAndBookId(const int& userId, const int& bookId) {
    auto transactions = LoadFromFile();
    auto it = std::find_if(transactions.begin(), transactions.end(),
        [&](const TransactionsDto& t) { 
            return t.UserId == userId && 
                   t.BookId == bookId && 
                   t.Status == BorrowStatus::BorrowStatus_BORROWED;
        });
        
    if (it != transactions.end()) {
        return *it;
    }
    return TransactionsDto{};
}

TransactionsDto Transactions::GetReturnedTransactionsByUserAndBookId(const int& userId, const int& bookId) {
    auto transactions = LoadFromFile();
    auto it = std::find_if(transactions.begin(), transactions.end(),
        [&](const TransactionsDto& t) { 
            return t.UserId == userId && 
                   t.BookId == bookId && 
                   t.Status == BorrowStatus::BorrowStatus_RETURNED;
        });
        
    if (it != transactions.end()) {
        return *it;
    }
    return TransactionsDto{};
}

std::vector<TransactionsDto> Transactions::GetTransactionsByDate(
    const std::time_t& startDate, const std::time_t& endDate) {
    std::vector<TransactionsDto> result;
    auto transactions = LoadFromFile();
    std::copy_if(transactions.begin(), transactions.end(), std::back_inserter(result),
        [startDate, endDate](const TransactionsDto& t) { 
            return t.BorrowDate >= startDate && t.BorrowDate <= endDate; 
        });
    return result;
}

std::vector<TransactionsDto> Transactions::GetTransactionsByDateAndUserId(
    const std::time_t& startDate, const std::time_t& endDate, const int& userId) {
    std::vector<TransactionsDto> result;
    auto transactions = LoadFromFile();
    std::copy_if(transactions.begin(), transactions.end(), std::back_inserter(result),
        [startDate, endDate, userId](const TransactionsDto& t) { 
            return t.UserId == userId && 
                   t.BorrowDate >= startDate && 
                   t.BorrowDate <= endDate; 
        });
    return result;
}

std::vector<TransactionsDto> Transactions::GetTransactionsByDueDate(
    const std::time_t& startDate, const std::time_t& endDate) {
    std::vector<TransactionsDto> result;
    auto transactions = LoadFromFile();
    std::copy_if(transactions.begin(), transactions.end(), std::back_inserter(result),
        [startDate, endDate](const TransactionsDto& t) { 
            return t.DueDate >= startDate && t.DueDate <= endDate; 
        });
    return result;
}

std::vector<TransactionsDto> Transactions::GetTransactionsByDueDateAndUserId(
    const std::time_t& startDate, const std::time_t& endDate, const int& userId) {
    std::vector<TransactionsDto> result;
    auto transactions = LoadFromFile();
    std::copy_if(transactions.begin(), transactions.end(), std::back_inserter(result),
        [startDate, endDate, userId](const TransactionsDto& t) { 
            return t.UserId == userId && 
                   t.DueDate >= startDate && 
                   t.DueDate <= endDate; 
        });
    return result;
}

void Transactions::SaveToFile(const std::vector<TransactionsDto>& transactions) const {
    json j = json::array();
    for (const auto& transaction : transactions) {
        json transactionJson;
        transactionJson["TransactionId"] = transaction.TransactionId;
        transactionJson["UserId"] = transaction.UserId;
        transactionJson["BookId"] = transaction.BookId;
        transactionJson["BorrowDate"] = transaction.BorrowDate;
        transactionJson["DueDate"] = transaction.DueDate;
        transactionJson["ReturnDate"] = transaction.ReturnDate;
        transactionJson["ActualReturnDate"] = transaction.ActualReturnDate;
        transactionJson["Status"] = static_cast<int>(transaction.Status);
        j.push_back(transactionJson);
    }
    
    std::ofstream file(filename);
    file << std::setw(4) << j << std::endl;
}

std::vector<TransactionsDto> Transactions::LoadFromFile() const {
    std::vector<TransactionsDto> transactions;
    std::ifstream file(filename);
    if (!file.is_open()) return transactions;
    
    json j;
    file >> j;
    
    for (const auto& transactionJson : j) {
        TransactionsDto transaction;
        transaction.TransactionId = transactionJson["TransactionId"];
        transaction.UserId = transactionJson["UserId"];
        transaction.BookId = transactionJson["BookId"];
        transaction.BorrowDate = transactionJson["BorrowDate"];
        transaction.DueDate = transactionJson["DueDate"];
        transaction.ReturnDate = transactionJson["ReturnDate"];
        transaction.ActualReturnDate = transactionJson["ActualReturnDate"];
        transaction.Status = static_cast<BorrowStatus>(transactionJson["Status"]);
        transactions.push_back(transaction);
    }
    
    return transactions;
}

int Transactions::GetNextTransactionId() const {
    auto transactions = LoadFromFile();
    if (transactions.empty()) return 1;
    
    return std::max_element(transactions.begin(), transactions.end(),
        [](const TransactionsDto& a, const TransactionsDto& b) { 
            return a.TransactionId < b.TransactionId; 
        })->TransactionId + 1;
}