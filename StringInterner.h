#ifndef STRINGINTERNER_H
#define STRINGINTERNER_H

#include <iostream>
#include <unordered_map>
#include <vector>
#include <memory>
#include <cstring>
#include <mutex>

class StringStorage {
public:
    // Adds a string and returns the index
    size_t addString(const std::string& str) {
        strings.push_back(str);
        return strings.size() - 1;
    }

    // Retrieves a string by its index
    std::string getString(size_t index) const {
        if (index >= strings.size()) {
            throw std::out_of_range("Index out of range");
        }
        return strings[index];
    }

    [[nodiscard]] void* getStringAddress(const size_t index) const {

        if (index >= strings.size()) {
            throw std::out_of_range("Index out of range");
        }

        const auto address = (void*)strings[index].c_str();
        return address;

    }


    // Clears all stored strings
    void clearStrings() {
        strings.clear();
    }

private:
    std::vector<std::string> strings;
};


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

    // Interns a string and returns its index.
    size_t intern(const std::string& str) {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = intern_map.find(str);
        if (it != intern_map.end()) {
            ref_counts[it->second]++;
            return it->second;
        }

        // Add the string to storage and get the index
        size_t index = storage.addString(str);
        intern_map[str] = index;
        ref_counts[index] = 1;

        return index;
    }

    // Retrieves a string by its index.
    std::string getString(size_t index) const {
        std::lock_guard<std::mutex> lock(mutex_);
        return storage.getString(index);
    }

    void* getStringAddress(size_t index) const {
        std::lock_guard<std::mutex> lock(mutex_);
        return storage.getStringAddress(index);
    }

    // Decreases the reference count of the string by its index and removes it if the count reaches zero.
    void release(size_t index) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (ref_counts.find(index) != ref_counts.end()) {
            ref_counts[index]--;
            if (ref_counts[index] == 0) {
                // Remove the string from the intern map
                auto it = std::find_if(intern_map.begin(), intern_map.end(),
                    [index](const auto& pair) { return pair.second == index; });
                if (it != intern_map.end()) {
                    intern_map.erase(it);
                }
            }
        }
    }

    // Lists all interned strings along with their reference counts.
    std::vector<std::pair<std::string, size_t>> list() const {
        std::lock_guard<std::mutex> lock(mutex_);
        std::vector<std::pair<std::string, size_t>> list;
        for (const auto& entry : intern_map) {
            list.emplace_back(entry.first, ref_counts.at(entry.second));
        }
        return list;
    }

    // Displays list of strings, their indices, and reference counts.
    void display_list() const {
        std::lock_guard<std::mutex> lock(mutex_);
        for (const auto& entry : intern_map) {
            std::cout << entry.first << " : " << entry.second 
                      << " (ref count: " << ref_counts.at(entry.second) << ")" << std::endl;
        }
    }

private:
    StringInterner() = default; // Private constructor for singleton.

    // Private members for holding the interned strings and mutex for thread safety.
    std::unordered_map<std::string, size_t> intern_map;
    std::unordered_map<size_t, size_t> ref_counts; // Store ref counts by index
    mutable std::mutex mutex_; // Mutable to allow locking in const functions.
    StringStorage storage; // Storage for the interned strings
};

#endif // STRINGINTERNER_H