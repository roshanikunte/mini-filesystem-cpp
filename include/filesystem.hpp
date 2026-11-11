#ifndef FILESYSTEM_HPP
#define FILESYSTEM_HPP

#include <iostream>
#include <string>
#include <map>
#include <memory>
#include <ctime>
#include <vector>

using namespace std;

struct FileEntry {
    string name;
    string content;
    time_t created_at;
    time_t modified_at;

    FileEntry(const string& filename);
};

struct Directory {
    string name;
    Directory* parent;

    map<string, unique_ptr<Directory>> directories;
    map<string, FileEntry> files;

    time_t created_at;

    Directory(const string& dirname, Directory* parent_dir = nullptr);
};

class FileSystem {
private:
    unique_ptr<Directory> root;

    Directory* resolveDirectory(const string& path);

public:
    FileSystem();

    Directory* getRoot() {
        return root.get();
    }

    void clear() {
        root = make_unique<Directory>("/");
    }
    bool mkdir(const string& path, const string& name);
    bool cd(const string& path, string& current_path);
    bool touch(const string& path, const string& filename);
    bool rm(const string& path, const string& name, bool recursive);

    string ls(const string& path, bool detailed = false);
    void pwd(const string& current_path);
};

#endif
