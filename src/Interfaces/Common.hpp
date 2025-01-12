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

enum class SessionState {
    INITIAL,
    LOGIN_EMAIL,
    LOGIN_PASSWORD,
    REGISTER_FIRST_NAME,
    REGISTER_LAST_NAME,
    REGISTER_ADDRESS,
    REGISTER_PHONE,
    REGISTER_EMAIL,
    REGISTER_PASSWORD,
    AUTHENTICATED,
    WAITING_SEARCH_TERM,
    WAITING_BOOK_ID,
    WAITING_BOOK_NAME,
    WAITING_BOOK_AUTHOR,
    WAITING_CATEGORY_NAME,
    WAITING_BOOK_ISBN,
    WAITING_BOOK_PUBLISHER,
    WAITING_BOOK_COPIES,
    WAITING_BOOK_CATEGORIES,
    WAITING_CATEGORY_DESCRIPTION,
    WAITING_USER_ID,
    WAITING_NEW_PASSWORD
};

enum class UserCommand {
    NONE = 0,
    SEARCH_BOOKS = 1,
    BORROW_BOOK = 2,
    RETURN_BOOK = 3,
    VIEW_BORROWED = 4,
    VIEW_RETURNED = 5,
    ADD_BOOK = 6,
    REMOVE_BOOK = 7,
    ADD_CATEGORY = 8,
    MANAGE_USERS = 9,
    LOGOUT = 10,
    ACTIVATE_USER = 11,
    DEACTIVATE_USER = 12,
    DELETE_USER = 13,
    CHANGE_TO_ADMIN = 14,
    CHANGE_TO_USER = 15,
    VIEW_USER_TRANSACTIONS = 16,
    VIEW_ALL_TRANSACTIONS = 17,
    HARD_DELETE_USER = 18,
    HARD_DELETE_USER_CONFIRMED = 19,
    CHANGE_PASSWORD = 20
};

enum class MenuType{
    INITIAL,
    MAIN,
    USER_MANAGEMENT,
    BOOK_MANAGEMENT
};

struct AuditLogDto
{
    int AuditLogId;
    std::string Action;
    std::string Description;
    std::time_t DateCreated;
};
#endif