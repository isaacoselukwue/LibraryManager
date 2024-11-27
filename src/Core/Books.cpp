#include <nlohmann/json.hpp>
#include <filesystem>
#include <sys/file.h>
#include <fcntl.h>
#include <unistd.h>

#include "../Interfaces/Books.hpp"

using json = nlohmann::json;
namespace fs = std::filesystem;

Books::Books()
{
    filename = "./resources/database/books.json";
    fs::create_directories("./resources/database");
    if (!fs::exists(filename)) {
        std::ofstream file(filename);
        file << "[]";
        file.close();
    }
}

Books::Books(const std::string& fname) : filename(fname) {}

Books::~Books() {}

int Books::GetNextBookId() const {
    auto books = LoadFromFile();
    if (books.empty()) return 1;
    
    return std::max_element(books.begin(), books.end(),
        [](const BooksDto& a, const BooksDto& b) { 
            return a.BookId < b.BookId; 
        })->BookId + 1;
}

bool Books::AddBook(BooksDto book) {
    try {
        int fd = open(filename.c_str(), O_RDWR);
        if (fd == -1) return false;
        
        // Lock file for writing
        flock(fd, LOCK_EX);
        
        auto books = LoadFromFile();
        book.BookId = GetNextBookId();
        book.DateCreated = std::time(nullptr);
        book.DateUpdated = std::time(nullptr);
        books.push_back(book);
        SaveToFile(books);
        
        // Unlock and close
        flock(fd, LOCK_UN);
        close(fd);
        return true;
    } catch (...) {
        return false;
    }
}

std::vector<BooksDto> Books::GetAllBooks() {
    int fd = open(filename.c_str(), O_RDONLY);
    if (fd == -1) return {};
    
    // Shared lock for reading
    flock(fd, LOCK_SH);
    auto books = LoadFromFile();
    flock(fd, LOCK_UN);
    close(fd);
    return books;
}

BooksDto Books::GetBooksById(int id) {
    auto books = GetAllBooks();
    auto it = std::find_if(books.begin(), books.end(),
        [id](const BooksDto& book) { return book.BookId == id; });
    return it != books.end() ? *it : BooksDto{};
}

bool Books::RemoveBook(int bookId) {
    try {
        int fd = open(filename.c_str(), O_RDWR);
        if (fd == -1) return false;
        
        flock(fd, LOCK_EX);
        auto books = LoadFromFile();
        
        auto it = std::remove_if(books.begin(), books.end(),
            [bookId](const BooksDto& book) { return book.BookId == bookId; });
        
        if (it != books.end()) {
            books.erase(it, books.end());
            SaveToFile(books);
        }
        
        flock(fd, LOCK_UN);
        close(fd);
        return true;
    } catch (...) {
        return false;
    }
}

void Books::SaveToFile(const std::vector<BooksDto>& books) const {
    json j = json::array();
    for (const auto& book : books) {
        json bookJson;
        bookJson["BookId"] = book.BookId;
        bookJson["Name"] = book.Name;
        bookJson["Isbn"] = book.Isbn;
        bookJson["Author"] = book.Author;
        bookJson["Publisher"] = book.Publisher;
        bookJson["NoOfCopies"] = book.NoOfCopies;
        bookJson["DateCreated"] = book.DateCreated;
        bookJson["DateUpdated"] = book.DateUpdated;
        bookJson["Status"] = static_cast<int>(book.Status);
        bookJson["Categories"] = json::array();
        for (const auto& category : book.Categories) {
            json categoryJson;
            categoryJson["CategoryId"] = category.CategoryId;
            categoryJson["Name"] = category.Name;
            categoryJson["Description"] = category.Description;
            categoryJson["DateCreated"] = category.DateCreated;
            bookJson["Categories"].push_back(categoryJson);
        }
        j.push_back(bookJson);
    }
    
    std::ofstream file(filename);
    file << std::setw(4) << j << std::endl;
}

std::vector<BooksDto> Books::LoadFromFile() const {
    std::vector<BooksDto> books;
    std::ifstream file(filename);
    if (!file.is_open()) return books;
    
    json j;
    file >> j;
    
    for (const auto& bookJson : j) {
        BooksDto book;
        book.BookId = bookJson["BookId"];
        book.Name = bookJson["Name"];
        book.Isbn = bookJson["Isbn"];
        book.Author = bookJson["Author"];
        book.Publisher = bookJson["Publisher"];
        book.NoOfCopies = bookJson["NoOfCopies"];
        book.DateCreated = bookJson["DateCreated"];
        book.DateUpdated = bookJson["DateUpdated"];
        book.Status = static_cast<BookStatus>(bookJson["Status"]);
        
        for (const auto& categoryJson : bookJson["Categories"]) {
            CategoryDto category;
            category.CategoryId = categoryJson["CategoryId"];
            category.Name = categoryJson["Name"];
            category.Description = categoryJson["Description"]; 
            category.DateCreated = categoryJson["DateCreated"];
            book.Categories.push_back(category);
        }
        books.push_back(book);
    }
    
    return books;
}

bool Books::AddBookCopies(int bookId, int copies) {
    try {
        int fd = open(filename.c_str(), O_RDWR);
        if (fd == -1) return false;
        
        flock(fd, LOCK_EX);
        auto books = LoadFromFile();
        
        auto it = std::find_if(books.begin(), books.end(),
            [bookId](const BooksDto& book) { return book.BookId == bookId; });
            
        if (it != books.end()) {
            it->NoOfCopies += copies;
            it->DateUpdated = std::time(nullptr);
            SaveToFile(books);
        }
        
        flock(fd, LOCK_UN);
        close(fd);
        return true;
    } catch (...) {
        return false;
    }
}

bool Books::RemoveBookCopies(int bookId, int copies) {
    return AddBookCopies(bookId, -copies);
}