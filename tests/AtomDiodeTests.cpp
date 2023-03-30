#include "gtest/gtest.h"
#include "AtomDiode.h"
#include <iostream>
#include <vector>
#include <fstream>
#include "TestUtils.h"

//=============================================================
// Helper functions
//=============================================================

struct AtomDiodeTestParams
{
    float32_t gain;
    float32_t morphTime;
    float32_t eps;
    int32_t nch;
};

class AtomDiode : public AtomTest<CAtomDiode>,
                  public testing::WithParamInterface<AtomDiodeTestParams>
{
};

//=============================================================
// Test cases
//=============================================================

TEST_P(AtomDiode, Triangle_1Hz_1s_0dB)
{
    auto params = GetParam();

    auto test_info = ::testing::UnitTest::GetInstance()->current_test_info();
    auto test_suite_name = std::string(test_info->test_suite_name());
    auto test_name = std::string(test_info->name());

    auto ind = test_suite_name.find_first_of("/") + 1;
    test_suite_name = test_suite_name.substr(ind, test_suite_name.size() - ind);
    ind = test_name.find_first_of("/");
    test_name = test_name.substr(0, ind);
    auto fpath_base = test_suite_name + "_" + test_name;
    fpath_base += "_Morph" + std::to_string(static_cast<int32_t>(params.morphTime));
    fpath_base += "ms_" + std::to_string(params.nch);
    fpath_base += "ch_R" + std::to_string(static_cast<int32_t>(params.gain)) + ".wav";

    MySetUp(params.nch, 1, ".wav", fpath_base);

    auto path_in = m_BaseDir / "in" / "Triangle_1Hz_1s_0dB.wav";
    auto wav_in = read_wav(path_in.string());
    cint32_t niter = ((cint32_t)wav_in[0].size() + m_Blocksize - 1) / m_Blocksize;
    std::vector<float32_t> wav_out(niter * m_Blocksize);

    // Extend and fill with zeros if WAV file is not divisible by m_Blocksize
    for (auto &w : wav_in)
    {
        w.resize(niter * m_Blocksize);
    }

    m_Atom.setMorphMs(params.morphTime);
    m_Atom.set(0, 0, params.gain);

    for (auto n = 0; n < niter; n++)
    {
        for (auto i = 0; i < m_Blocksize; i++)
            m_In[0][i] = wav_in[0][m_Blocksize * n + i];
        m_Atom.play(m_In, m_Out);
        for (auto i = 0; i < m_Blocksize; i++)
            wav_out[m_Blocksize * n + i] = m_Out[0][i];
    }
    write_wav(m_PathOut.string(), wav_out);
    ASSERT_EQ(true, compare_wav(m_PathOut.string(), m_PathRef.string(), params.eps));
}

std::vector<AtomDiodeTestParams> GetTests()
{
    return {
        {0.F, 0.F, 2.F / 32768.F, 1},
        {1.F, 0.F, 1.F / 32768.F, 1},
        {10.F, 0.F, 1.F / 32768.F, 1},
        {1000.F, 0.F, 1.F / 32768.F, 1},
        {10000.F, 0.F, 1.F / 32768.F, 1},
        {100000.F, 0.F, 1.F / 32768.F, 1},
        {1000000.F, 0.F, 1.F / 32768.F, 1},
    };
}

INSTANTIATE_TEST_SUITE_P(AtomDiodeP, AtomDiode, testing::ValuesIn(GetTests()));

#if 0
TEST_F(AtomDiode, Multisine_Diode_Morph_Stereo)
{
    MySetUp(2, 0);

    auto path_in = m_BaseDir / "in" / "Multisine_100_1k_10k_3s_2ch.wav";
    auto wavIn = read_wav(path_in.string());
    cint32_t niter = ((cint32_t)wavIn[0].size() + m_Blocksize - 1) / m_Blocksize;
    std::vector<std::vector<float32_t>> wavOut(2);
    wavOut[0].resize(niter * m_Blocksize);
    wavOut[1].resize(niter * m_Blocksize);

    m_Diode.setMorphMs(100.0F);
    m_Diode.set(SET_ALL_CH_IND, 0, -10.0F);

    for (auto n = 0; n < niter; n++)
    {
        if (n == niter / 4)
        {
            m_Diode.set(0, 0, 3.0F);
        }
        else if (n == niter / 2)
        {
            m_Diode.set(1, 0, 3.0F);
        }
        else if (n == 3 * niter / 4)
        {
            m_Diode.set(SET_ALL_CH_IND, 0, -140.0F);
        }

        for (auto i = 0; i < m_Blocksize; i++)
        {
            m_In[0][i] = wavIn[0][m_Blocksize * n + i];
            m_In[1][i] = wavIn[1][m_Blocksize * n + i];
        }
        m_Diode.play(m_In, m_Out);
        for (auto i = 0; i < m_Blocksize; i++)
        {
            wavOut[0][m_Blocksize * n + i] = m_Out[0][i];
            wavOut[1][m_Blocksize * n + i] = m_Out[1][i];
        }
    }
    write_wav(m_PathOut.string(), wavOut);
    ASSERT_EQ(true, compare_wav(m_PathOut.string(), m_PathRef.string()));
}

TEST_F(AtomDiode, Multisine_Diode_NoMorph_Stereo)
{
    MySetUp(2, 0);

    auto path_in = m_BaseDir / "in" / "Multisine_100_1k_10k_3s_2ch.wav";
    auto wavIn = read_wav(path_in.string());
    cint32_t niter = ((cint32_t)wavIn[0].size() + m_Blocksize - 1) / m_Blocksize;
    std::vector<std::vector<float32_t>> wavOut(2);
    wavOut[0].resize(niter * m_Blocksize);
    wavOut[1].resize(niter * m_Blocksize);

    m_Diode.setMorphMs(0.0F);
    m_Diode.set(SET_ALL_CH_IND, 0, -10.0F);

    for (auto n = 0; n < niter; n++)
    {
        if (n == niter / 4)
        {
            m_Diode.set(0, 0, 3.0F);
        }
        else if (n == niter / 2)
        {
            m_Diode.set(1, 0, 3.0F);
        }
        else if (n == 3 * niter / 4)
        {
            m_Diode.set(SET_ALL_CH_IND, 0, -140.0F);
        }

        for (auto i = 0; i < m_Blocksize; i++)
        {
            m_In[0][i] = wavIn[0][m_Blocksize * n + i];
            m_In[1][i] = wavIn[1][m_Blocksize * n + i];
        }
        m_Diode.play(m_In, m_Out);
        for (auto i = 0; i < m_Blocksize; i++)
        {
            wavOut[0][m_Blocksize * n + i] = m_Out[0][i];
            wavOut[1][m_Blocksize * n + i] = m_Out[1][i];
        }
    }
    write_wav(m_PathOut.string(), wavOut);
    ASSERT_EQ(true, compare_wav(m_PathOut.string(), m_PathRef.string()));
}

#endif