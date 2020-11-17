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


#include "Epf.hpp"
#include "EpfTypes.hpp"
#include "FileProcessor.hpp"
#include "Reprocessor.hpp"
#include "Writer.hpp"
#include "../common/Common.hpp"

#include <unordered_set>

#include <pdal/Dimension.hpp>
#include <pdal/PointLayout.hpp>
#include <pdal/StageFactory.hpp>
#include <pdal/util/Bounds.hpp>
#include <pdal/util/FileUtils.hpp>
#include <pdal/util/ProgramArgs.hpp>

using namespace pdal;

int main(int argc, char *argv[])
{
    std::vector<std::string> arglist;

    // Skip the program name.
    argv++;
    argc--;
    while (argc--)
        arglist.push_back(*argv++);

    ept2::epf::Epf preflight;
    try
    {
        preflight.run(arglist);
    }
    catch (const ept2::epf::Error& err)
    {
        std::cerr << "epf Error: " << err.what() << "\n";
        return -1;
    }

    return 0;
}

namespace ept2
{
namespace epf
{

void writeMetadata(const std::string& outputDir, const Grid& grid,
    const std::string& srs, const pdal::PointLayoutPtr& layout)
{
    std::ofstream out(outputDir + "/" + MetadataFilename);
    pdal::BOX3D b = grid.processingBounds();
    out.precision(10);
    out << b.minx << " " << b.miny << " " << b.minz << "\n";
    out << b.maxx << " " << b.maxy << " " << b.maxz << "\n";
    out << "\n";

    b = grid.conformingBounds();
    out << b.minx << " " << b.miny << " " << b.minz << "\n";
    out << b.maxx << " " << b.maxy << " " << b.maxz << "\n";
    out << "\n";

    out << srs << "\n";
    out << "\n";

    for (Dimension::Id id : layout->dims())
        out << layout->dimName(id) << " " << (int)layout->dimType(id) << " " <<
            layout->dimOffset(id) << "\n";
}

/// Epf

Epf::Epf() : m_pool(8)
{}

void Epf::addArgs(ProgramArgs& programArgs)
{
    programArgs.add("output_dir", "Output directory", m_outputDir).setPositional();
    programArgs.add("files", "Files to preflight", m_files).setPositional();
    programArgs.add("file_limit", "Max number of files to process", m_fileLimit, (size_t)10000000);
    programArgs.add("level", "Set the actual level to use, rather than guess.", m_level, -1);
    programArgs.add("cube", "Make a cube, rather than a rectangular solid.", m_doCube, true);
}

void Epf::run(const std::vector<std::string>& options)
{
    double millionPoints = 0;
    BOX3D totalBounds;
    ProgramArgs programArgs;

    addArgs(programArgs);
    try
    {
        programArgs.parse(options);
    }
    catch (const pdal::arg_error& err)
    {
        std::cerr << err.what() << "\n";
        return;
    }
    if (pdal::FileUtils::fileExists(m_outputDir + "/" + MetadataFilename))
        throw Error("Output directory already contains EPT data.");

    m_grid.setCubic(m_doCube);

    std::vector<FileInfo> fileInfos;
    createFileInfo(fileInfos);

    if (m_level != -1)
        m_grid.resetLevel(m_level);

    // This is just a debug thing that will allow the number of input files to be limited.
    if (fileInfos.size() > m_fileLimit)
        fileInfos.resize(m_fileLimit);

    // Stick all the dimension names from each input file in a set.
    std::unordered_set<std::string> allDimNames;
    for (const FileInfo& fi : fileInfos)
        for (const FileDimInfo& fdi : fi.dimInfo)
            allDimNames.insert(fdi.name);

    // Register the dimensions, either as the default type or double if we don't know
    // what it is.
    PointLayoutPtr layout(new PointLayout());
    for (const std::string& dimName : allDimNames)
    {
        Dimension::Type type = Dimension::defaultType(Dimension::id(dimName));
        if (type == Dimension::Type::None)
            type = Dimension::Type::Double;
        layout->registerOrAssignDim(dimName, type);
    }
    layout->finalize();

    // Fill in dim info now that the layout is finalized.
    for (FileInfo& fi : fileInfos)
    {
        for (FileDimInfo& di : fi.dimInfo)
        {
            di.dim = layout->findDim(di.name);
            di.type = layout->dimType(di.dim);
            di.offset = layout->dimOffset(di.dim);
        }
    }

    // Make a writer with 4 threads.
    m_writer.reset(new Writer(m_outputDir, 4));

    // Sort file infos so the largest files come first. This helps to make sure we don't delay
    // processing big files that take the longest (use threads more efficiently).
    std::sort(fileInfos.begin(), fileInfos.end(), [](const FileInfo& f1, const FileInfo& f2)
        { return f1.numPoints > f2.numPoints; });
    // Add the files to the processing pool
    for (const FileInfo& fi : fileInfos)
    {
        int pointSize = layout->pointSize();
        m_pool.add([&fi, pointSize, this]()
        {
            FileProcessor fp(fi, pointSize, m_grid, m_writer.get());
            fp.run();
        });
    }

    // Wait for  all the processors to finish and restart.
    m_pool.cycle();

    // Tell the writer that it can exit. stop() will block until the writer threads
    // are finished.
    m_writer->stop();

    // Get totals from the current writer.
    //ABELL - would be nice to avoid this copy, but it probably doesn't matter much.
    Totals totals = m_writer->totals();

    // Make a new writer.
    m_writer.reset(new Writer(m_outputDir, 4));
    for (auto& t : totals)
    {
        VoxelKey key = t.first;
        int numPoints = t.second;
        if (numPoints > MaxPointsPerNode)
        {
            int pointSize = layout->pointSize();

            m_pool.add([key, numPoints, pointSize, this]()
            {
                Reprocessor r(key, numPoints, pointSize, m_outputDir, m_grid, m_writer.get());
                r.run();
            });
        }
    }
    m_pool.stop();
    m_writer->stop();
    writeMetadata(m_outputDir, m_grid,
        m_srsFileInfo.valid() ? m_srsFileInfo.srs.getWKT() : "NONE", layout);
}

void Epf::createFileInfo(std::vector<FileInfo>& fileInfos)
{
    std::vector<std::string> filenames;

    // If any of the specified input files is a directory, get the names of the files
    // in the directory and add them.
    for (std::string& filename : m_files)
    {
        if (FileUtils::isDirectory(filename))
        {
            std::vector<std::string> dirfiles = FileUtils::directoryList(filename);
            filenames.insert(filenames.end(), dirfiles.begin(), dirfiles.end());
        }
        else
            filenames.push_back(filename);
    }

    // Determine a driver for each file and get a preview of the file.  If we couldn't
    // Create a FileInfo object containing the file bounds, dimensions, filename and
    // associated driver.  Expand our grid by the bounds and file point count.
    for (std::string& filename : filenames)
    {
        StageFactory factory;
        std::string driver = factory.inferReaderDriver(filename);
        if (driver.empty())
            throw Error("Can't infer reader for '" + filename + "'.");
        Stage *s = factory.createStage(driver);
        Options opts;
        opts.add("filename", filename);
        s->setOptions(opts);

        QuickInfo qi = s->preview();

        if (!qi.valid())
            throw "Couldn't get quick info for '" + filename + "'.";

        FileInfo fi;
        fi.bounds = qi.m_bounds;
        fi.numPoints = qi.m_pointCount;
        for (const std::string& name : qi.m_dimNames)
            fi.dimInfo.push_back(FileDimInfo(name));
        fi.filename = filename;
        fi.driver = driver;

        if (m_srsFileInfo.valid() && m_srsFileInfo.srs != qi.m_srs)
        {
            std::cerr << "Files have mismatched SRS values. Using SRS from '" <<
                m_srsFileInfo.filename << "'.\n";
        }
        fi.srs = qi.m_srs;
        fileInfos.push_back(fi);
        if (!m_srsFileInfo.valid() && qi.m_srs.valid())
        {
            m_srsFileInfo = fi;
            std::cerr << "Set SRS file info fo " << m_srsFileInfo.filename << "!\n";
        }

        m_grid.expand(qi.m_bounds, qi.m_pointCount);
    }
}

} // namespace epf
} // namespace ept2
