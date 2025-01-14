# Library Management System

A C++ based library management system that provides both server and client functionality for managing books, users, and transactions in a library setting.

## Features

- User Authentication (Login/Register)
- Book Management (Add/Remove/Search)
- Category Management
- User Management (Admin features)
- Transaction Tracking (Borrow/Return)
- Multi-user support via Client/Server architecture

### User Types

- **Regular Users**: Can browse books, borrow/return books, view history
- **Admin Users**: Additional privileges for managing books, categories, and users

## Requirements

- C++17 or higher
- G++ compiler
- Make
- POSIX-compliant system (Linux/Unix)
- nlohmann/json library (automatically downloaded during build)

## Building the Application

```bash
# Clone the repository
git clone <repository-url>
cd LibManager

# Build the application
make clean && make

```
## Building the Application

### Start the Server
```
./build/library server
```

### Start the Client
```
./build/library client
```

### Run Unit Tests
```
./build/library 1
```

## Usage Guide
### Regular User Commands
- Search Books
- Borrow Book
- Return Book
- View Borrowed Books
- View Returned Books
- Logout
- Change Password

### Admin User Commands
- Additional commands for admin users: 6. Add Book 7. Remove Book 8. Add Category 9. Manage Users

### User Management Commands
- Activate User
- Deactivate User
- Delete User
- Change to Admin
- Change to User
- View User Transactions
- View All Transactions
- Hard Delete User

## Technical Details
- Client-Server architecture using TCP/IP
- File-based storage using JSON
- Thread-safe operations
- Password encryption
- Session management
- Concurrent user support

## Testing
The system includes comprehensive unit tests for:

- Book Management
- Category Management
- User Management
- Transaction Management

To run all unit tests:
```bash
./build/library 1
```