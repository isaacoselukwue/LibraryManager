#include <iostream>

#include "../Interfaces/Books.hpp"
#include "../Interfaces/Categories.hpp"
#include "../Interfaces/Users.hpp"
#include "../Tests/UnitTests/BookTests.hpp"
#include "../Tests/UnitTests/CategoryTests.hpp"
#include "../Tests/UnitTests/UserTests.hpp"

void RunUnitTests() {
    BookTests bookTests;
    CategoryTests categoryTests;
    UserTests userTests;
    
    std::cout << "Running Unit Tests...\n\n";

    std::cout << "\nCategory Tests:\n";
    categoryTests.RunAllTests();
    
    std::cout << "\nBook Tests:\n";
    bookTests.RunAllTests();

    std::cout << "\nUser Tests:\n";
    userTests.RunAllTests();
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