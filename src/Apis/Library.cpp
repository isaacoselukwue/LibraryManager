#include <iostream>

#include "../Interfaces/Books.hpp"
#include "../Interfaces/Categories.hpp"
#include "../Tests/UnitTests/BookTests.hpp"
#include "../Tests/UnitTests/CategoryTests.hpp"

void RunUnitTests() {
    BookTests bookTests;
    CategoryTests categoryTests;
    
    std::cout << "Running Unit Tests...\n\n";

    std::cout << "\nCategory Tests:\n";
    categoryTests.RunAllTests();
    
    std::cout << "Book Tests:\n";
    bookTests.RunAllTests();
}

int main(int argc, char* argv[])
{
    if (argc > 1 && std::string(argv[1]) == "1") {
        RunUnitTests();
        return 0;
    }

    std::cout << "Library Management System\n";
    // Normal program logic here
    return 0;
}