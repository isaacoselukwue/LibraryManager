#ifndef BOOKS_HPP
#define BOOKS_HPP

#include <iostream>
#include <fstream>
#include <string>
#include <vector>

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

private:
	std::string filename;
	void SaveToFile(const std::vector<BooksDto>& books) const;
	std::vector<BooksDto> LoadFromFile() const;
    int GetNextBookId() const;
};

#endif