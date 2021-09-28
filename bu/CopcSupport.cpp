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

#include <iostream>
#include <limits>

#include "CopcSupport.hpp"

#include <pdal/PointLayout.hpp>
#include <pdal/util/Algorithm.hpp>
#include <pdal/util/OStream.hpp>
#include <pdal/util/Extractor.hpp>

#include <lazperf/filestream.hpp>

#include "../untwine/Common.hpp"
#include "../untwine/FileDimInfo.hpp"


namespace untwine
{
namespace bu
{

CopcSupport::CopcSupport(const BaseInfo& b) :
    m_b(b),
    m_lazVlr(b.opts.pointFormatId, ebVLRSize(), lazperf::VariableChunkSize),
    m_ebVlr(ebVLRCount()),
    m_wktVlr(b.srs.getWKT1()),
    m_extentVlr(extentVLRSize())
{
    m_f.open(b.opts.outputName, std::ios::out | std::ios::binary);

    //ABELL
    m_header.file_source_id = 0;
    m_header.global_encoding = (1 << 4); // Set for WKT
    //ABELL
    m_header.creation.day = 1;
    m_header.creation.year = 1;
    m_header.header_size = lazperf::header14::Size;
    m_header.vlr_count = 4;
    m_header.point_format_id = b.opts.pointFormatId;
    m_header.point_format_id |= (1 << 7);    // Bit for laszip
    m_header.point_record_length = lazperf::baseCount(b.opts.pointFormatId) + ebVLRSize();
    m_header.scale.x = b.scale[0];
    m_header.scale.y = b.scale[1];
    m_header.scale.z = b.scale[2];
    m_header.offset.x = b.offset[0];
    m_header.offset.y = b.offset[1];
    m_header.offset.z = b.offset[2];
    m_header.point_offset = lazperf::header14::Size +
        lazperf::vlr_header::Size + m_copcVlr.size() +
        lazperf::vlr_header::Size + m_lazVlr.size() +
        lazperf::vlr_header::Size + m_wktVlr.size() +
        lazperf::vlr_header::Size + m_extentVlr.size();
    if (m_header.ebCount())
    {
        m_header.vlr_count++;
        m_header.point_offset += lazperf::vlr_header::Size + m_ebVlr.size();
    }

    // The chunk table offset is written as the first 8 bytes of the point data in LAZ.
    m_chunkOffsetPos = m_header.point_offset;
    // The actual point data comes after the chunk table offset.
    m_pointPos = m_chunkOffsetPos + sizeof(uint64_t);

    setEbVLR();
}


void CopcSupport::setEbVLR()
{

    using DT = pdal::Dimension::Type;
    const pdal::Dimension::Type lastypes[] = {
        DT::None, DT::Unsigned8, DT::Signed8, DT::Unsigned16, DT::Signed16,
        DT::Unsigned32, DT::Signed32, DT::Unsigned64, DT::Signed64,
        DT::Float, DT::Double
    };

    VLRInfo info = computeVLRInfo();
    m_ebVlr.items.clear();
    for (FileDimInfo& fdi: info.ebDims)
    {
        lazperf::eb_vlr::ebfield field;

        field.name = fdi.name;

        uint8_t lastype = 0;
        for (size_t i = 0; i < sizeof(lastypes) / sizeof(lastypes[0]); ++i)
            if (fdi.type == lastypes[i])
            {
                lastype = i;
                break;
            }

        field.data_type = lastype;
        m_ebVlr.items.push_back(field);
    }



}


CopcSupport::VLRInfo::VLRInfo()
    : ebVLRSize(0)
    , extentsVLRSize(0)
{}


CopcSupport::VLRInfo CopcSupport::computeVLRInfo() const
{
    using namespace pdal;

    VLRInfo info;

    // Start with PDRF 6 dim list for statistics
    // No XYZ because we don't gather stats for those
    Dimension::IdList statsDims = { Dimension::Id::X, Dimension::Id::Y, Dimension::Id::Z,
        Dimension::Id::Intensity, Dimension::Id::ReturnNumber, Dimension::Id::NumberOfReturns,
        Dimension::Id::ScanDirectionFlag, Dimension::Id::EdgeOfFlightLine,
        Dimension::Id::Classification, Dimension::Id::ScanAngleRank, Dimension::Id::UserData,
        Dimension::Id::PointSourceId, Dimension::Id::GpsTime };

    if (m_b.opts.pointFormatId == 7)
    {
        statsDims.push_back(Dimension::Id::Red);
        statsDims.push_back(Dimension::Id::Green);
        statsDims.push_back(Dimension::Id::Blue);
    }
    if (m_b.opts.pointFormatId == 8)
    {
        statsDims.push_back(Dimension::Id::Red);
        statsDims.push_back(Dimension::Id::Green);
        statsDims.push_back(Dimension::Id::Blue);
        statsDims.push_back(Dimension::Id::Infrared);
    }

    PointLayout layout;
    for (const FileDimInfo& fdi : m_b.dimInfo)
    {
        Dimension::Id dim = layout.registerOrAssignDim(fdi.name, fdi.type);
        if (Utils::contains(statsDims, fdi.dim))
        {
            info.extentsVLRSize = info.extentsVLRSize + (2* sizeof(double));
        } else
        {
            info.extentsVLRSize = info.extentsVLRSize + (2* sizeof(double));
            info.ebVLRSize = info.ebVLRSize + layout.dimSize(dim);
            info.ebVLRCount++;

            info.ebDims.push_back(fdi);
            statsDims.push_back(fdi.dim);
        }
    }
    info.statsDims = statsDims;
    return info;

}


int CopcSupport::ebVLRSize() const
{
    VLRInfo info = computeVLRInfo();
    return info.ebVLRSize;
}

int CopcSupport::ebVLRCount() const
{
    VLRInfo info = computeVLRInfo();
    return info.ebVLRCount;
}


int CopcSupport::extentVLRSize() const
{
    VLRInfo info = computeVLRInfo();
    return info.extentsVLRSize;
}


/// \param  size  Size of the chunk in bytes
/// \param  count  Number of points in the chunk
/// \param  key  Key of the voxel the chunk represents
/// \return  The offset of the chunk in the file.
uint64_t CopcSupport::newChunk(const VoxelKey& key, int32_t size, int32_t count)
{
    uint64_t chunkStart = m_pointPos;
    m_pointPos += size;
    assert(count <= (std::numeric_limits<int32_t>::max)() && count >= 0);
    m_chunkTable.push_back({(uint64_t)count, (uint64_t)size});
    m_header.point_count += count;
    m_header.point_count_14 += count;
    m_hierarchy[key] = { chunkStart, size, count };
    return chunkStart;
}

void CopcSupport::updateHeader(const StatsMap& stats)
{
    m_header.maxx = stats.at("X").maximum();
    m_header.maxy = stats.at("Y").maximum();
    m_header.maxz = stats.at("Z").maximum();
    m_header.minx = stats.at("X").minimum();
    m_header.miny = stats.at("Y").minimum();
    m_header.minz = stats.at("Z").minimum();

    for (int i = 1; i <= 15; ++i)
    {
        PointCount count = 0;
        try
        {
            count = stats.at("ReturnNumber").values().at(i);
        }
        catch (const std::out_of_range&)
        {}

        m_header.points_by_return_14[i - 1] = count;
        if (i <= 5)
        {
            m_header.points_by_return[i - 1] = 0;
        }
    }

    // Don't use old point_count because we are pointSourceId 6, 7, or 8
    m_header.point_count = 0;

    VLRInfo info = computeVLRInfo();
    pdal::PointLayout layout;
    for (const FileDimInfo& fdi : m_b.dimInfo)
    {
        pdal::Dimension::Id dim = layout.registerOrAssignDim(fdi.name, fdi.type);
    }

    std::vector<copc_extents_vlr::CopcExtent> extents;
    for (const pdal::Dimension::Id& dimId: info.statsDims)
    {
        if (layout.hasDim(dimId))
        {
            std::string name (layout.dimName(dimId));
            double min = stats.at(name).minimum();
            double max = stats.at(name).maximum();
            extents.push_back(copc_extents_vlr::CopcExtent(min, max));
        }
    }

    // copy into our VLR:
    for (size_t i = 0; i < extents.size(); i++)
    {
        m_extentVlr.items[i].maximum = extents[i].maximum;
        m_extentVlr.items[i].minimum = extents[i].minimum;
    }

}


void CopcSupport::writeHeader()
{
    uint64_t start;
    uint64_t end;
    std::ostream& out = m_f;

    out.seekp(0);
    m_header.write(out);

    m_copcVlr.header().write(out);
    uint64_t copcPos = out.tellp();
    m_copcVlr.write(out);

    m_lazVlr.header().write(out);
    start = out.tellp();
    m_lazVlr.write(out);
    end = out.tellp();
    m_copcVlr.laz_vlr_offset = start;
    m_copcVlr.laz_vlr_size = end - start;

    m_wktVlr.header().write(out);
    start = out.tellp();
    m_wktVlr.write(out);
    end = out.tellp();
    m_copcVlr.wkt_vlr_offset = start;
    m_copcVlr.wkt_vlr_size = end - start;

    m_extentVlr.header().write(out);
    start = out.tellp();
    m_extentVlr.write(out);
    end = out.tellp();
    m_copcVlr.extent_vlr_offset = start;
    m_copcVlr.extent_vlr_size = end - start;


    if (m_header.ebCount())
    {
        m_ebVlr.header().write(out);
        start = out.tellp();
        m_ebVlr.write(out);
        end = out.tellp();
        m_copcVlr.eb_vlr_offset = start;
        m_copcVlr.eb_vlr_size = end - start;
    }

    // Rewrite the COPC VLR with the updated positions and seek back to the end of the VLRs.
    out.seekp(copcPos);
    m_copcVlr.write(out);
    out.seekp(end);

}

void CopcSupport::writeChunkTable()
{
    m_chunkTable.resize(m_chunkTable.size());

    // Write chunk table offset
    pdal::OLeStream out(&m_f);
    out.seek(m_chunkOffsetPos);
    out << m_pointPos;

    // Write chunk table header.
    out.seek(m_pointPos);
    uint32_t version = 0;
    out << version;
    out << (uint32_t)m_chunkTable.size();

    // Write the chunk table itself.
    lazperf::OutFileStream stream(m_f);
    lazperf::compress_chunk_table(stream.cb(), m_chunkTable, true);

    // The ELVRs start after the chunk table.
    m_header.evlr_count = 1;
    m_header.evlr_offset = out.position();
}

void CopcSupport::writeHierarchy(const CountMap& counts)
{
    // Move to the location *after* the EVLR header.
    m_f.seekp(m_header.evlr_offset + lazperf::evlr_header::Size);

    uint64_t beginPos = m_f.tellp();
    Hierarchy root = emitRoot(VoxelKey(0, 0, 0, 0), counts);
    m_copcVlr.root_hier_offset = root.offset;
    m_copcVlr.root_hier_size = root.byteSize;
    uint64_t endPos = m_f.tellp();

    // Now write VLR header.
    lazperf::evlr_header h { 0, "entwine", 1000, (endPos - beginPos), "EPT Hierarchy" };
    m_f.seekp(m_header.evlr_offset);
    h.write(m_f);
}

CopcSupport::Hierarchy CopcSupport::emitRoot(const VoxelKey& root, const CountMap& counts)
{
    const int LevelBreak = 4;
    Entries entries;

    int stopLevel = root.level() + LevelBreak;
    entries.push_back({root, m_hierarchy[root]});
    emitChildren(root, counts, entries, stopLevel);

    pdal::OLeStream out(&m_f);
    uint64_t startPos = out.position();
    for (auto it = entries.begin(); it != entries.end(); ++it)
    {
        VoxelKey& key = it->first;
        Hierarchy& h = it->second;
        out << key.level() << key.x() << key.y() << key.z();
        out << h.offset << h.byteSize << h.pointCount;
    }
    uint64_t endPos = out.position();

    // This is the information about where the hierarchy node was written, to be
    // written with the parent.
    return { startPos, (int32_t)(endPos - startPos), -1 };
}


void CopcSupport::emitChildren(const VoxelKey& p, const CountMap& counts,
    Entries& entries, int stopLevel)
{
    const int MinHierarchySize = 50;

    for (int i = 0; i < 8; ++i)
    {
        VoxelKey c = p.child(i);
        auto ci = counts.find(c);
        if (ci != counts.end())
        {
            // If we're not at a stop level or the number of child nodes is less than 50,
            // just stick them here.
            if (c.level() != stopLevel || ci->second <= MinHierarchySize)
            {
                entries.push_back({c, m_hierarchy[c]});
                emitChildren(c, counts, entries, stopLevel);
            }
            else
                entries.push_back({c, emitRoot(c, counts)});
        }
    }
}


copc_extents_vlr::copc_extents_vlr()
{}


copc_extents_vlr::copc_extents_vlr(int byteSize)
{
    int itemCount = byteSize / sizeof(CopcExtent);
    items.resize(itemCount);
}


copc_extents_vlr::~copc_extents_vlr()
{}


copc_extents_vlr::CopcExtent::CopcExtent() :
    minimum((std::numeric_limits<double>::max)()),
    maximum((std::numeric_limits<double>::min)())
{}


copc_extents_vlr::CopcExtent::CopcExtent(double minimum, double maximum) :
    minimum(minimum),
    maximum(maximum)
{}


copc_extents_vlr copc_extents_vlr::create(std::istream& in, int byteSize)
{
    copc_extents_vlr extentsVlr;
    extentsVlr.read(in, byteSize);
    return extentsVlr;
}


void copc_extents_vlr::read(std::istream& in, int byteSize)
{
    std::vector<char> buf(byteSize);
    pdal::LeExtractor s(buf.data(), buf.size());
    in.read(buf.data(), buf.size());

    int numItems = byteSize / sizeof(CopcExtent);
    items.clear();
    for (int i = 0; i < numItems; ++i)
    {
        CopcExtent field;

        s >> field.minimum >> field.maximum;
        items.push_back(field);
    }
}


void copc_extents_vlr::write(std::ostream& out) const
{
    std::vector<char> buf(size());
    pdal::LeInserter s(buf.data(), buf.size());

    for (auto& i: items)
    {
        s << i.minimum << i.maximum;
    }

    out.write(buf.data(), buf.size());
}


size_t copc_extents_vlr::size() const
{
    return items.size() * sizeof(CopcExtent);
}


lazperf::vlr_header copc_extents_vlr::header() const
{
    return lazperf::vlr_header { 0, "entwine", 10000, (uint16_t)size(), "COPC extents" };
}


// Initialized in header.
copc_info_vlr::copc_info_vlr()
{}


copc_info_vlr::~copc_info_vlr()
{}


copc_info_vlr copc_info_vlr::create(std::istream& in)
{
    copc_info_vlr copcVlr;
    copcVlr.read(in);
    return copcVlr;
}


void copc_info_vlr::read(std::istream& in)
{
    std::vector<char> buf(size());
    in.read(buf.data(), buf.size());
    pdal::LeExtractor s(buf.data(), buf.size());

    s >> span >> root_hier_offset >> root_hier_size;
    s >> laz_vlr_offset >> laz_vlr_size >> wkt_vlr_offset >> wkt_vlr_size;
    s >> eb_vlr_offset >> eb_vlr_size >> extent_vlr_offset >> extent_vlr_size;
    for (int i = 0; i < 9; ++i)
        s >> reserved[i];
}


void copc_info_vlr::write(std::ostream& out) const
{
    std::vector<char> buf(size());
    pdal::LeInserter s(buf.data(), buf.size());

    s << span << root_hier_offset << root_hier_size;
    s << laz_vlr_offset << laz_vlr_size << wkt_vlr_offset << wkt_vlr_size;
    s << eb_vlr_offset << eb_vlr_size << extent_vlr_offset << extent_vlr_size;
    for (int i = 0; i < 9; ++i)
        s << reserved[i];
    out.write(buf.data(), buf.size());
}


size_t copc_info_vlr::size() const
{
    return sizeof(uint64_t) * 20;
}


lazperf::vlr_header copc_info_vlr::header() const
{
    return lazperf::vlr_header { 0, "entwine", 1, (uint16_t)size(), "COPC info" };
}


} // namespace bu
} // namespace untwine
