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
#include "../untwine/VoxelKey.hpp"

#include <lazperf/lazperf.hpp>
#include <lazperf/vlr.hpp>


namespace untwine
{

class BaseInfo;
struct FileDimInfo;
using DimInfoList = std::vector<FileDimInfo>;

namespace bu
{


struct copc_extents_vlr : public lazperf::vlr
{
public:

    struct CopcExtent
    {
        double minimum;
        double maximum;

        CopcExtent(double minimum, double maximum);
        CopcExtent() {};
    };

    std::vector<CopcExtent> items;

    copc_extents_vlr(int itemCount);
    copc_extents_vlr();
    virtual ~copc_extents_vlr();

    static copc_extents_vlr create(std::istream& in, int byteSize);
    void read(std::istream& in, int byteSize);
    void write(std::ostream& out) const;
    virtual size_t size() const;
    virtual lazperf::vlr_header header() const;
};

struct copc_info_vlr : public lazperf::vlr
{
public:
    int64_t span {0};
    uint64_t root_hier_offset {0};
    uint64_t root_hier_size {0};
    uint64_t laz_vlr_offset {0};
    uint64_t laz_vlr_size {0};
    uint64_t wkt_vlr_offset {0};
    uint64_t wkt_vlr_size {0};
    uint64_t eb_vlr_offset {0};
    uint64_t eb_vlr_size {0};
    uint64_t extent_vlr_offset {0};
    uint64_t extent_vlr_size {0};
    uint64_t reserved[9] {0};

    copc_info_vlr();
    virtual ~copc_info_vlr();

    static copc_info_vlr create(std::istream& in);
    void read(std::istream& in);
    void write(std::ostream& out) const;
    virtual size_t size() const;
    virtual lazperf::vlr_header header() const;
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


    struct VLRInfo
    {
        int ebVLRSize {0};
        int ebVLRCount {0};
        int extentVLRCount {0};
        DimInfoList ebDims;
        pdal::Dimension::IdList statsDims;

        VLRInfo();
    };

    BaseInfo m_b;
    std::ofstream m_f;
    lazperf::header14 m_header;
    copc_info_vlr m_copcVlr;
    lazperf::laz_vlr m_lazVlr;
    lazperf::eb_vlr m_ebVlr;
    lazperf::wkt_vlr m_wktVlr;
    copc_extents_vlr m_extentVlr;
    std::vector<lazperf::chunk> m_chunkTable;
    uint64_t m_chunkOffsetPos;
    uint64_t m_pointPos;
    std::unordered_map<VoxelKey, Hierarchy> m_hierarchy;

    int ebVLRSize() const;
    int ebVLRCount() const;
    int extentVLRCount() const;
    Hierarchy emitRoot(const VoxelKey& root, const CountMap& counts);
    void emitChildren(const VoxelKey& root, const CountMap& counts,
        Entries& entries, int stopLevel);

    VLRInfo computeVLRInfo() const;
    void setEbVLR();

};


} // namesapce bu
} // namesapce untwine
