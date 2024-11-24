#ifndef COMMON_HPP
#define COMMON_HPP
#include <iostream>
#include <iomanip>
#include <sstream>
#include <ctime>

enum BookStatus : int {
	BookStatus_PENDING = 0,
	BookStatus_ACTIVE,
	BookStatus_DELETED
};

enum BorrowStatus : int
{
	BorrowStatus_BORROWED = 0,
	BorrowStatus_RETURNED,
	BorrowStatus_UNAVAILABLE
};

enum UserType : int {
	UserType_ADMIN = 0,
	UserType_STAFF,
	UserType_USERS,
	UserType_MANAGER
};

enum UserStatus : int
{
	UserStatus_Pending,
	UserStatus_ACTIVE,
	UserStatus_INACTIVE,
	UserStatus_DELETED
};




struct AuditLogDto
{
    int AuditLogId;
    std::string Action;
    std::string Description;
    std::time_t DateCreated;
};
#endif