#include "gtest/gtest.h"
#include "AtomBiquad.h"
#include <iostream>
#include <vector>
#include <fstream>
#include <filesystem>
#include "TestUtils.h"

namespace fs = std::filesystem;

//=============================================================
// Defines
//=============================================================

//=============================================================
// Helper functions
//=============================================================
static void helper_test(
    const std::vector<CAtomBiquad::tAtomBiquadParams> &params,
    cint32_t nch,
    cint32_t nel,
    const testing::TestInfo *test_info,
    cfloat32_t eps = 4.E-5,
    bool_t verbose = false)
{
    auto base_dir = fs::absolute(__FILE__).parent_path();
    auto fpath_base = std::string(test_info->test_suite_name()) + "_" +
                      std::string(test_info->name()) + ".txt";
    auto path_out = base_dir / "out" / fpath_base;
    auto path_ref = base_dir / "ref" / fpath_base;

    cint32_t size = 65536;
    cint32_t bs = 64;
    cint32_t fs = 48000;
    cint32_t niter = size / bs;
    std::ofstream ofs(path_out);
    ofs << std::fixed << std::setprecision(8);

    CQuarkProps props{
        fs,
        bs,
        nch,
        nch,
        0,
        0,
        nel};
    CAtomBiquad biquad;
    biquad.init(props);

    for (auto param : params)
    {
        biquad.set(&param, sizeof(CAtomBiquad::tAtomBiquadParams));
    }

    float32_t **in = new float *[nch];
    float32_t **out = new float *[nch];
    for (auto ch = 0; ch < nch; ch++)
    {
        in[ch] = new float32_t[bs]();
        out[ch] = new float32_t[bs]();
    }

    for (auto ch = 0; ch < nch; ch++)
    {
        // dirac
        in[ch][0] = static_cast<float32_t>(size) / 2.0F;
        for (auto i = 1; i < nel; i++)
            in[ch][i] = 0.0F;
    }
    biquad.play(in, out);
    for (auto sample = 0; sample < bs; sample++)
    {
        for (auto ch = 0; ch < nch; ch++)
        {
            ofs << out[ch][sample] << ",";
        }
        ofs << std::endl;
    }

    for (auto ch = 0; ch < nch; ch++)
    {
        // all zeros
        for (auto i = 0; i < nel; i++)
            in[ch][i] = 0.0F;
    }

    for (auto i = 1; i < niter; i++)
    {
        biquad.play(in, out);
        for (auto sample = 0; sample < bs; sample++)
        {
            for (auto ch = 0; ch < nch; ch++)
            {
                ofs << out[ch][sample] << ",";
            }
            ofs << std::endl;
        }
    }

    for (auto ch = 0; ch < nch; ch++)
    {
        delete[] in[ch];
        delete[] out[ch];
    }
    delete in;
    delete out;

    ASSERT_EQ(true, compare_csv(path_out.string(), path_ref.string()));
}

//=============================================================
// Test cases
//=============================================================

TEST(AtomBiquad, Bypass_IR)
{
    helper_test(
        {{
            0,                                     // ch;
            0,                                     // el;
            CAtomBiquad::eBiquadType::BIQT_BYPASS, // type
            1000.0F,                               // freq
            0.707F,                                // q
            0.0                                    // gainDb
        }},
        1,
        1,
        ::testing::UnitTest::GetInstance()->current_test_info());
}

TEST(AtomBiquad, LPF_500Hz_0707q_0dB_IR)
{
    helper_test(
        {{
            0,                                  // ch;
            0,                                  // el;
            CAtomBiquad::eBiquadType::BIQT_LPF, // type
            500.0F,                             // freq
            0.707F,                             // q
            0.0                                 // gainDb
        }},
        1,
        1,
        ::testing::UnitTest::GetInstance()->current_test_info());
}

TEST(AtomBiquad, LPF6DB_500Hz_0707q_0dB_IR)
{
    helper_test(
        {{
            0,                                      // ch;
            0,                                      // el;
            CAtomBiquad::eBiquadType::BIQT_LPF_6DB, // type
            500.0F,                                 // freq
            0.707F,                                 // q
            0.0                                     // gainDb
        }},
        1,
        1,
        ::testing::UnitTest::GetInstance()->current_test_info());
}

TEST(AtomBiquad, HPF_500Hz_0707q_0dB_IR)
{
    helper_test(
        {{
            0,                                  // ch;
            0,                                  // el;
            CAtomBiquad::eBiquadType::BIQT_HPF, // type
            500.0F,                             // freq
            0.707F,                             // q
            0.0                                 // gainDb
        }},
        1,
        1,
        ::testing::UnitTest::GetInstance()->current_test_info());
}

TEST(AtomBiquad, HPF6DB_500Hz_0707q_0dB_IR)
{
    helper_test(
        {{
            0,                                      // ch;
            0,                                      // el;
            CAtomBiquad::eBiquadType::BIQT_HPF_6DB, // type
            500.0F,                                 // freq
            0.707F,                                 // q
            0.0                                     // gainDb
        }},
        1,
        1,
        ::testing::UnitTest::GetInstance()->current_test_info());
}

TEST(AtomBiquad, PEAK_100Hz_5000q_10dB_8kHz_10000q_m10dB_IR)
{
    helper_test(
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
        2,
        ::testing::UnitTest::GetInstance()->current_test_info());
}

TEST(AtomBiquad, NOTCH_100Hz_1000q_10dB_8kHz_10000q_m10dB_IR)
{
    helper_test(
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
        2,
        ::testing::UnitTest::GetInstance()->current_test_info());
}

TEST(AtomBiquad, LSH_8kHz_1500q_3dB_HSH_300Hz_5000q_m10dB_IR)
{
    helper_test(
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
        2,
        ::testing::UnitTest::GetInstance()->current_test_info());
}

TEST(AtomBiquad, HSH_8kHz_0707q_3dB_LSH_300Hz_0707q_m10dB_IR)
{
    helper_test(
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
        2,
        ::testing::UnitTest::GetInstance()->current_test_info());
}

TEST(AtomBiquad, LPF_8kHz_1500q_3dB_HPF_300Hz_5000q_m10dB_IR)
{
    helper_test(
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
        2,
        ::testing::UnitTest::GetInstance()->current_test_info());
}

TEST(AtomBiquad, APF_500Hz_0707q_0dB_IR)
{
    helper_test(
        {{
            0,                                  // ch;
            0,                                  // el;
            CAtomBiquad::eBiquadType::BIQT_APF, // type
            500.0F,                             // freq
            0.707F,                             // q
            0.0                                 // gainDb
        }},
        1,
        1,
        ::testing::UnitTest::GetInstance()->current_test_info());
}

#if 0
TEST(AtomBiquad, APF180_500Hz_0707q_0dB_IR)
{
    helper_test(
        {{
            0,                                      // ch;
            0,                                      // el;
            CAtomBiquad::eBiquadType::BIQT_APF_180, // type
            500.0F,                                 // freq
            0.707F,                                 // q
            0.0                                     // gainDb
        }},
        1,
        1,
        ::testing::UnitTest::GetInstance()->current_test_info());
}
#endif