#ifndef TRANSACTIONS_HPP
#define TRANSACTIONS_HPP

#include <string>
#include <vector>

#include "Common.hpp"

using transactionsDto = struct TransactionsDto
{
    int TransactionId;
    int UserId;
    int BookId;
    std::time_t CreatedDate;
    std::time_t BorrowDate;  //this needs to be nullable
    std::time_t DueDate; //this needs to be nullable
    std::time_t ReturnDate;  //this needs to be nullable
    std::time_t ActualReturnDate;  //this needs to be nullable
    BorrowStatus Status;
};

class Transactions
{
    public:
        Transactions();
        Transactions(const std::string& filename);
        ~Transactions();

        std::string AddTransaction(TransactionsDto transaction);
        std::string UpdateTransaction(TransactionsDto transaction);
        std::string RemoveTransaction(int transactionId);
        std::vector<TransactionsDto> GetAllTransactions();
        std::vector<TransactionsDto> GetTransactionsByUserId(int userId);
        TransactionsDto GetTransactionById(int transactionId);
        std::vector<TransactionsDto> GetTransactionsByStatus(BorrowStatus status);
        std::vector<TransactionsDto> GetTransactionsByBookId(const int& bookId);
        std::vector<TransactionsDto> GetTransactionsByDate(const std::time_t& startDate, const std::time_t& endDate);
        std::vector<TransactionsDto> GetTransactionsByDateAndUserId(const std::time_t& startDate, const std::time_t& endDate, const int& userId);
        std::vector<TransactionsDto> GetTransactionsByDueDate(const std::time_t& startDate, const std::time_t& endDate);
        std::vector<TransactionsDto> GetTransactionsByDueDateAndUserId(const std::time_t& startDate, const std::time_t& endDate, const int& userId);

    private:
        std::string filename;
        void SaveToFile(const std::vector<TransactionsDto>& transactions) const;
        std::vector<TransactionsDto> LoadFromFile() const;
        int GetNextTransactionId() const;
};

#endif