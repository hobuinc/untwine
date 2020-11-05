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


#include <mutex>

#include "BufferCache.hpp"

namespace ept2
{
namespace epf
{

// If we have a buffer in the cache, return it. Otherwise create a new one and return that.
DataVecPtr BufferCache::fetch()
{
    std::lock_guard<std::mutex> lock(m_mutex);

    if (m_buffers.size())
    {
        DataVecPtr buf(std::move(m_buffers.back()));
        m_buffers.pop_back();
        return buf;
    }

    constexpr size_t BufSize = 4096 * 10;
    return DataVecPtr(new DataVec(BufSize));
}

// Put a buffer back in the cache.
void BufferCache::replace(DataVecPtr&& buf)
{
    std::lock_guard<std::mutex> lock(m_mutex);

    m_buffers.push_back(std::move(buf));
}

} // namespace epf
} // namespace ept2
