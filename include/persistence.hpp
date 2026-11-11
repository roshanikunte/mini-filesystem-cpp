#ifndef PERSISTENCE_HPP
#define PERSISTENCE_HPP

#include "filesystem.hpp"
#include <string>

class Persistence {
public:
    static bool save(FileSystem& fs, const std::string& filename);
    static bool load(FileSystem& fs, const std::string& filename);
};

#endif
