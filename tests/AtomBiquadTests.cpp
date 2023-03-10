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
