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

#include <pdal/Dimension.hpp>
#include <pdal/SpatialReference.hpp>
#include <pdal/util/Bounds.hpp>

#include <cstdint>
#include <memory>
#include <unordered_map>
#include <vector>

#include "../common/FileDimInfo.hpp"
#include "../common/VoxelKey.hpp"

namespace ept2
{
namespace epf
{

using DataVec = std::vector<uint8_t>;
using DataVecPtr = std::unique_ptr<DataVec>;
using Totals = std::unordered_map<VoxelKey, size_t>;
constexpr int MaxPointsPerNode = 100000;

struct Error : public std::runtime_error
{
    Error(const std::string& err) : std::runtime_error(err)
    {}
};

struct FileInfo
{
    std::string filename;
    std::string driver;
    DimInfoList dimInfo;
    uint64_t numPoints;
    pdal::BOX3D bounds;
    pdal::SpatialReference srs;

    bool valid() const
    { return filename.size(); }
};

} // namespace epf
} // namespace ept2
