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


#include "EpfTypes.hpp"
#include "Grid.hpp"
#include "Cell.hpp"

namespace ept2
{
namespace epf
{

class Writer;

// Processes a single input file (FileInfo) and writes data to the Writer.
class FileProcessor
{
public:
    FileProcessor(const FileInfo& fi, size_t pointSize, const Grid& grid, Writer *writer);

    Cell *getCell(const VoxelKey& key);
    void operator()();

private:
    FileInfo m_fi;
    CellMgr m_cellMgr;
    Grid m_grid;
    int m_cnt;

    static int m_totalCnt;
};

} // namespace epf
} // namespace ept2
