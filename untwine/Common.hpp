#pragma once

#include <string>

namespace untwine
{

struct Error : public std::runtime_error
{
    Error(const std::string& err) : std::runtime_error(err)
    {}
};

struct Options
{
    std::string outputDir;
    pdal::StringList inputFiles;
    std::string tempDir;
    bool doCube;
    size_t fileLimit;
    int level;
};

const std::string MetadataFilename {"info2.txt"};

} // namespace untwine
