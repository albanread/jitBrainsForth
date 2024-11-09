
#ifndef STRINGINTERNER_H
#define STRINGINTERNER_H
#include <iostream>
#include <unordered_map>
#include <vector>
#include <memory>
#include <cstring>
#include <mutex>

class StringInterner {
public:
    // Returns the singleton instance of StringInterner.
    static StringInterner& getInstance() {
        static StringInterner instance;
        return instance;
    }

    // Deletes the copy constructor and assignment operator to prevent copying.
    StringInterner(const StringInterner&) = delete;
    StringInterner& operator=(const StringInterner&) = delete;

    // Interns a string and returns a pointer to the interned string.
    const char* intern(const std::string& str) {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = intern_map.find(str);
        if (it != intern_map.end()) {
            return it->second.get();
        }

        // Create a new interned string
        std::unique_ptr<char[]> internedStr(new char[str.size() + 1]);
        std::strcpy(internedStr.get(), str.c_str());

        const char* internedPtr = internedStr.get();
        intern_map[str] = std::move(internedStr);

        interned_strings.push_back(internedPtr);

        return internedPtr;
    }

    // Lists all interned strings.
    std::vector<const char*> list() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return interned_strings;
    }

    // display list of strings and their memory addresses
    void display_list() const {
        std::lock_guard<std::mutex> lock(mutex_);
        for (const auto& str : interned_strings) {
            std::cout << str << " : " << str << std::endl;
        }
    }


private:
    StringInterner() = default; // Private constructor for singleton.

    // Private members for holding the interned strings and mutex for thread safety.
    std::unordered_map<std::string, std::unique_ptr<char[]>> intern_map;
    std::vector<const char*> interned_strings;
    mutable std::mutex mutex_; // Mutable to allow locking in const functions.
};


#endif //STRINGINTERNER_H
