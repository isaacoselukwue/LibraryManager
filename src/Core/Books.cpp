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

std::string Books::GetSoundex(const std::string& word) const {
    if (word.empty()) return "";
    
    std::string result(4, '0');
    result[0] = std::toupper(word[0]);
    
    const std::string mapping = "01230120022455012623010202";
    int j = 1;
    char last = '0';
    
    for (size_t i = 1; i < word.length() && j < 4; i++) {
        char current = mapping[std::tolower(word[i]) - 'a'];
        if (current != '0' && current != last) {
            result[j++] = current;
            last = current;
        }
    }
    
    return result;
}

double Books::CalculateNGramSimilarity(const std::string& s1, const std::string& s2, int n) const {
    if (s1.empty() || s2.empty()) return 0.0;
    
    std::set<std::string> ngrams1, ngrams2;
    
    for (size_t i = 0; i <= s1.length() - n; i++)
        ngrams1.insert(s1.substr(i, n));
    for (size_t i = 0; i <= s2.length() - n; i++)
        ngrams2.insert(s2.substr(i, n));
    
    int common = 0;
    for (const auto& ngram : ngrams1) {
        if (ngrams2.find(ngram) != ngrams2.end())
            common++;
    }
    
    return (2.0 * common) / (ngrams1.size() + ngrams2.size());
}

std::vector<std::string> Books::Tokenize(const std::string& text) const {
    std::vector<std::string> tokens;
    std::stringstream ss(text);
    std::string token;
    while (ss >> token) {
        std::transform(token.begin(), token.end(), token.begin(), ::tolower);
        tokens.push_back(token);
    }
    return tokens;
}

double Books::CalculateSearchScore(const BooksDto& book, const std::string& query) const {
    auto queryTokens = Tokenize(query);
    double score = 0.0;

    std::string bookName = book.Name;
    std::string bookAuthor = book.Author;
    std::string bookPublisher = book.Publisher;
    std::transform(bookName.begin(), bookName.end(), bookName.begin(), ::tolower);
    std::transform(bookAuthor.begin(), bookAuthor.end(), bookAuthor.begin(), ::tolower);
    std::transform(bookPublisher.begin(), bookPublisher.end(), bookPublisher.begin(), ::tolower);
    
    for (const auto& queryToken : queryTokens) {
        // Exact match bonuses
        if (bookName.find(queryToken) != std::string::npos) score += 1.0;
        if (bookAuthor.find(queryToken) != std::string::npos) score += 0.8;
        if (book.Isbn.find(queryToken) != std::string::npos) score += 1.0;
        if (bookPublisher.find(queryToken) != std::string::npos) score += 0.5;
        
        // Fuzzy matches
        double titleScore = CalculateNGramSimilarity(queryToken, bookName, 2) * 0.6;
        double authorScore = CalculateNGramSimilarity(queryToken, bookAuthor, 2) * 0.4;
        double publisherScore = CalculateNGramSimilarity(queryToken, bookPublisher, 2) * 0.2;
        
        // Phonetic matching
        if (GetSoundex(queryToken) == GetSoundex(bookName) ||
            GetSoundex(queryToken) == GetSoundex(bookAuthor))
            score += 0.3;
            
        score += titleScore + authorScore + publisherScore;
    }
    
    return score / queryTokens.size();
}

std::vector<Books::SearchResult> Books::SearchBooks(const std::string& query, size_t limit) const {
    std::vector<SearchResult> results;
    auto books = LoadFromFile();
    
    std::cout << "Searching for: " << query << std::endl;
    std::cout << "Number of books in database: " << books.size() << std::endl;
    
    for (const auto& book : books) {
        double score = CalculateSearchScore(book, query);
        std::cout << "Score for book '" << book.Name << "': " << score << std::endl;
        
        if (score > 0.1) {
            results.push_back({book, score});
        }
    }
    
    std::sort(results.begin(), results.end());
    
    if (results.size() > limit) {
        results.resize(limit);
    }
    
    std::cout << "Found " << results.size() << " results" << std::endl;
    return results;
}