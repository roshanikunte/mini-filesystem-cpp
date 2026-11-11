#include "filesystem.hpp"
#include <sstream>
#include <iomanip>
#include <iostream>
#include <vector>

using namespace std;

// Initialize a file
FileEntry::FileEntry(const string& filename) {
    name = filename;
    content = "";
    created_at = time(nullptr);
    modified_at = created_at;
}

// Initialize a directory
Directory::Directory(const string& dirname, Directory* parent_dir) {
    name = dirname;
    parent = parent_dir;
    created_at = time(nullptr);
}

// Initialize filesystem
FileSystem::FileSystem() {
    root = make_unique<Directory>("/");
}

// Resolve an absolute directory path
Directory* FileSystem::resolveDirectory(const string& path) {
    Directory* current = root.get();

    if (path.empty() || path == "/") {
        return current;
    }

    stringstream ss(path);
    string component;

    while (getline(ss, component, '/')) {
        if (component.empty() || component == ".") {
            continue;
        }

        if (component == "..") {
            if (current->parent != nullptr) {
                current = current->parent;
            }
            continue;
        }

        auto it = current->directories.find(component);

        if (it == current->directories.end()) {
            return nullptr;
        }

        current = it->second.get();
    }

    return current;
}

// Create directory
bool FileSystem::mkdir(const string& path, const string& name) {
    Directory* parent = resolveDirectory(path);

    if (parent == nullptr || name.empty() ||
        name == "." || name == ".." ||
        name.find('/') != string::npos) {
        return false;
    }

    if (parent->directories.count(name) ||
        parent->files.count(name)) {
        return false;
    }

    parent->directories[name] =
        make_unique<Directory>(name, parent);

    return true;
}

// Create file
bool FileSystem::touch(const string& path, const string& filename) {
    Directory* parent = resolveDirectory(path);

    if (parent == nullptr || filename.empty() ||
        filename == "." || filename == ".." ||
        filename.find('/') != string::npos) {
        return false;
    }

    if (parent->directories.count(filename)) {
        return false;
    }

    auto it = parent->files.find(filename);

    if (it == parent->files.end()) {
        parent->files.emplace(filename, FileEntry(filename));
    } else {
        it->second.modified_at = time(nullptr);
    }

    return true;
}

// Change directory
bool FileSystem::cd(const string& path, string& current_path) {
    Directory* target = resolveDirectory(path);

    if (target == nullptr) {
        return false;
    }

    if (target == root.get()) {
        current_path = "/";
        return true;
    }

    vector<string> components;
    Directory* current = target;

    while (current != root.get()) {
        components.push_back(current->name);
        current = current->parent;
    }

    current_path = "";

    for (auto it = components.rbegin();
         it != components.rend(); ++it) {
        current_path += "/" + *it;
    }

    return true;
}

// List directory contents
string FileSystem::ls(const string& path, bool detailed) {
    Directory* directory = resolveDirectory(path);

    if (directory == nullptr) {
        return "Directory not found\n";
    }

    string result;

    for (const auto& entry : directory->directories) {
        if (detailed) {
            result += "DIR  " + entry.first + "/\n";
        } else {
            result += entry.first + "/\n";
        }
    }

    for (const auto& entry : directory->files) {
        if (detailed) {
            result += "FILE " + entry.first +
                      "  Size: " +
                      to_string(entry.second.content.size()) +
                      " bytes\n";
        } else {
            result += entry.first + "\n";
        }
    }

    if (result.empty()) {
        result = "Directory is empty\n";
    }

    return result;
}

// Remove file or directory
bool FileSystem::rm(const string& path, const string& name,
                    bool recursive) {
    Directory* parent = resolveDirectory(path);

    if (parent == nullptr || name.empty() ||
        name == "." || name == ".." ||
        name.find('/') != string::npos) {
        return false;
    }

    auto file = parent->files.find(name);

    if (file != parent->files.end()) {
        parent->files.erase(file);
        return true;
    }

    auto directory = parent->directories.find(name);

    if (directory == parent->directories.end()) {
        return false;
    }

    if (!recursive &&
        (!directory->second->directories.empty() ||
         !directory->second->files.empty())) {
        return false;
    }

    parent->directories.erase(directory);
    return true;
}

// Display current directory
void FileSystem::pwd(const string& current_path) {
    cout << current_path << endl;
}
