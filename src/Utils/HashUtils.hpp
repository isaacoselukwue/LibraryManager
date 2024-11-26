#ifndef HASH_UTILS_HPP
#define HASH_UTILS_HPP

#include <string>
#include <functional>
#include <iomanip>
#include <sstream>

namespace Utils {
    inline std::string CreateSaltedHash(const std::string& email, const std::string& password) {
        std::hash<std::string> hasher;
        std::string saltedPassword = email + "|" + password;
        auto hashValue = hasher(saltedPassword);
        std::stringstream ss;
        ss << std::hex << std::setw(16) << std::setfill('0') << hashValue;
        return ss.str();
    }
}

#endif