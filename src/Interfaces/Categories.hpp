#ifndef CATEGORIES_HPP
#define CATEGORIES_HPP

#include <string>
#include <vector>

#include "Common.hpp"

using categoryDto = struct CategoryDto
{
    int CategoryId;
    std::string Name;
    std::string Description;
    std::time_t DateCreated;
};

class Categories
{
public:
    Categories();
    Categories(const std::string& filename);
    ~Categories();

    bool AddCategory(CategoryDto category);
    bool RemoveCategory(int categoryId);
    std::vector<CategoryDto> GetAllCategories();
    CategoryDto GetCategoryById(int id);

private:
    std::string filename;
    void SaveToFile(const std::vector<CategoryDto>& categories) const;
    std::vector<CategoryDto> LoadFromFile() const;
};

#endif