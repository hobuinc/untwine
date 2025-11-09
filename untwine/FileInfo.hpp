/*****************************************************************************
 *   Copyright (c) 2024, Hobu, Inc. (info@hobu.co)                           *
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

#include "Common.hpp"

#include <pdal/SpatialReference.hpp>
#include <pdal/util/Bounds.hpp>

#include "FileDimInfo.hpp"

namespace untwine
{

struct FileInfo
{
    FileInfo()
    {}

    std::string filename;
    std::string driver;
    bool no_srs {true};
    DimInfoList dimInfo;
    uint64_t numPoints {0};
    uint64_t start {0};
    pdal::BOX3D bounds;
    pdal::SpatialReference srs;
    int untwineBitsOffset {-1};
    // Currently only set for LAS files.
    int fileVersion {0};
    Transform xform;

    bool valid() const
    { return filename.size(); }
};

} // namespace untwine
