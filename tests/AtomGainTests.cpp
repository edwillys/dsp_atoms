#include "gtest/gtest.h"
#include "AtomGain.h"
#include <iostream>
#include <vector>
#include <fstream>
#include <filesystem>
#include "TestUtils.h"

namespace fs = std::filesystem;

//=============================================================
// Helper functions
//=============================================================

class AtomGain : public ::testing::Test
{

protected:
    float32_t **m_In = nullptr;
    float32_t **m_Out = nullptr;
    int32_t m_NumCh = 0;
    int32_t m_NumEl = 0;
    int32_t m_Blocksize = 64;
    int32_t m_Fs = 48000;
    CAtomGain m_Gain;
    fs::path m_PathOut;
    fs::path m_PathRef;
    fs::path m_BaseDir;

    AtomGain()
    {
    }

    ~AtomGain() override
    {
    }

    void MySetUp(cint32_t nch, cint32_t nel, std::string ext = ".wav")
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
        m_Gain.init(props);

        auto test_info = ::testing::UnitTest::GetInstance()->current_test_info();
        m_BaseDir = fs::absolute(__FILE__).parent_path();
        auto fpath_base = std::string(test_info->test_suite_name()) + "_" +
                          std::string(test_info->name()) + ext;
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

//=============================================================
// Test cases
//=============================================================

TEST_F(AtomGain, Multisine_Gain_Morph_Mono)
{
    MySetUp(1, 1);

    auto path_in = m_BaseDir / "in" / "Multisine_100_1k_10k_3s.wav";
    auto wav_in = read_wav(path_in.string());
    cint32_t niter = ((cint32_t)wav_in[0].size() + m_Blocksize - 1) / m_Blocksize;
    std::vector<float32_t> wav_out(niter * m_Blocksize);

    m_Gain.setMorphMs(500.0F);
    m_Gain.set(0, 0, -10.0F);

    for (auto n = 0; n < niter; n++)
    {
        if (n == niter / 3)
        {
            m_Gain.set(0, 0, 6.0F);
        }
        else if (n == 2 * niter / 3)
        {
            m_Gain.set(0, 0, -140.0F);
        }

        for (auto i = 0; i < m_Blocksize; i++)
            m_In[0][i] = wav_in[0][m_Blocksize * n + i];
        m_Gain.play(m_In, m_Out);
        for (auto i = 0; i < m_Blocksize; i++)
            wav_out[m_Blocksize * n + i] = m_Out[0][i];
    }
    write_wav(m_PathOut.string(), wav_out);
    ASSERT_EQ(true, compare_wav(m_PathOut.string(), m_PathRef.string()));
}

TEST_F(AtomGain, Multisine_Gain_Morph_Stereo)
{
    MySetUp(2, 0);

    auto path_in = m_BaseDir / "in" / "Multisine_100_1k_10k_3s_2ch.wav";
    auto wavIn = read_wav(path_in.string());
    cint32_t niter = ((cint32_t)wavIn[0].size() + m_Blocksize - 1) / m_Blocksize;
    std::vector<std::vector<float32_t>> wavOut(2);
    wavOut[0].resize(niter * m_Blocksize);
    wavOut[1].resize(niter * m_Blocksize);

    m_Gain.setMorphMs(100.0F);
    m_Gain.set(SET_ALL_CH_IND, 0, -10.0F);

    for (auto n = 0; n < niter; n++)
    {
        if (n == niter / 4)
        {
            m_Gain.set(0, 0, 3.0F);
        }
        else if (n == niter / 2)
        {
            m_Gain.set(1, 0, 3.0F);
        }
        else if (n == 3 * niter / 4)
        {
            m_Gain.set(SET_ALL_CH_IND, 0, -140.0F);
        }

        for (auto i = 0; i < m_Blocksize; i++)
        {
            m_In[0][i] = wavIn[0][m_Blocksize * n + i];
            m_In[1][i] = wavIn[1][m_Blocksize * n + i];
        }
        m_Gain.play(m_In, m_Out);
        for (auto i = 0; i < m_Blocksize; i++)
        {
            wavOut[0][m_Blocksize * n + i] = m_Out[0][i];
            wavOut[1][m_Blocksize * n + i] = m_Out[1][i];
        }
    }
    write_wav(m_PathOut.string(), wavOut);
    ASSERT_EQ(true, compare_wav(m_PathOut.string(), m_PathRef.string()));
}

TEST_F(AtomGain, Multisine_Gain_NoMorph_Stereo)
{
    MySetUp(2, 0);

    auto path_in = m_BaseDir / "in" / "Multisine_100_1k_10k_3s_2ch.wav";
    auto wavIn = read_wav(path_in.string());
    cint32_t niter = ((cint32_t)wavIn[0].size() + m_Blocksize - 1) / m_Blocksize;
    std::vector<std::vector<float32_t>> wavOut(2);
    wavOut[0].resize(niter * m_Blocksize);
    wavOut[1].resize(niter * m_Blocksize);

    m_Gain.setMorphMs(0.0F);
    m_Gain.set(SET_ALL_CH_IND, 0, -10.0F);

    for (auto n = 0; n < niter; n++)
    {
        if (n == niter / 4)
        {
            m_Gain.set(0, 0, 3.0F);
        }
        else if (n == niter / 2)
        {
            m_Gain.set(1, 0, 3.0F);
        }
        else if (n == 3 * niter / 4)
        {
            m_Gain.set(SET_ALL_CH_IND, 0, -140.0F);
        }

        for (auto i = 0; i < m_Blocksize; i++)
        {
            m_In[0][i] = wavIn[0][m_Blocksize * n + i];
            m_In[1][i] = wavIn[1][m_Blocksize * n + i];
        }
        m_Gain.play(m_In, m_Out);
        for (auto i = 0; i < m_Blocksize; i++)
        {
            wavOut[0][m_Blocksize * n + i] = m_Out[0][i];
            wavOut[1][m_Blocksize * n + i] = m_Out[1][i];
        }
    }
    write_wav(m_PathOut.string(), wavOut);
    ASSERT_EQ(true, compare_wav(m_PathOut.string(), m_PathRef.string()));
}