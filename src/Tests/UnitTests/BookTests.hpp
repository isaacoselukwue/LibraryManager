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

    void TestSearchBooks() {
        Books books(TEST_FILE);
        
        // Setup test books
        std::vector<BooksDto> testBooks = {
            {
                .BookId = 1,
                .Name = "Harry Potter and the Sorcerer's Stone",
                .Isbn = "978-0590353427",
                .Author = "J.K. Rowling",
                .Publisher = "Scholastic",
                .NoOfCopies = 5,
                .DateCreated = std::time(nullptr),
                .DateUpdated = std::time(nullptr),
                .Status = BookStatus::BookStatus_ACTIVE,
                .Categories = {}
            },
            {
                .BookId = 2,
                .Name = "The Hobbit",
                .Isbn = "978-0547928227",
                .Author = "J.R.R. Tolkien",
                .Publisher = "Houghton Mifflin",
                .NoOfCopies = 3,
                .DateCreated = std::time(nullptr),
                .DateUpdated = std::time(nullptr),
                .Status = BookStatus::BookStatus_ACTIVE,
                .Categories = {},
            },
            {
                .BookId = 3,
                .Name = "1984",
                .Isbn = "978-0451524935",
                .Author = "George Orwell",
                .Publisher = "Signet Classic",
                .NoOfCopies = 4,
                .DateCreated = std::time(nullptr),
                .DateUpdated = std::time(nullptr),
                .Status = BookStatus::BookStatus_ACTIVE,
                .Categories = {}
            }
        };
        
        for (const auto& book : testBooks) {
            bool result = books.AddBook(book);
            assert(result && "Failed to add test book");
        }

        std::cout << "Testing book search...\n";

        // Test exact title match
        auto results = books.SearchBooks("Harry Potter");
        assert(!results.empty() && "Should find Harry Potter");
        assert(results[0].book.Name.find("Harry Potter") != std::string::npos && "First result should be Harry Potter");
        
        // Test partial match
        results = books.SearchBooks("harry");
        assert(!results.empty() && "Should find Harry Potter with partial match");
        assert(results[0].book.Name.find("Harry") != std::string::npos && "First result should be Harry Potter");
        
        // Test phonetic/fuzzy match
        results = books.SearchBooks("hary poter");
        assert(!results.empty() && "Should find Harry Potter despite typo");
        assert(results[0].book.Name.find("Harry Potter") != std::string::npos && "First result should be Harry Potter");
        
        // Test author search
        results = books.SearchBooks("Tolkien");
        assert(!results.empty() && "Should find book by Tolkien");
        assert(results[0].book.Author.find("Tolkien") != std::string::npos && "First result should be by Tolkien");
        
        // Test ISBN search
        results = books.SearchBooks("978-0590353427");
        assert(!results.empty() && "Should find book by ISBN");
        assert(results[0].book.Isbn == "978-0590353427" && "First result should match ISBN");
        
        // Test publisher search
        results = books.SearchBooks("Scholastic");
        assert(!results.empty() && "Should find book by publisher");
        assert(results[0].book.Publisher == "Scholastic" && "First result should be from Scholastic");
        
        // Test multi-term search
        results = books.SearchBooks("george 1984");
        assert(!results.empty() && "Should find book by multiple terms");
        assert(results[0].book.Name == "1984" && results[0].book.Author.find("George") != std::string::npos && 
            "First result should match both terms");
        
        // Test result limit
        results = books.SearchBooks("the", 1);
        assert(results.size() == 1 && "Should respect result limit");
        
        // Test relevance sorting
        results = books.SearchBooks("the");
        assert(results.size() > 1 && "Should find multiple results");
        assert(results[0].score >= results[1].score && "Results should be sorted by relevance");
        
        std::cout << "Book search tests passed!\n";
    }

public:
    void RunAllTests() {
        try {
            SetUp();
            std::cout << "Running book tests...\n";
            TestAddBook();
            TestGetBook();
            TestAddCopies();
            TestSearchBooks();
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