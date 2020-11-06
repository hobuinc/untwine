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

#include <string>
#include <unordered_map>
#include <vector>

#include "PyramidManager.hpp"
#include "BuTypes.hpp"

namespace pdal
{
    class ProgramArgs;
}

namespace ept2
{
namespace bu
{

class FileInfo;

class BuPyramid
{
public:
    BuPyramid();
    void run(const std::vector<std::string>& options);

private:
    void addArgs(pdal::ProgramArgs& programArgs);
    void getInputFiles();
    void readBaseInfo();
    void queueWork();
    void writeInfo();
    void createDirs();

    PyramidManager m_manager;
    BaseInfo m_b;
    std::unordered_map<VoxelKey, FileInfo> m_allFiles;
};

} // namespace bu
} // namespace ept2
