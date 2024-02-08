#include <TestConfig.hpp>
#include <pdal/util/Utils.hpp>
#include <gtest/gtest.h>

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


std::filesystem::path bigfile()
{
    return datapath("autzen_trim.laz");
}

std::filesystem::path outfile(const std::string& path = {})
{
    std::string filename = "untwine_test.tmp";
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

} // unnamed namespace

namespace untwine
{
namespace test
{

bool verify()
{
    return true;
}

struct Stats
{
    int pdrf_;
    int eb1size_;
    int eb2size_;
    int eb3size_;
    std::array<Summary, 13> summary_;

    Stats(int pdrf, int eb1size = 0, int eb2size = 0, int eb3size = 0) :
        pdrf_(pdrf), eb1size_(eb1size), eb2size_(eb2size), eb3size_(eb3size)
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
        if (pdrf_ > 5)
            accumulate((lazperf::las::point14 *)buf);
        else
            accumulate((lazperf::las::point10 *)buf);
        buf += lazperf::baseCount(pdrf_);

        if (eb1size_)
        {
            summary_[Eb1].insert(fetchEb(buf, eb1size_));
            buf += eb1size_;
        }
        if (eb2size_)
        {
            summary_[Eb2].insert(fetchEb(buf, eb2size_));
            buf += eb2size_;
        }
        if (eb3size_)
        {
            summary_[Eb3].insert(fetchEb(buf, eb3size_));
            buf += eb3size_;
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
    static double fetchEb(char *buf, int ebsize)
    {
        uint16_t s;
        uint32_t i;
        double d;

        // Not worrying about byte order.
        switch (ebsize)
        {
            case 1:
                d = *buf;
                break;
            case 2:
                memcpy(&s, buf, 2);
                d = s;
                break;
            case 4:
                memcpy(&i, buf, 4);
                d = i;
                break;
            case 8:
                memcpy(&d, buf, 8);
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
        if ((s1.pdrf_ == 0 || s1.pdrf_ == 1) && (i == Stats::Red || i == Stats::Green || i == Stats::Blue))
            continue;
        if ((s1.pdrf_ == 0 || s1.pdrf_ == 2) && i == Stats::GpsTime)
            continue;
        const std::string& dim = Stats::dimNames_[i];
        test(s1.summary_[i].minimum(), s2.summary_[i].minimum(), 1E-7, "minimum", dim);
        test(s1.summary_[i].maximum(), s2.summary_[i].maximum(), 1E-7, "maximum", dim);
        test(s1.summary_[i].average(), s2.summary_[i].average(), 1E-5, "average", dim);
        test(s1.summary_[i].variance(), s2.summary_[i].variance(), .1, "variance", dim);
    }
}


void verifyStats(const std::filesystem::path& file1, const std::filesystem::path& file2)
{
    lazperf::reader::named_file f1(file1.generic_string());
    lazperf::reader::named_file f2(file2.generic_string());

    size_t pc1 = f1.header().point_count;
    size_t pc2 = f2.header().point_count;
    ASSERT_EQ(pc1, pc2);

    auto ebSize = [](const lazperf::eb_vlr& vlr, size_t pos) -> int
    {
        // This is a list of the sizes of the LAS data types.
        static const std::array<int, 10> sizes { 1, 1, 2, 2, 4, 4, 8, 8, 4, 8 };

        if (pos >= vlr.items.size())
            return 0;

        int type = vlr.items[pos].data_type;
        if (type < 1 || type > 10)
            return 0;
        return sizes[--type];
    };

    lazperf::eb_vlr vlr = f1.ebVlr();
    Stats s1(f1.header().point_format_id, ebSize(vlr, 0), ebSize(vlr, 1), ebSize(vlr, 2));

    vlr = f2.ebVlr();
    Stats s2(f2.header().point_format_id, ebSize(vlr, 0), ebSize(vlr, 1), ebSize(vlr, 2));

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

    runUntwine(bigfile(), outfile());
//    verify(outfile());
    verifyStats(bigfile(), outfile());

    std::filesystem::remove(outfile());
}

} // namespace test
} // namespace untwine
