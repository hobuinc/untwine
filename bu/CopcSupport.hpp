/*****************************************************************************
 *   Copyright (c) 2021, Hobu, Inc. (info@hobu.co)                           *
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

#include <unordered_map>

#include "Stats.hpp"
#include "../untwine/FileDimInfo.hpp"
#include "../untwine/VoxelKey.hpp"

#include <lazperf/lazperf.hpp>
#include <lazperf/vlr.hpp>


namespace untwine
{

struct BaseInfo;

namespace bu
{


struct copc_extents_vlr : public lazperf::vlr
{
public:

    struct CopcExtent
    {
        double minimum;
        double maximum;

        CopcExtent(double minimum, double maximum) : minimum(minimum), maximum(maximum)
        {}

        CopcExtent() : minimum(0), maximum(0)
        {}
    };

    std::vector<CopcExtent> items;

    copc_extents_vlr();
    copc_extents_vlr(int numExtentItems);
    void setItem(int i, const CopcExtent& item);
    void addItem(const CopcExtent& item);
    virtual ~copc_extents_vlr();

    static copc_extents_vlr create(std::istream& in, int byteSize);
    void read(std::istream& in, int byteSize);
    void write(std::ostream& out) const;
    virtual uint64_t size() const;
    virtual lazperf::vlr_header header() const;
    virtual lazperf::evlr_header eheader() const;
};

class CopcSupport
{
public:
    struct Hierarchy
    {
        uint64_t offset;
        int32_t byteSize;
        int32_t pointCount;
    };

    using CountMap = std::unordered_map<VoxelKey, int>;
    using Entries = std::vector<std::pair<VoxelKey, Hierarchy>>;

    CopcSupport(const BaseInfo& b);
    uint64_t newChunk(const VoxelKey& key, int32_t size, int32_t count);
    void updateHeader(const StatsMap& stats);
    void writeHeader();
    void writeChunkTable();
    void writeHierarchy(const CountMap& counts);

private:
    const BaseInfo& m_b;
    std::ofstream m_f;
    lazperf::header14 m_header;
    lazperf::copc_info_vlr m_copcVlr;
    lazperf::laz_vlr m_lazVlr;
    lazperf::eb_vlr m_ebVlr;
    lazperf::wkt_vlr m_wktVlr;
    copc_extents_vlr m_extentVlr;
    std::vector<lazperf::chunk> m_chunkTable;
    uint64_t m_chunkOffsetPos;
    uint64_t m_pointPos;
    std::unordered_map<VoxelKey, Hierarchy> m_hierarchy;

    int extraByteSize() const;
    int numExtentItems() const;
    void setExtentsVlr(const StatsMap& stats);
    Hierarchy emitRoot(const VoxelKey& root, const CountMap& counts);
    void emitChildren(const VoxelKey& root, const CountMap& counts,
        Entries& entries, int stopLevel);
};


} // namesapce bu
} // namesapce untwine
