#pragma once

#include <mutex>

#include "../untwine/Common.hpp"

namespace untwine
{

class ProgressWriter
{
public:
    static const PointCount ChunkSize = 100'000;

    ProgressWriter();
    ProgressWriter(int fd, bool debug);

    // Set the progress file descriptor.
    void setFd(int fd);

    /// Set the increment to use on the next call to setIncrement.
    void setIncrement(double increment);
    /// Set the absolute percentage to use for the next setIncrement.
    void setPercent(double percent);

    /// Write a message using the current increment.
    void writeIncrement(const std::string& message);
    /// Write an error message.
    void writeErrorMessage(const std::string& message);
    /// Write a message and set the current percentage.
    void write(double percent, const std::string& message);

    // Point handling support.
    void setPointIncrementer(PointCount total, int totalClicks);
    void update(PointCount numProcessed = ChunkSize);

private:
    std::mutex m_mutex;
    int m_fd;
    bool m_debug;
    double m_percent; // Current percent.
    double m_increment; // Current increment.

    PointCount m_pointIncrement;
    PointCount m_nextClick;
    PointCount m_current;

    void writeMessage(uint32_t percent, const std::string& message);
};

} //namespace untwine
