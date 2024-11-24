// src/Tests/UnitTests/BookTests.hpp
#ifndef BOOK_TESTS_HPP
#define BOOK_TESTS_HPP

#include <cassert>
#include <filesystem>
#include "../../Interfaces/Books.hpp"

namespace fs = std::filesystem;

class BookTests {
private:
    const std::string TEST_DIR = "./resources/test/database";
    const std::string TEST_FILE = TEST_DIR + "/test_books.json";

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

    void TestAddBook() {
        Books books(TEST_FILE);
        BooksDto book{};
        book.BookId = 1;
        book.Name = "Test Book";
        book.Isbn = "123-456";
        book.Author = "Test Author";
        book.Publisher = "Test Publisher";
        book.NoOfCopies = 1;
        book.DateCreated = std::time(nullptr);
        book.Status = BookStatus::BookStatus_ACTIVE;
        
        std::cout << "Adding book with ID: " << book.BookId << std::endl;
        bool result = books.AddBook(book);
        assert(result && "Add book failed");
        
        auto savedBook = books.GetBooksById(1);
        assert(savedBook.Name == "Test Book" && "Book data mismatch");
    }

    void TestGetBook() {
        Books books(TEST_FILE);
        auto book = books.GetBooksById(1);
        assert(book.BookId == 1 && "Get book failed");
    }

    void TestRemoveBook() {
        Books books(TEST_FILE);
        bool result = books.RemoveBook(1);
        assert(result && "Remove book failed");
    }

    void TestAddCopies() {
        Books books(TEST_FILE);
        bool result = books.AddBookCopies(1, 5);
        assert(result && "Add copies failed");
        
        auto book = books.GetBooksById(1);
        assert(book.NoOfCopies == 6 && "Copies not added correctly");
    }

public:
    void RunAllTests() {
        try {
            SetUp();
            std::cout << "Running book tests...\n";
            TestAddBook();
            TestGetBook();
            TestAddCopies();
            // TestRemoveBook();
            // TearDown();
            std::cout << "All book tests passed!\n";
        }
        catch (const std::exception& e) {
            std::cerr << "Test failed: " << e.what() << std::endl;
            TearDown();
            throw;
        }
    }
};

#endif