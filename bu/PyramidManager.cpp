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


#include <regex>
#include <string>
#include <vector>

#include <pdal/util/FileUtils.hpp>

#include "../common/VoxelKey.hpp"

#include "Processor.hpp"
#include "PyramidManager.hpp"
#include "VoxelInfo.hpp"

namespace ept2
{
namespace bu
{

PyramidManager::PyramidManager(const BaseInfo& b) : m_b(b), m_pool(6), m_totalPoints(0)
{}


PyramidManager::~PyramidManager()
{}


void PyramidManager::queue(const OctantInfo& o)
{
    {
        std::lock_guard<std::mutex> lock(m_mutex);

        m_queue.push(o);
    }
    m_cv.notify_one();
}

void PyramidManager::run()
{
    while (true)
    {
        OctantInfo o;
        {
            std::unique_lock<std::mutex> lock(m_mutex);

            m_cv.wait(lock, [this](){return m_queue.size();});
            o = m_queue.front();
            m_queue.pop();
        }

        if (o.key() == VoxelKey(0, 0, 0, 0))
            break;
        process(o);
    }
    createHierarchy();
}


void PyramidManager::process(const OctantInfo& o)
{
    VoxelKey pk = o.key().parent();
    addComplete(o);
    if (!childrenComplete(pk))
        return;

    VoxelInfo vi(m_b.bounds, pk);
    for (int i = 0; i < 8; ++i)
        vi[i] = removeComplete(pk.child(i));

    // If there are no points in this voxel, just queue it as a child.
    if (!vi.hasPoints())
        queue(vi.octant());
    else
    {
        std::unique_ptr<Processor> p(new Processor(*this, vi, m_b));
        std::function<void()> f = std::bind(&Processor::operator(), p.get());
        m_pool.add(f);
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            m_processors.push_back(std::move(p));
        }
    }
}


void PyramidManager::destroy(Processor *p)
{
    std::lock_guard<std::mutex> lock(m_mutex);

    for (auto pi = m_processors.begin(); pi != m_processors.end(); ++pi)
        if (pi->get() == p)
        {
            m_processors.erase(pi);
            break;
        }
}


void PyramidManager::addComplete(const OctantInfo& o)
{
    m_completes.insert({o.key(), o});
}


bool PyramidManager::childrenComplete(const VoxelKey& parent)
{
    for (int i = 0; i < 8; ++i)
        if (m_completes.find(parent.child(i)) == m_completes.end())
            return false;
    return true;
}


OctantInfo PyramidManager::removeComplete(const VoxelKey& k)
{
    OctantInfo o;

    auto oi = m_completes.find(k);
    if (oi != m_completes.end())
    {
        o = std::move(oi->second);
        m_completes.erase(oi);
    }
    return o;
}


void PyramidManager::logOctant(const VoxelKey& k, int cnt)
{
    std::lock_guard<std::mutex> lock(m_mutex);

    m_written.insert({k, cnt});
    m_totalPoints += cnt;
}


void PyramidManager::createHierarchy()
{
    std::function<int(const VoxelKey&)> calcCounts;
    calcCounts = [this, &calcCounts](const VoxelKey& k)
    {
        int count = 0;
        for (int i = 0; i < 8; ++i)
        {
            VoxelKey c = k.child(i);
            if (m_written.find(c) != m_written.end())
                count += calcCounts(c);
        }
        m_childCounts[k] = count;
        return count + 1;
    };

    calcCounts(VoxelKey(0, 0, 0, 0));

    std::deque<VoxelKey> roots;

    roots.push_back(VoxelKey(0, 0, 0, 0));
    while (roots.size())
    {
        VoxelKey k = roots.front();
        roots.pop_front();
        auto newRoots = emitRoot(k);
        roots.insert(roots.end(), newRoots.begin(), newRoots.end());
    }
}

std::deque<VoxelKey> PyramidManager::emitRoot(const VoxelKey& root)
{
    int level = root.level();
    int stopLevel = level + LevelBreak;

    Entries entries;
    entries.push_back({root, m_written[root]});
    std::deque<VoxelKey> roots = emit(root, stopLevel, entries);

    std::ofstream out(m_b.outputDir + "/ept-hierarchy/" + root.toString() + ".json");

    out << "{\n";

    for (auto it = entries.begin(); it != entries.end(); ++it)
    {
        if (it != entries.begin())
            out << ",\n";
        out << "\"" << it->first << "\": " << it->second;
    }
    out << "\n";

    out << "}\n";

    return roots;
}


std::deque<VoxelKey> PyramidManager::emit(const VoxelKey& p, int stopLevel, Entries& entries)
{
    std::deque<VoxelKey> roots;

    for (int i = 0; i < 8; ++i)
    {
        VoxelKey c = p.child(i);
        auto ci = m_childCounts.find(c);
        if (ci != m_childCounts.end())
        {

            if (c.level() != stopLevel || ci->second <= MinHierarchySize)
            {
                entries.push_back({c, m_written[c]});
                auto r = emit(c, stopLevel, entries);
                roots.insert(roots.end(), r.begin(), r.end());
            }
            else
            {
                entries.push_back({c, -1});
                roots.push_back(c);
            }
        }
    }
    return roots;
}

} // namespace bu
} // namespace ept2
