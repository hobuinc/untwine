#include <regex>
#include <set>
#include <string>
#include <vector>

#include <pdal/util/ProgramArgs.hpp>

#include "../common/VoxelKey.hpp"

#include "BuPyramid.hpp"
#include "BuTypes.hpp"
#include "FileInfo.hpp"
#include "OctantInfo.hpp"

int main(int argc, char *argv[])
{
    std::vector<std::string> arglist;

    // Skip the program name.
    argv++;
    argc--;
    while (argc--)
        arglist.push_back(*argv++);

    ept2::bu::BuPyramid builder;
    builder.run(arglist);

    return 0;
}

namespace ept2
{
namespace bu
{

/// BuPyramid

BuPyramid::BuPyramid() : m_manager(m_b)
{}

void BuPyramid::addArgs(pdal::ProgramArgs& programArgs)
{

    programArgs.add("output_dir", "Output directory", m_b.outputDir).setPositional();
    programArgs.add("input_dir", "Input directory", m_b.inputDir).setPositional();
}


void BuPyramid::run(const std::vector<std::string>& options)
{
    pdal::ProgramArgs programArgs;

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

    std::thread runner(&PyramidManager::run, &m_manager);
    try
    {
        readBaseInfo();
        getInputFiles();
        createDirs();
        queueWork();
    }
    catch (const Error& err)
    {
        std::cerr << err.what() << "!\n";
    }
    runner.join();
    writeInfo();
}


void BuPyramid::readBaseInfo()
{
    std::string baseFilename = m_b.inputDir + "/info.txt";

    std::ifstream in(baseFilename);
    if (!in)
        throw Error("Can't open 'info.txt' in directory '" + m_b.inputDir + "'.");

    in >> m_b.maxLevel;

    in >> m_b.bounds.minx >> m_b.bounds.miny >> m_b.bounds.minz;
    in >> m_b.bounds.maxx >> m_b.bounds.maxy >> m_b.bounds.maxz;

    if (!in)
        throw "Couldn't read info file.";

    m_b.pointSize = 0;
    while (true)
    {
        FileDimInfo fdi;
        in >> fdi;
        if (!in)
            break;
        if (fdi.name.empty())
            throw Error("Invalid dimension in info.txt.");
        m_b.pointSize += pdal::Dimension::size(fdi.type);
        m_b.dimInfo.push_back(fdi);
    }
}


void BuPyramid::createDirs()
{
    pdal::FileUtils::createDirectory(m_b.outputDir);
    pdal::FileUtils::deleteFile(m_b.outputDir + "/ept.json");
    pdal::FileUtils::deleteDirectory(m_b.outputDir + "/ept-data");
    pdal::FileUtils::deleteDirectory(m_b.outputDir + "/ept-hierarchy");
    pdal::FileUtils::createDirectory(m_b.outputDir + "/ept-data");
    pdal::FileUtils::createDirectory(m_b.outputDir + "/ept-hierarchy");
}


void BuPyramid::writeInfo()
{
    std::ofstream out(m_b.outputDir + "/ept.json");

    out << "{\n";

    out << "\"bounds\": [" <<
        m_b.bounds.minx << ", " << m_b.bounds.miny << ", " << m_b.bounds.minz << ", " <<
        m_b.bounds.maxx << ", " << m_b.bounds.maxy << ", " << m_b.bounds.maxz << "],\n";
    out << "\"boundsConforming\": [" <<
        m_b.bounds.minx << ", " << m_b.bounds.miny << ", " << m_b.bounds.minz << ", " <<
        m_b.bounds.maxx << ", " << m_b.bounds.maxy << ", " << m_b.bounds.maxz << "],\n";
    out << "\"dataType\": \"laszip\",\n";
    out << "\"hierarchyType\": \"json\",\n";
    out << "\"points\": " << m_manager.totalPoints() << ",\n";
    out << "\"span\": 128,\n";
    out << "\"version\": \"1.0.0\",\n";
    out << "\"schema\": [\n";
    for (auto di = m_b.dimInfo.begin(); di != m_b.dimInfo.end(); ++di)
    {
        const FileDimInfo& fdi = *di;

        out << "\t{";
            out << "\"name\": \"" << fdi.name << "\", ";
            out << "\"type\": \"" <<
                pdal::Dimension::toName(pdal::Dimension::base(fdi.type)) << "\", ";
            out << "\"size\": " << pdal::Dimension::size(fdi.type) << " ";
        out << "}";
        if (di + 1 != m_b.dimInfo.end())
            out << ",";
        out << "\n";
    }
    out << "],\n";
    out << "\"srs\": {}\n";
    out << "}\n";
}


void BuPyramid::getInputFiles()
{
    auto matches = [](const std::string& f)
    {
        std::regex check("([0-9]+)-([0-9]+)-([0-9]+)-([0-9]+)\\.bin");
        std::smatch m;
        if (!std::regex_match(f, m, check))
            return VoxelKey(0, 0, 0, 0);
        int level = std::stoi(m[1].str());
        int x = std::stoi(m[2].str());
        int y = std::stoi(m[3].str());
        int z = std::stoi(m[4].str());
        return VoxelKey(x, y, z, level);
    };

    std::vector<std::string> files = pdal::FileUtils::directoryList(m_b.inputDir);

    VoxelKey root;
    for (std::string file : files)
    {
        uintmax_t size = pdal::FileUtils::fileSize(file);
        file = pdal::FileUtils::getFilename(file);
        VoxelKey key = matches(file);
        if (key != root)
            m_allFiles.emplace(key, FileInfo(file, size / m_b.pointSize));
    }

    // Remove any files that are hangers-on from a previous run - those that are parents
    // of other input.
    for (auto it = m_allFiles.begin(); it != m_allFiles.end(); ++it)
    {
        VoxelKey k = it->first;
        while (k != root)
        {
            k = k.parent();
            m_allFiles.erase(k);
        };
    }
}


void BuPyramid::queueWork()
{
    std::set<VoxelKey> needed;
    std::vector<OctantInfo> have;
    const VoxelKey root;

    for (auto& afi : m_allFiles)
    {
        VoxelKey k = afi.first;
        FileInfo& f = afi.second;

        // Stick an OctantInfo for this file in the 'have' list.
        OctantInfo o(k);
        o.appendFileInfo(f);
        have.push_back(o);

        // Walk up the tree and make sure that we're populated for all children necessary
        // to process to the top level.
        while (k != root)
        {
            k = k.parent();
            for (int i = 0; i < 8; ++i)
                needed.insert(k.child(i));
        }
    }

    // Now remove entries for the files we have and their parents.
    for (const OctantInfo& o : have)
    {
        VoxelKey k = o.key();
        while (k != root)
        {
            needed.erase(k);
            k = k.parent();
        }
    }

    // Queue what we have and what's left that's needed.
    for (const OctantInfo& o : have)
        m_manager.queue(o);
    for (const VoxelKey& k : needed)
        m_manager.queue(OctantInfo(k));
}

/**
void BuPyramid::queueWork()
{
    int voxelWidth = std::pow(2, m_b.maxLevel);

    // Loop through the of each octet of voxels in the max level.
    for (int x = 0; x <= voxelWidth; x++)
    for (int y = 0; y <= voxelWidth; y++)
    for (int z = 0; z <= voxelWidth; z++)
    {
        VoxelKey k(x, y, z, m_b.maxLevel);
        OctantInfo o(k);

        std::string s = k.toString() + ".bin";
        auto fii = std::find_if(m_allFiles.begin(), m_allFiles.end(),
            [&s](const FileInfo& fi){ return s == fi.filename(); });
        if (fii != m_allFiles.end())
        {
            FileInfo& fi = *fii;
            o.appendFileInfo(fi);
        }
        m_manager.queue(o);
    }
}
**/

} // namespace bu
} // namespace ept2
