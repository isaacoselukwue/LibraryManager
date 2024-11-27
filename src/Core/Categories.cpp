#include <nlohmann/json.hpp>
#include <filesystem>
#include <fstream>
#include <sys/file.h>
#include <fcntl.h>
#include <unistd.h>

#include "../Interfaces/Categories.hpp"

using json = nlohmann::json;
namespace fs = std::filesystem;

Categories::Categories() {
    filename = "./resources/database/categories.json";
    fs::create_directories("./resources/database");
    if (!fs::exists(filename)) {
        std::ofstream file(filename);
        file << "[]";
        file.close();
    }
}

Categories::Categories(const std::string& fname) : filename(fname) {}

Categories::~Categories() {}

int Categories::GetNextCategoryId() const {
    auto categories = LoadFromFile();
    if (categories.empty()) return 1;
    
    return std::max_element(categories.begin(), categories.end(),
        [](const CategoryDto& a, const CategoryDto& b) { 
            return a.CategoryId < b.CategoryId; 
        })->CategoryId + 1;
}

bool Categories::AddCategory(CategoryDto category) {
    try {
        std::cout << "Attempting to open file: " << filename << std::endl;
        
        int fd = open(filename.c_str(), O_RDWR);
        if (fd == -1) {
            std::cerr << "Failed to open file. Error: " << strerror(errno) << std::endl;
            return false;
        }
        
        std::cout << "File opened successfully. Attempting to lock..." << std::endl;
        
        if (flock(fd, LOCK_EX) == -1) {
            std::cerr << "Failed to lock file. Error: " << strerror(errno) << std::endl;
            close(fd);
            return false;
        }
        
        std::cout << "File locked. Loading existing categories..." << std::endl;
        auto categories = LoadFromFile();
        category.CategoryId = GetNextCategoryId();
        std::cout << "Adding new category..." << std::endl;
        categories.push_back(category);
        
        std::cout << "Saving updated categories..." << std::endl;
        SaveToFile(categories);
        
        std::cout << "Unlocking and closing file..." << std::endl;
        flock(fd, LOCK_UN);
        close(fd);
        
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Exception in AddCategory: " << e.what() << std::endl;
        return false;
    } catch (...) {
        std::cerr << "Unknown exception in AddCategory" << std::endl;
        return false;
    }
}

bool Categories::RemoveCategory(int categoryId) {
    try {
        int fd = open(filename.c_str(), O_RDWR);
        if (fd == -1) return false;
        
        flock(fd, LOCK_EX);
        auto categories = LoadFromFile();
        
        auto it = std::remove_if(categories.begin(), categories.end(),
            [categoryId](const CategoryDto& cat) { return cat.CategoryId == categoryId; });
        
        if (it != categories.end()) {
            categories.erase(it, categories.end());
            SaveToFile(categories);
        }
        
        flock(fd, LOCK_UN);
        close(fd);
        return true;
    } catch (...) {
        return false;
    }
}

std::vector<CategoryDto> Categories::GetAllCategories() {
    int fd = open(filename.c_str(), O_RDONLY);
    if (fd == -1) return {};
    
    flock(fd, LOCK_SH);
    auto categories = LoadFromFile();
    flock(fd, LOCK_UN);
    close(fd);
    return categories;
}

CategoryDto Categories::GetCategoryById(int id) {
    auto categories = GetAllCategories();
    auto it = std::find_if(categories.begin(), categories.end(),
        [id](const CategoryDto& cat) { return cat.CategoryId == id; });
    return it != categories.end() ? *it : CategoryDto{};
}

void Categories::SaveToFile(const std::vector<CategoryDto>& categories) const {
    json j = json::array();
    for (const auto& category : categories) {
        json categoryJson;
        categoryJson["CategoryId"] = category.CategoryId;
        categoryJson["Name"] = category.Name;
        categoryJson["Description"] = category.Description;
        categoryJson["DateCreated"] = category.DateCreated;
        j.push_back(categoryJson);
    }
    
    std::ofstream file(filename);
    file << std::setw(4) << j << std::endl;
}

std::vector<CategoryDto> Categories::LoadFromFile() const {
    std::vector<CategoryDto> categories;
    std::ifstream file(filename);
    if (!file.is_open()) return categories;
    
    json j;
    file >> j;
    
    for (const auto& categoryJson : j) {
        CategoryDto category;
        category.CategoryId = categoryJson["CategoryId"];
        category.Name = categoryJson["Name"];
        category.Description = categoryJson["Description"];
        category.DateCreated = categoryJson["DateCreated"];
        categories.push_back(category);
    }
    
    return categories;
}