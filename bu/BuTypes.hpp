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

#include <stdexcept>

#include <pdal/util/Bounds.hpp>

#include "../common/FileDimInfo.hpp"

namespace ept2
{
namespace bu
{

struct Error : public std::runtime_error
{
    Error(const std::string& err) : std::runtime_error(err)
    {}
};

enum class Status
{
    Sampled,
    Continue
};

struct BaseInfo
{
    pdal::BOX3D bounds;
    size_t pointSize;
    std::string inputDir;
    std::string outputDir;
    int maxLevel;
    DimInfoList dimInfo;
};

} // namespace bu
} // namespace ept2
