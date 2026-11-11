#include "persistence.hpp"

#include <fcntl.h>
#include <unistd.h>
#include <cstdint>
#include <cerrno>
#include <cstdio>
#include <memory>
#include <string>

// Write all bytes safely
bool writeAll(int fd, const void* data, size_t size) {
    const char* ptr = static_cast<const char*>(data);

    while (size > 0) {
        ssize_t written = write(fd, ptr, size);

        if (written < 0 && errno == EINTR)
            continue;

        if (written <= 0)
            return false;

        ptr += written;
        size -= static_cast<size_t>(written);
    }

    return true;
}

// Read all bytes safely
bool readAll(int fd, void* data, size_t size) {
    char* ptr = static_cast<char*>(data);

    while (size > 0) {
        ssize_t received = read(fd, ptr, size);

        if (received < 0 && errno == EINTR)
            continue;

        if (received <= 0)
            return false;

        ptr += received;
        size -= static_cast<size_t>(received);
    }

    return true;
}

// Save a string
bool saveString(int fd, const string& value) {
    uint64_t length = value.size();

    return writeAll(fd, &length, sizeof(length)) &&
           writeAll(fd, value.data(), length);
}

// Load a string
bool loadString(int fd, string& value) {
    uint64_t length = 0;

    if (!readAll(fd, &length, sizeof(length)))
        return false;

    if (length > 10000000)
        return false;

    value.resize(static_cast<size_t>(length));

    return readAll(fd, value.data(), value.size());
}

// Save directory recursively
bool saveDirectory(int fd, Directory* dir) {
    int64_t created = dir->created_at;

    uint64_t dirCount = dir->directories.size();
    uint64_t fileCount = dir->files.size();

    if (!writeAll(fd, &created, sizeof(created)))
        return false;

    if (!writeAll(fd, &dirCount, sizeof(dirCount)))
        return false;

    for (auto& entry : dir->directories) {
        if (!saveString(fd, entry.first))
            return false;

        if (!saveDirectory(fd, entry.second.get()))
            return false;
    }

    if (!writeAll(fd, &fileCount, sizeof(fileCount)))
        return false;

    for (auto& entry : dir->files) {
        FileEntry& file = entry.second;

        int64_t createdTime = file.created_at;
        int64_t modifiedTime = file.modified_at;

        if (!saveString(fd, file.name))
            return false;

        if (!saveString(fd, file.content))
            return false;

        if (!writeAll(fd, &createdTime, sizeof(createdTime)))
            return false;

        if (!writeAll(fd, &modifiedTime, sizeof(modifiedTime)))
            return false;
    }

    return true;
}

// Load directory recursively
bool loadDirectory(int fd, Directory* dir, int depth = 0) {
    if (depth > 100)
        return false;

    int64_t created;
    uint64_t dirCount;
    uint64_t fileCount;

    if (!readAll(fd, &created, sizeof(created)))
        return false;

    dir->created_at = static_cast<time_t>(created);

    if (!readAll(fd, &dirCount, sizeof(dirCount)))
        return false;

    if (dirCount > 100000)
        return false;

    for (uint64_t i = 0; i < dirCount; i++) {
        string name;

        if (!loadString(fd, name))
            return false;

        if (name.empty() || name == "." ||
            name == ".." || name.find('/') != string::npos)
            return false;

        if (dir->directories.count(name) ||
            dir->files.count(name))
            return false;

        auto child = make_unique<Directory>(name, dir);
        Directory* childPtr = child.get();

        dir->directories[name] = move(child);

        if (!loadDirectory(fd, childPtr, depth + 1))
            return false;
    }

    if (!readAll(fd, &fileCount, sizeof(fileCount)))
        return false;

    if (fileCount > 100000)
        return false;

    for (uint64_t i = 0; i < fileCount; i++) {
        string name;
        string content;

        int64_t createdTime;
        int64_t modifiedTime;

        if (!loadString(fd, name))
            return false;

        if (!loadString(fd, content))
            return false;

        if (!readAll(fd, &createdTime, sizeof(createdTime)))
            return false;

        if (!readAll(fd, &modifiedTime, sizeof(modifiedTime)))
            return false;

        if (name.empty() || name == "." ||
            name == ".." || name.find('/') != string::npos)
            return false;

        if (dir->files.count(name) ||
            dir->directories.count(name))
            return false;

        FileEntry file(name);

        file.content = content;
        file.created_at = static_cast<time_t>(createdTime);
        file.modified_at = static_cast<time_t>(modifiedTime);

        dir->files.insert_or_assign(name, file);
    }

    return true;
}

// Save complete filesystem safely
bool Persistence::save(FileSystem& fs, const string& filename) {
    string tempFile = filename + ".tmp";

    int fd = open(
        tempFile.c_str(),
        O_WRONLY | O_CREAT | O_TRUNC,
        0644
    );

    if (fd < 0)
        return false;

    bool success = saveDirectory(fd, fs.getRoot());

    if (success && fsync(fd) < 0)
        success = false;

    if (close(fd) < 0)
        success = false;

    if (!success) {
        unlink(tempFile.c_str());
        return false;
    }

    if (rename(tempFile.c_str(), filename.c_str()) < 0) {
        unlink(tempFile.c_str());
        return false;
    }

    return true;
}

// Restore complete filesystem safely
bool Persistence::load(FileSystem& fs, const string& filename) {
    int fd = open(filename.c_str(), O_RDONLY);

    if (fd < 0)
        return false;

    FileSystem temporaryFS;

    bool success = loadDirectory(fd, temporaryFS.getRoot());

    if (close(fd) < 0)
        success = false;

    if (!success)
        return false;

    fs = move(temporaryFS);

    return true;
}
