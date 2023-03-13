#include "gtest/gtest.h"
#include "AtomBiquad.h"
#include <iostream>
#include <vector>
#include <fstream>
#include <filesystem>
#include "TestUtils.h"

namespace fs = std::filesystem;

//=============================================================
// Helper functions
//=============================================================

class AtomBiquad : public ::testing::Test
{

protected:
    float32_t **m_In = nullptr;
    float32_t **m_Out = nullptr;
    int32_t m_NumCh = 0;
    int32_t m_NumEl = 0;
    int32_t m_Blocksize = 64;
    int32_t m_Fs = 48000;
    CAtomBiquad m_Biquad;
    fs::path m_PathOut;
    fs::path m_PathRef;
    fs::path m_BaseDir;

    AtomBiquad()
    {
    }

    ~AtomBiquad() override
    {
    }

    void MySetUp(cint32_t nch, cint32_t nel, std::string ext = ".txt")
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
        m_Biquad.init(props);

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

    void testDirac(
        const std::vector<CAtomBiquad::tAtomBiquadParams> &params,
        cint32_t nch,
        cint32_t nel,
        cfloat32_t eps = 4.E-5,
        bool_t verbose = false)
    {
        MySetUp(nch, nel);

        cint32_t size = 65536;
        cint32_t niter = size / m_Blocksize;
        std::ofstream ofs(m_PathOut);
        ofs << std::fixed << std::setprecision(8);

        for (auto param : params)
        {
            m_Biquad.set(&param, sizeof(CAtomBiquad::tAtomBiquadParams));
        }

        for (auto ch = 0; ch < m_NumCh; ch++)
        {
            // dirac
            m_In[ch][0] = static_cast<float32_t>(size) / 2.0F;
            for (auto i = 1; i < m_Blocksize; i++)
                m_In[ch][i] = 0.0F;
        }
        m_Biquad.play(m_In, m_Out);

        if (m_NumCh > 0)
        {
            for (auto sample = 0; sample < m_Blocksize; sample++)
            {
                ofs << m_Out[0][sample];
                for (auto ch = 1; ch < m_NumCh; ch++)
                {
                    ofs << "," << m_Out[ch][sample];
                }
                ofs << std::endl;
            }
        }

        for (auto ch = 0; ch < m_NumCh; ch++)
        {
            // all zeros
            for (auto i = 0; i < m_Blocksize; i++)
                m_In[ch][i] = 0.0F;
        }

        for (auto i = 1; i < niter; i++)
        {
            m_Biquad.play(m_In, m_Out);
            if (m_NumCh > 0)
            {
                for (auto sample = 0; sample < m_Blocksize; sample++)
                {
                    ofs << m_Out[0][sample];
                    for (auto ch = 1; ch < m_NumCh; ch++)
                    {
                        ofs << "," << m_Out[ch][sample];
                    }
                    ofs << std::endl;
                }
            }
        }

        ASSERT_EQ(true, compare_csv(m_PathOut.string(), m_PathRef.string()));
    }
};

//=============================================================
// Test cases
//=============================================================

TEST_F(AtomBiquad, Bypass_IR)
{
    testDirac(
        {{
            0,                                     // ch;
            0,                                     // el;
            CAtomBiquad::eBiquadType::BIQT_BYPASS, // type
            1000.0F,                               // freq
            0.707F,                                // q
            0.0                                    // gainDb
        }},
        1,
        1);
}

TEST_F(AtomBiquad, LPF_500Hz_0707q_0dB_IR)
{
    testDirac(
        {{
            0,                                  // ch;
            0,                                  // el;
            CAtomBiquad::eBiquadType::BIQT_LPF, // type
            500.0F,                             // freq
            0.707F,                             // q
            0.0                                 // gainDb
        }},
        1,
        1);
}

TEST_F(AtomBiquad, LPF6DB_500Hz_0707q_0dB_IR)
{
    testDirac(
        {{
            0,                                      // ch;
            0,                                      // el;
            CAtomBiquad::eBiquadType::BIQT_LPF_6DB, // type
            500.0F,                                 // freq
            0.707F,                                 // q
            0.0                                     // gainDb
        }},
        1,
        1);
}

