/******************************************************************************
* Copyright (c) 2011, Michael P. Gerlek (mpg@flaxen.com)
*
* All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following
* conditions are met:
*
*     * Redistributions of source code must retain the above copyright
*       notice, this list of conditions and the following disclaimer.
*     * Redistributions in binary form must reproduce the above copyright
*       notice, this list of conditions and the following disclaimer in
*       the documentation and/or other materials provided
*       with the distribution.
*     * Neither the name of Hobu, Inc. or Flaxen Geo Consulting nor the
*       names of its contributors may be used to endorse or promote
*       products derived from this software without specific prior
*       written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
* "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
* LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
* FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
* COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
* INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
* BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
* OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
* AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
* OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
* OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
* OF SUCH DAMAGE.
****************************************************************************/

#pragma once

#include <cstdint>

#include <pdal/Filter.hpp>
#include <pdal/Streamable.hpp>

namespace untwine
{
namespace test
{

class Summary
{
public:
    Summary(const std::string& name) : m_name(name)
    { reset(); }
    Summary()
    { reset(); }

    // Merge another summary with this one. 'name', 'enumerate' and 'advanced' must match
    // or false is returned and no merge occurs.
    bool merge(const Summary& s);
    double minimum() const
        { return m_min; }
    double maximum() const
        { return m_max; }
    double average() const
        { return M1; }
    double populationVariance() const
        { return M2 / m_cnt; }
    double sampleVariance() const
        { return M2 / (m_cnt - 1.0); }
    double variance() const
        { return sampleVariance(); }
    double populationStddev() const
        { return std::sqrt(populationVariance()); }
    double sampleStddev() const
        { return std::sqrt(sampleVariance()); }
    double stddev() const
        { return sampleStddev(); }
    double median() const
        { return m_median; }
    uint64_t count() const
        { return m_cnt; }
    std::string name() const
        { return m_name; }

    void computeGlobalStats();

    void reset()
    {
        m_max = (std::numeric_limits<double>::lowest)();
        m_min = (std::numeric_limits<double>::max)();
        m_cnt = 0;
        m_median = 0.0;
        M1 = M2 = 0.0;
    }

    void insert(double value)
    {
        m_cnt++;
        m_min = (std::min)(m_min, value);
        m_max = (std::max)(m_max, value);

        // stolen from http://www.johndcook.com/blog/skewness_kurtosis/
        uint64_t n(m_cnt);

        // Difference from the mean
        double delta = value - M1;
        // Portion that this point's difference from the mean contributes
        // to the mean.
        double delta_n = delta / n;
        double term1 = delta * delta_n * (n - 1);

        // First moment - average.
        M1 += delta_n;

        // Second moment - variance (sum part)
        M2 += term1;
    }

private:
    std::string m_name;
    double m_max;
    double m_min;
    double m_median;
    uint64_t m_cnt;
    double M1, M2;
};

} // namespace test
} // namespace untwine
