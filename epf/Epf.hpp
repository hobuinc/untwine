/*****************************************************************************
 *   Copyright (c) 2020, Hobu, Inc. (info@hobu.co)                           *
 *                                                                           *
 *   All rights reserved.                                                    *
 *                                                                           *
 *   This program is free software; you can redistribute it and/or modify    *
 *   it under the terms of the GNU General Public License as published by    *
 *   the Free Software Foundation; either version 3 of the License, or       *
 *   (at your option) any later version.                                     *
 *                                                                           *
 ****************************************************************************/


#pragma once

#include <map>
#include <vector>

#include <pdal/SpatialReference.hpp>
#include <pdal/util/ThreadPool.hpp>

#include "EpfTypes.hpp"
#include "Grid.hpp"

namespace pdal
{
    class ProgramArgs;
}

namespace ept2
{
namespace epf
{

std::string indexToString(int index);
int toIndex(int x, int y, int z);

struct FileInfo;
class Writer;

class Epf
{
public:
    Epf();

    void run(const std::vector<std::string>& options);

private:
    void addArgs(pdal::ProgramArgs& programArgs);
    void createFileInfo(std::vector<FileInfo>& fileInfos);

    std::vector<std::string> m_files;
    std::string m_outputDir;
    Grid m_grid;
    std::unique_ptr<Writer> m_writer;
    pdal::ThreadPool m_pool;
    size_t m_fileLimit;
    int m_level;
    bool m_doCube;
    FileInfo *m_srsFileInfo;
};

} // namespace epf
} // namespace ept2
