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

#include <pdal/util/Bounds.hpp>
#include "../common/VoxelKey.hpp"

namespace ept2
{
namespace epf
{

class Grid
{
public:
    Grid() : m_gridSize(-1), m_maxLevel(-1), m_millionPoints(0)
    {}

    void expand(const pdal::BOX3D& bounds, size_t points);
    VoxelKey key(double x, double y, double z);
    pdal::BOX3D bounds() const
        { return m_bounds; }
    int maxLevel() const
        { return m_maxLevel; }
    void resetLevel(int level);

private:
    int m_gridSize;
    int m_maxLevel;
    pdal::BOX3D m_bounds;
    size_t m_millionPoints;
    double m_xsize;
    double m_ysize;
    double m_zsize;
};

} // namespace epf
} // namespace ept2
