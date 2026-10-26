#include <TestConfig.hpp>
#include <pdal/util/Utils.hpp>
#include <gtest/gtest.h>
#include <filesystem>
#include <fstream>
#include <lazperf/writers.hpp>

#include <lazperf/readers.hpp>
#include <lazperf/las.hpp>

#include "Stats.hpp"

GTEST_API_ int main(int argc, char **argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

namespace
{

std::filesystem::path binpath(const std::string& path = {})
{
    std::filesystem::path out = TestConfig::binaryPath();
    if (!path.empty())
        out += path;
    return out;
}

std::filesystem::path datapath(const std::string& path = {})
{
    std::filesystem::path out = TestConfig::dataPath();
    if (!path.empty())
        out += path;
    return out;
}

std::filesystem::path temppath(const std::string& path = {})
{
    std::filesystem::path out = binpath() / "temp";
    std::filesystem::create_directory(out);
    if (!path.empty())
        out /= path;
    return out;
}

std::filesystem::path outfile(const std::string& path = {})
{
    std::string filename = "untwine_test.copc.laz";
    if (!path.empty())
        filename = path;
    return temppath(filename);
}


int runUntwine(const std::filesystem::path& in, const std::filesystem::path& out)
{
    std::string cmd = binpath("untwine").generic_string() +
        " --files=" + in.generic_string() + " " + out.generic_string();

    std::string output;
    return pdal::Utils::run_shell_command(cmd, output);
}

int runBuffers(const std::string& args, const std::filesystem::path& out, std::string& log)
{
    std::filesystem::remove_all(out.generic_string() + "_tmp");
    std::string command = "\"" + binpath("untwine").generic_string() +
        "\" \"" + out.generic_string() + "\" " + args + " 2>&1";
#ifdef _WIN32
    command = "\"" + command + "\"";
#endif
    return pdal::Utils::run_shell_command(command, log);
}

} // unnamed namespace

namespace untwine
{
namespace test
{

bool verify()
{
    return true;
}

struct EbInfo
{
    std::string name;
    int size;

};

struct Stats
{
    int pdrf_;
    pdal::Dimension::Type eb1type_;
    pdal::Dimension::Type eb2type_;
    pdal::Dimension::Type eb3type_;
    std::array<Summary, 13> summary_;

    Stats(int pdrf, pdal::Dimension::Type eb1type, pdal::Dimension::Type eb2type,
            pdal::Dimension::Type eb3type) :
        pdrf_(pdrf), eb1type_(eb1type), eb2type_(eb2type), eb3type_(eb3type)
    {}

    static const int X = 0;
    static const int Y = 1;
    static const int Z = 2;
    static const int Intensity = 3;
    static const int UserData = 4;
    static const int PointSource = 5;
    static const int GpsTime = 6;
    static const int Red = 7;
    static const int Green = 8;
    static const int Blue = 9;
    static const int Eb1 = 10;
    static const int Eb2 = 11;
    static const int Eb3 = 12;

    static const std::array<std::string, 13> dimNames_;

    void accumulate(char *buf)
    {
        auto ebSize = [](pdal::Dimension::Type type)
        {
            switch (type)
    	    {
		    case pdal::Dimension::Type::Unsigned8:
                return 1;
		    case pdal::Dimension::Type::Signed8:
                return 1;
		    case pdal::Dimension::Type::Unsigned16:
                return 2;
		    case pdal::Dimension::Type::Signed16:
                return 2;
		    case pdal::Dimension::Type::Unsigned32:
                return 4;
		    case pdal::Dimension::Type::Signed32:
                return 4;
		    case pdal::Dimension::Type::Unsigned64:
                return 8;
		    case pdal::Dimension::Type::Signed64:
                return 8;
		    case pdal::Dimension::Type::Float:
                return 4;
		    case pdal::Dimension::Type::Double:
                return 8;
            default:
                return 0;
            }
        };

        if (pdrf_ > 5)
            accumulate((lazperf::las::point14 *)buf);
        else
            accumulate((lazperf::las::point10 *)buf);
        buf += lazperf::baseCount(pdrf_);

        if (eb1type_ != pdal::Dimension::Type::None)
        {
            summary_[Eb1].insert(fetchEb(buf, eb1type_));
            buf += ebSize(eb1type_);
        }
        if (eb2type_ != pdal::Dimension::Type::None)
        {
            summary_[Eb2].insert(fetchEb(buf, eb2type_));
            buf += ebSize(eb2type_);
        }
        if (eb3type_ != pdal::Dimension::Type::None)
        {
            summary_[Eb3].insert(fetchEb(buf, eb3type_));
            buf += ebSize(eb2type_);
        }
    }

    void accumulate(lazperf::las::point14 *p)
    {
        using namespace lazperf;

        summary_[X].insert(p->x());
        summary_[Y].insert(p->y());
        summary_[Z].insert(p->z());
        summary_[Intensity].insert(p->intensity());
        summary_[UserData].insert(p->userData());
        summary_[PointSource].insert(p->pointSourceID());
        summary_[GpsTime].insert(p->gpsTime());

        char *buf = (char *)p;
        buf += 30;  // 30 bytes for point format 6
        if (pdrf_ == 7 || pdrf_ == 8)
        {
            summary_[Red].insert(utils::unpack<uint16_t>(buf)); buf += 2;  // Red
            summary_[Green].insert(utils::unpack<uint16_t>(buf)); buf += 2;  // Green
            summary_[Blue].insert(utils::unpack<uint16_t>(buf)); buf += 2;  // Blue
        }
    }

    void accumulate(lazperf::las::point10 *p)
    {
        using namespace lazperf;

        summary_[X].insert(p->x);
        summary_[Y].insert(p->y);
        summary_[Z].insert(p->z);
        summary_[Intensity].insert(p->intensity);
        summary_[UserData].insert(p->user_data);
        summary_[PointSource].insert(p->point_source_ID);

        char *buf = (char *)p;
        buf += 20;  // 20 bytes for point format 0
        if (pdrf_ == 1 || pdrf_ == 3)
        {
            summary_[GpsTime].insert(utils::unpack<double>(buf)); buf += 8;
        }
        if (pdrf_ == 2 || pdrf_ == 3)
        {
            summary_[Red].insert(utils::unpack<uint16_t>(buf)); buf += 2;
            summary_[Green].insert(utils::unpack<uint16_t>(buf)); buf += 2;
            summary_[Blue].insert(utils::unpack<uint16_t>(buf)); buf += 2;
        }
    }

    friend bool operator==(const Stats& s1, const Stats& s2);

private:
    static double fetchEb(char *buf, pdal::Dimension::Type type)
    {
        using namespace pdal::Dimension;
        using namespace lazperf::utils;

        double d = 0;

        switch (type)
        {
            case Type::Unsigned8:
                d = (uint8_t)*buf;
                break;
            case Type::Signed8:
                d = (int8_t)*buf;
                break;
            case Type::Signed16:
                d = unpack<int16_t>(buf);
                break;
            case Type::Unsigned16:
                d = unpack<uint16_t>(buf);
                break;
            case Type::Signed32:
                d = unpack<int32_t>(buf);
                break;
            case Type::Unsigned32:
                d = unpack<uint32_t>(buf);
                break;
            case Type::Signed64:
                d = (double)unpack<int64_t>(buf);
                break;
            case Type::Unsigned64:
                d = (double)unpack<uint64_t>(buf);
                break;
            case Type::Float:
                d = unpack<float>(buf);
                break;
            case Type::Double:
                d = unpack<double>(buf);
                break;
            default:
                break;
        }
        return d;
    }
};

const std::array<std::string, 13> Stats::dimNames_ { "X", "Y", "Z", "Intensity", "UserData",
        "PointSource", "GpsTime", "Red", "Green", "Blue", "Eb1", "Eb2", "Eb3" };

void verifyStats(const Stats& s1, const Stats& s2)
{
    auto test = [](double d1, double d2, double bound, const std::string& state,
        const std::string& dim)
    {
        EXPECT_NEAR(d1, d2, bound) << "Bad " << state << " for '" << dim <<
            "': " << d1 << " vs. " << d2;
    };

    for (size_t i = 0; i < s1.summary_.size(); ++i)
    {
        if ((s1.pdrf_ == 0 || s1.pdrf_ == 1) &&
            (i == Stats::Red || i == Stats::Green || i == Stats::Blue))
            continue;
        if ((s1.pdrf_ == 0 || s1.pdrf_ == 2) && i == Stats::GpsTime)
            continue;
        const std::string& dim = Stats::dimNames_[i];
        test(s1.summary_[i].minimum(), s2.summary_[i].minimum(), 1E-7, "minimum", dim);
        test(s1.summary_[i].maximum(), s2.summary_[i].maximum(), 1E-7, "maximum", dim);
        test(s1.summary_[i].average(), s2.summary_[i].average(), 1E-5, "average", dim);
        test(s1.summary_[i].stddev(), s2.summary_[i].stddev(), 1E-5, "stddev", dim);
    }
}


void verifyStats(const std::filesystem::path& file1, const std::filesystem::path& file2)
{
    lazperf::reader::named_file f1(file1.generic_string());
    lazperf::reader::named_file f2(file2.generic_string());

    size_t pc1 = f1.header().point_count;
    size_t pc2 = f2.header().point_count;
    ASSERT_EQ(pc1, pc2);

    auto ebType = [](const lazperf::eb_vlr& vlr, size_t pos) -> pdal::Dimension::Type
    {
        using namespace pdal::Dimension;

        static const std::array<Type, 11> types { Type::None, Type::Unsigned8, Type::Signed8,
            Type::Unsigned16, Type::Signed16, Type::Unsigned32, Type::Signed32, Type::Unsigned64,
            Type::Signed64, Type::Float, Type::Double };

        if (pos >= vlr.items.size())
            return Type::None;

        int itype = vlr.items[pos].data_type;
        if (itype < 1 || itype > 10)
            return Type::None;

        return types[itype];
    };

    lazperf::eb_vlr vlr = f1.ebVlr();
    Stats s1(f1.header().point_format_id, ebType(vlr, 0), ebType(vlr, 1), ebType(vlr, 2));

    vlr = f2.ebVlr();
    Stats s2(f2.header().point_format_id, ebType(vlr, 0), ebType(vlr, 1), ebType(vlr, 2));

    char buf[1000];
    for (size_t i = 0; i < pc1; ++i)
    {
        f1.readPoint(buf);
        s1.accumulate(buf);
        f2.readPoint(buf);
        s2.accumulate(buf);
    }

    verifyStats(s1, s2);
}

TEST(Untwine, t1)
{
    std::filesystem::remove(outfile());

    std::filesystem::path filename = datapath("autzen_trim.laz");
    runUntwine(filename, outfile());
//    verify(outfile());
    verifyStats(filename, outfile());

    std::filesystem::remove(outfile());
}

// Verfiy extra bytes fields are written correctly and the types match the input.
// (Issue 155)
TEST(Untwine, t2)
{
    std::filesystem::remove(outfile());

    std::filesystem::path filename = datapath("eb.laz");
    runUntwine(filename, outfile());
    verifyStats(filename, outfile());

    lazperf::reader::named_file f1(filename.generic_string());
    lazperf::reader::named_file f2(outfile().generic_string());
    lazperf::eb_vlr vlr1 = f1.ebVlr();
    lazperf::eb_vlr vlr2 = f2.ebVlr();

    EXPECT_EQ(vlr1.items.size(), vlr2.items.size());
    for (size_t i = 0; i < vlr1.items.size(); ++i)
    {
        const lazperf::eb_vlr::ebfield& f1 = vlr1.items[i];
        const lazperf::eb_vlr::ebfield& f2 = vlr2.items[i];

        EXPECT_EQ(f1.data_type, f2.data_type);
        EXPECT_EQ(f1.name, f2.name);
    }
    f1.close();
    f2.close();

    std::filesystem::remove(outfile());
}

TEST(Buffers, conversions)
{
    const auto in = datapath("eb.laz");
    const std::vector<std::string> settings {
        "", " --buffer_size=40960 --buffer_count=1000",
        " --buffer_size=1031 --buffer_count=3",
        " --buffer_size=1048577 --buffer_count=4096"
    };
    for (const auto& setting : settings)
    {
        SCOPED_TRACE(setting);
        std::string log;
        ASSERT_EQ(runBuffers("--threads=2 --files=\"" + in.generic_string() + "\"" +
            setting, outfile(), log), 0) << log;
        verifyStats(in, outfile());
    }
}

TEST(Buffers, invalidNumericLeavesOutputUntouched)
{
    const std::vector<std::string> settings {
        "--buffer_size=0", "--buffer_size=-1", "--buffer_size=abc", "--buffer_count=0",
        "--buffer_count=-1", "--buffer_size=18446744073709551616",
        "--buffer_count=18446744073709551616",
        "--buffer_size=18446744073709551615", "--buffer_count=18446744073709551615",
        "--buffer_size=9223372036854775807 --buffer_count=3",
        "--buffer_count=2", "--buffer_count=1"
    };
    for (const auto& setting : settings)
    {
        SCOPED_TRACE(setting);
        {
            std::ofstream out(outfile());
            out << "keep output";
        }
        std::string log;
        EXPECT_NE(runBuffers("--threads=2 --files=\"" + datapath("eb.laz").generic_string() +
            "\" " + setting, outfile(), log), 0);
        EXPECT_TRUE(log.find("Invalid --buffer") != std::string::npos ||
            log.find("--buffer_count must exceed") != std::string::npos) << log;
        std::ifstream out(outfile());
        std::string text((std::istreambuf_iterator<char>(out)), {});
        EXPECT_EQ(text, "keep output");
    }
}

TEST(Buffers, recordTooLarge)
{
    std::string log;
    EXPECT_NE(runBuffers("--threads=2 --buffer_count=3 --buffer_size=1 --files=\"" +
        datapath("eb.laz").generic_string() + "\"", outfile(), log), 0);
    EXPECT_NE(log.find("record"), std::string::npos) << log;
}

TEST(Buffers, reprocessingPreservesPointsAndFlags)
{
    auto in = temppath("flags.laz");
    lazperf::writer::named_file::config config;
    config.pdrf = 6;
    config.minor_version = 4;
    config.scale = {0.01, 0.01, 0.01};

    // Exceed MaxPointsPerNode (100000) in the cluster to force reprocessing.
    // The final, distant point expands the bounds so the cluster shares one voxel.
    const size_t count = 100257;
    {
        lazperf::writer::named_file writer(in.generic_string(), config);
        for (size_t i = 0; i < count; ++i)
        {
            lazperf::las::point14 point {};
            point.setX(i + 1 == count ? 100000 : i % 101);
            point.setY(i + 1 == count ? 100000 : (i / 101) % 101);
            point.setZ(i + 1 == count ? 100000 : i % 37);
            point.setGpsTime(double(i));
            point.setReturns(0x11);
            point.setFlags(i % 256);
            point.setClassification(i % 32);
            writer.writePoint(reinterpret_cast<const char *>(&point));
        }
        writer.close();
    }
    for (const auto& setting : {std::string("--buffer_size=1031 --buffer_count=3"),
        std::string("--buffer_size=1048577 --buffer_count=4096")})
    {
        std::string log;
        ASSERT_EQ(runBuffers("--threads=2 --level=0 --progress_debug --files=\"" + in.generic_string() +
            "\" " + setting, outfile(), log), 0) << log;
        EXPECT_NE(log.find("Reprocessed voxel"), std::string::npos) << log;
        lazperf::reader::named_file reader(outfile().generic_string());
        ASSERT_EQ(reader.pointCount(), count);
        std::vector<bool> seen(count);
        for (size_t n = 0; n < count; ++n)
        {
            char bytes[30];
            reader.readPoint(bytes);
            lazperf::las::point14 point(bytes);
            size_t i = size_t(point.gpsTime());
            ASSERT_LT(i, count);
            EXPECT_FALSE(seen[i]);
            seen[i] = true;
            EXPECT_EQ(point.flags(), i % 256);
            EXPECT_EQ(point.classification(), i % 32);
            EXPECT_NEAR(point.x() * reader.header().scale.x + reader.header().offset.x,
                (i + 1 == count ? 100000 : i % 101) * 0.01, 0.005);
            EXPECT_NEAR(point.y() * reader.header().scale.y + reader.header().offset.y,
                (i + 1 == count ? 100000 : (i / 101) % 101) * 0.01, 0.005);
            EXPECT_NEAR(point.z() * reader.header().scale.z + reader.header().offset.z,
                (i + 1 == count ? 100000 : i % 37) * 0.01, 0.005);
        }
    }
}

TEST(Buffers, multipleInputs)
{
    std::string log;
    const auto in = datapath("eb.laz");
    ASSERT_EQ(runBuffers("--threads=2 --buffer_count=3 --buffer_size=1031 "
        "--files=\"" + in.generic_string() + "\" --files=\"" + in.generic_string() + "\"",
        outfile(), log), 0) << log;
    lazperf::reader::named_file input(in.generic_string());
    lazperf::reader::named_file output(outfile().generic_string());
    EXPECT_EQ(output.pointCount(), input.pointCount() * 2);
}

TEST(Buffers, exactlyOneInternalRecord)
{
    const auto in = datapath("eb.laz");
    std::string log;
    const std::string args = "--threads=2 --buffer_count=3 --files=\"" +
        in.generic_string() + "\"";
    ASSERT_NE(runBuffers(args + " --buffer_size=1", outfile(), log), 0);
    const auto start = log.find("record (");
    ASSERT_NE(start, std::string::npos) << log;
    const size_t recordSize = std::stoull(log.substr(start + 8));
    ASSERT_GT(recordSize, 1u);
    ASSERT_EQ(runBuffers(args + " --buffer_size=" + std::to_string(recordSize),
        outfile(), log), 0) << log;
    verifyStats(in, outfile());
}

} // namespace test
} // namespace untwine