TEST_F(AtomBiquad, HPF_500Hz_0707q_0dB_IR)
{
    testDirac(
        {{
            0,                                  // ch;
            0,                                  // el;
            CAtomBiquad::eBiquadType::BIQT_HPF, // type
            500.0F,                             // freq
            0.707F,                             // q
            0.0                                 // gainDb
        }},
        1,
        1);
}

TEST_F(AtomBiquad, HPF6DB_500Hz_0707q_0dB_IR)
{
    testDirac(
        {{
            0,                                      // ch;
            0,                                      // el;
            CAtomBiquad::eBiquadType::BIQT_HPF_6DB, // type
            500.0F,                                 // freq
            0.707F,                                 // q
            0.0                                     // gainDb
        }},
        1,
        1);
}

TEST_F(AtomBiquad, PEAK_100Hz_5000q_10dB_8kHz_10000q_m10dB_IR)
{
    testDirac(
        {{
             0,                                   // ch;
             0,                                   // el;
             CAtomBiquad::eBiquadType::BIQT_PEAK, // type
             100.0F,                              // freq
             5.000F,                              // q
             10.0                                 // gainDb
         },
         {
             0,                                   // ch;
             1,                                   // el;
             CAtomBiquad::eBiquadType::BIQT_PEAK, // type
             8000.0F,                             // freq
             10.000F,                             // q
             -10.0                                // gainDb
         }},
        1,
        2);
}

TEST_F(AtomBiquad, NOTCH_100Hz_1000q_10dB_8kHz_10000q_m10dB_IR)
{
    testDirac(
        {{
             0,                                    // ch;
             0,                                    // el;
             CAtomBiquad::eBiquadType::BIQT_NOTCH, // type
             100.0F,                               // freq
             1.000F,                               // q
             10.0                                  // gainDb
         },
         {
             0,                                    // ch;
             1,                                    // el;
             CAtomBiquad::eBiquadType::BIQT_NOTCH, // type
             8000.0F,                              // freq
             10.000F,                              // q
             -10.0                                 // gainDb
         }},
        1,
        2);
}

TEST_F(AtomBiquad, LSH_8kHz_1500q_3dB_HSH_300Hz_5000q_m10dB_IR)
{
    testDirac(
        {{
             0,                                  // ch;
             0,                                  // el;
             CAtomBiquad::eBiquadType::BIQT_LSH, // type
             8000.0F,                            // freq
             1.500F,                             // q
             3.0                                 // gainDb
         },
         {
             0,                                  // ch;
             1,                                  // el;
             CAtomBiquad::eBiquadType::BIQT_HSH, // type
             300.0F,                             // freq
             5.000F,                             // q
             -10.0                               // gainDb
         }},
        1,
        2);
}

TEST_F(AtomBiquad, HSH_8kHz_0707q_3dB_LSH_300Hz_0707q_m10dB_IR)
{
    testDirac(
        {{
             0,                                  // ch;
             0,                                  // el;
             CAtomBiquad::eBiquadType::BIQT_HSH, // type
             8000.0F,                            // freq
             0.707F,                             // q
             3.0                                 // gainDb
         },
         {
             0,                                  // ch;
             1,                                  // el;
             CAtomBiquad::eBiquadType::BIQT_LSH, // type
             300.0F,                             // freq
             0.707F,                             // q
             -10.0                               // gainDb
         }},
        1,
        2);
}

TEST_F(AtomBiquad, HSH_8kHz_0707q_3dB_LSH_300Hz_0707q_m10dB_2ch_IR)
{
    testDirac(
        {{
             0,                                  // ch;
             0,                                  // el;
             CAtomBiquad::eBiquadType::BIQT_HSH, // type
             8000.0F,                            // freq
             0.707F,                             // q
             3.0                                 // gainDb
         },
         {
             1,                                  // ch;
             0,                                  // el;
             CAtomBiquad::eBiquadType::BIQT_LSH, // type
             300.0F,                             // freq
             0.707F,                             // q
             -10.0                               // gainDb
         }},
        2,
        1);
}

