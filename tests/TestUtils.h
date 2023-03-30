#pragma once

#include <vector>
#include <string>
#include "AudioTypes.h"
#include "gtest/gtest.h"
#include <filesystem>

namespace fs = std::filesystem;

template <class T>
class AtomTest : public ::testing::Test
{

protected:
    float32_t **m_In = nullptr;
    float32_t **m_Out = nullptr;
    int32_t m_NumCh = 0;
    int32_t m_NumEl = 0;
    int32_t m_Blocksize = 64;
    int32_t m_Fs = 48000;
    T m_Atom;
    fs::path m_PathOut;
    fs::path m_PathRef;
    fs::path m_BaseDir;

    AtomTest()
    {
    }

    ~AtomTest() override
    {
    }

    void MySetUp(cint32_t nch, cint32_t nel,
                 std::string ext = ".wav", std::string fpath_base = "")
    {
        m_NumCh = nch;
        m_NumEl = nel;
        m_In = new float *[nch];
        m_Out = new float *[nch];
        for (auto ch = 0; ch < nch; ch++)
        {
            m_In[ch] = new float32_t[m_Blocksize]();
            m_Out[ch] = new float32_t[m_Blocksize]();
        }
        CQuarkProps props{
            m_Fs,
            m_Blocksize,
            m_NumCh,
            m_NumCh,
            0,
            0,
            m_NumEl};
        m_Atom.init(props);

        auto test_info = ::testing::UnitTest::GetInstance()->current_test_info();
        m_BaseDir = fs::absolute(__FILE__).parent_path();
        if (fpath_base.empty())
        {
            fpath_base = std::string(test_info->test_suite_name()) + "_" +
                         std::string(test_info->name()) + ext;
        }
        m_PathOut = m_BaseDir / "out" / fpath_base;
        m_PathRef = m_BaseDir / "ref" / fpath_base;
    }

    void TearDown() override
    {
        for (auto ch = 0; ch < m_NumCh; ch++)
        {
            delete[] m_In[ch];
            delete[] m_Out[ch];
        }
        delete m_In;
        delete m_Out;
    }
};

/**
 * @brief Splits a string according to a delimiter
 *
 * @param s
 * @param delimiter
 * @return std::vector<std::string>
 */
std::vector<std::string> split(const std::string &s, const std::string &delimiter);

/**
 * @brief
 *
 * @param s
 * @param delim
 * @return std::vector<std::string>
 */
std::vector<std::string> split(const std::string &s, const char delim);

/**
 * @brief
 *
 */
std::vector<std::vector<float32_t>> deinterleave(const std::vector<float32_t> &in, cint32_t nch);

/**
 * @brief
 *
 * @param in
 * @return std::vector<float32_t>
 */
std::vector<float32_t> interleave(const std::vector<std::vector<float32_t>> &in);

/**
 * @brief
 *
 * @param outwav
 * @param content
 * @param fs
 * @param bps
 * @param nch
 */
void write_wav(const std::string &outwav, const std::vector<float32_t> &content,
               cint32_t fs = 48000, cint32_t bps = 16, cint32_t nch = 1);

/**
 * @brief
 *
 * @param outwav
 * @param content
 * @param fs
 * @param bps
 */
void write_wav(const std::string &outwav, const std::vector<std::vector<float32_t>> &content,
               cint32_t fs = 48000, cint32_t bps = 16);

/**
 * @brief
 *
 * @param path_wav
 * @return std::vector<std::vector<float32_t>>
 */
std::vector<std::vector<float32_t>> read_wav(const std::string &path_wav);

/**
 * @brief
 *
 * @param wavL
 * @param wavR
 * @param eps
 * @return true
 * @return false
 */
bool compare_wav(const std::string &wavL, const std::string &wavR,
                 float32_t eps = 1.F / 32768); // approx -90dB (1 bit out of 16 for the PCM WAV)

/**
 * @brief
 *
 * @param left
 * @param right
 * @param eps
 * @return true
 * @return false
 */
bool compare_csv(const std::string &left, const std::string &right, float32_t eps = 1e-9);

/**
 * @brief
 *
 * @tparam T
 * @param v
 * @param n
 * @return std::vector<std::vector<T>>
 */
template <typename T>
std::vector<std::vector<T>> split(std::vector<T> &v, int32_t n)
{
    std::vector<std::vector<float32_t>> vec_split;
    cint32_t niter = static_cast<cint32_t>(ceil(v.size() / (float32_t)n));
    v.resize(niter * n);
    for (auto i = 0; i < niter; i++)
    {
        auto first = v.begin() + n * i;
        auto last = v.begin() + n * (i + 1);
        std::vector<float32_t> vec(first, last);
        vec_split.push_back(vec);
    }

    return vec_split;
}

/**
 * @brief
 *
 * @tparam T
 * @param v
 * @param n
 * @return std::vector<std::vector<T>>
 */
template <typename T>
std::vector<std::vector<T>> split(std::vector<T> &v, std::vector<int32_t> n)
{
    std::vector<std::vector<float32_t>> vec_split;
    int32_t sum = 0;
    for (auto it = n.begin(); it != n.end(); it++)
        sum += (*it);
    v.resize((int32_t)ceil(v.size() / (float32_t)sum) * sum);
    int32_t offset = 0, i = 0;
    while (offset < v.size())
    {
        auto len = n[i % n.size()];
        auto first = v.begin() + offset;
        auto last = v.begin() + offset + len;
        std::vector<float32_t> vec(first, last);
        vec_split.push_back(vec);
        offset += len;
        i++;
    }

    return vec_split;
}
