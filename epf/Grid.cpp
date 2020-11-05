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


#include <cmath>
#include <cstdint>

#include "Epf.hpp"
#include "Grid.hpp"

using namespace pdal;

namespace ept2
{
namespace epf
{

void Grid::expand(const BOX3D& bounds, size_t points)
{
    m_bounds.grow(bounds);
    m_millionPoints += (points / 1000000.0);

    //ABELL - Fix this for small point clouds.
    if (m_millionPoints >= 2000)
        resetLevel(6);
    else if (m_millionPoints >= 100)
        resetLevel(5);
    else
        resetLevel(4);
}

void Grid::resetLevel(int level)
{
    m_maxLevel = level;
    m_gridSize = std::pow(2, level);

    m_xsize = (m_bounds.maxx - m_bounds.minx) / m_gridSize;
    m_ysize = (m_bounds.maxy - m_bounds.miny) / m_gridSize;
    m_zsize = (m_bounds.maxz - m_bounds.minz) / m_gridSize;
}

VoxelKey Grid::key(double x, double y, double z)
{
    int xi = std::floor((x - m_bounds.minx) / m_xsize);
    int yi = std::floor((y - m_bounds.miny) / m_ysize);
    int zi = std::floor((z - m_bounds.minz) / m_zsize);
    xi = (std::min)((std::max)(0, xi), m_gridSize - 1);
    yi = (std::min)((std::max)(0, yi), m_gridSize - 1);
    zi = (std::min)((std::max)(0, zi), m_gridSize - 1);

    return VoxelKey(xi, yi, zi, m_maxLevel);
}

} // namespace epf
} // namespace ept2