TEST_F(AtomBiquad, LPF_8kHz_1500q_3dB_HPF_300Hz_5000q_m10dB_IR)
{
    testDirac(
        {{
             0,                                  // ch;
             0,                                  // el;
             CAtomBiquad::eBiquadType::BIQT_LPF, // type
             8000.0F,                            // freq
             1.500F,                             // q
             3.0                                 // gainDb
         },
         {
             0,                                  // ch;
             1,                                  // el;
             CAtomBiquad::eBiquadType::BIQT_HPF, // type
             300.0F,                             // freq
             5.000F,                             // q
             -10.0                               // gainDb
         }},
        1,
        2);
}

TEST_F(AtomBiquad, APF_500Hz_0707q_0dB_IR)
{
    testDirac(
        {{
            0,                                  // ch;
            0,                                  // el;
            CAtomBiquad::eBiquadType::BIQT_APF, // type
            500.0F,                             // freq
            0.707F,                             // q
            0.0                                 // gainDb
        }},
        1,
        1);
}

#if 0
TEST_F(AtomBiquad, APF180_500Hz_0707q_0dB_IR)
{
    testDirac(
        {{
            0,                                      // ch;
            0,                                      // el;
            CAtomBiquad::eBiquadType::BIQT_APF_180, // type
            500.0F,                                 // freq
            0.707F,                                 // q
            0.0                                     // gainDb
        }},
        1,
        1);
}
#endif

TEST_F(AtomBiquad, Multisine_PEAK_Gain_Morph)
{
    MySetUp(1, 3, ".wav");

    auto path_in = m_BaseDir / "in" / "Multisine_100_1k_10k_3s.wav";
    auto wav_in = read_wav(path_in.string());
    cint32_t niter = ((cint32_t)wav_in[0].size() + m_Blocksize - 1) / m_Blocksize;
    std::vector<float32_t> wav_out(niter * m_Blocksize);

    m_Biquad.setMorphMs(500.0F);

    std::vector<CAtomBiquad::tAtomBiquadParams> params =
        {{
             0,                                   // ch;
             0,                                   // el;
             CAtomBiquad::eBiquadType::BIQT_PEAK, // type
             100.0F,                              // freq
             5.000F,                              // q
             6.0                                  // gainDb
         },
         {
             0,                                   // ch;
             1,                                   // el;
             CAtomBiquad::eBiquadType::BIQT_PEAK, // type
             1000.0F,                             // freq
             5.000F,                              // q
             -60.0                                // gainDb
         },
         {
             0,                                   // ch;
             2,                                   // el;
             CAtomBiquad::eBiquadType::BIQT_PEAK, // type
             10000.0F,                            // freq
             5.000F,                              // q
             -60.0                                // gainDb
         }};

    for (auto param : params)
    {
        m_Biquad.set(&param, sizeof(CAtomBiquad::tAtomBiquadParams));
    }

    for (auto n = 0; n < niter; n++)
    {
        if (n == niter / 3)
        {
            params[0].gainDb = -60.0;
            params[1].gainDb = 6.0;
            m_Biquad.set(&params[0], sizeof(CAtomBiquad::tAtomBiquadParams));
            m_Biquad.set(&params[1], sizeof(CAtomBiquad::tAtomBiquadParams));
        }
        else if (n == 2 * niter / 3)
        {
            params[1].gainDb = -60.0;
            params[2].gainDb = 6.0;
            m_Biquad.set(&params[1], sizeof(CAtomBiquad::tAtomBiquadParams));
            m_Biquad.set(&params[2], sizeof(CAtomBiquad::tAtomBiquadParams));
        }

        for (auto i = 0; i < m_Blocksize; i++)
            m_In[0][i] = wav_in[0][m_Blocksize * n + i];
        m_Biquad.play(m_In, m_Out);
        for (auto i = 0; i < m_Blocksize; i++)
            wav_out[m_Blocksize * n + i] = m_Out[0][i];
    }
    write_wav(m_PathOut.string(), wav_out);
    ASSERT_EQ(true, compare_wav(m_PathOut.string(), m_PathRef.string()));
}