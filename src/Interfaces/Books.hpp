#ifndef BOOKS_HPP
#define BOOKS_HPP

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
#include <set>
#include <cctype>
#include <sstream>

#include "Common.hpp"
#include "Categories.hpp"

using BooksDto = struct BooksDto
{
	int BookId;
	std::string Name;
	std::string Isbn;
    std::string Author;
    std::string Publisher;
	int NoOfCopies;
	std::time_t DateCreated;
	std::time_t DateUpdated;
	BookStatus Status;
    std::vector<CategoryDto> Categories;
};

class Books
{
public:
	Books();
	Books(const std::string& filename);
	~Books();

	bool AddBook(BooksDto book);
	bool AddBookCopies(int bookId, int copies);
	bool RemoveBook(int bookId);
	bool RemoveBookCopies(int bookId, int copies);
	std::vector<BooksDto> GetAllBooks();
	BooksDto GetBooksById(int id);

    struct SearchResult {
        BooksDto book;
        double score;
        
        bool operator<(const SearchResult& other) const {
            return score > other.score;
        }
    };
    std::vector<SearchResult> SearchBooks(const std::string& query, size_t limit = 10) const;

private:
	std::string filename;
	void SaveToFile(const std::vector<BooksDto>& books) const;
	std::vector<BooksDto> LoadFromFile() const;
    int GetNextBookId() const;
    double CalculateNGramSimilarity(const std::string& s1, const std::string& s2, int n = 2) const;
    std::string GetSoundex(const std::string& word) const;
    std::vector<std::string> Tokenize(const std::string& text) const;
    double CalculateSearchScore(const BooksDto& book, const std::string& query) const;
};

#endif