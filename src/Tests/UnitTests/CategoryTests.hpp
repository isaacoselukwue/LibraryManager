#ifndef CATEGORY_TESTS_HPP
#define CATEGORY_TESTS_HPP

#include <cassert>
#include <filesystem>
#include "../../Interfaces/Categories.hpp"

namespace fs = std::filesystem;

class CategoryTests {
private:
    const std::string TEST_DIR = "./resources/test/database";
    const std::string TEST_FILE = TEST_DIR + "/test_categories.json";

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

    void TestAddCategory() {
        Categories categories(TEST_FILE);
        CategoryDto category{};
        category.CategoryId = 1;
        category.Name = "Test Category";
        category.Description = "Test Description";
        category.DateCreated = std::time(nullptr);
        
        std::cout << "Adding category with ID: " << category.CategoryId << std::endl;
        bool result = categories.AddCategory(category);
        assert(result && "Add category failed");
        
        auto savedCategory = categories.GetCategoryById(1);
        assert(savedCategory.Name == "Test Category" && "Category data mismatch");
    }

    void TestGetCategory() {
        Categories categories(TEST_FILE);
        auto category = categories.GetCategoryById(1);
        assert(category.CategoryId == 1 && "Get category failed");
    }

    void TestRemoveCategory() {
        Categories categories(TEST_FILE);
        bool result = categories.RemoveCategory(1);
        assert(result && "Remove category failed");
    }

public:
    void RunAllTests() {
        try {
            SetUp();
            std::cout << "Running category tests...\n";
            TestAddCategory();
            TestGetCategory();
            // TestRemoveCategory();
            // TearDown();
            std::cout << "All category tests passed!\n";
        }
        catch (const std::exception& e) {
            std::cerr << "Test failed: " << e.what() << std::endl;
            TearDown();
            throw;
        }
    }
};

#endif