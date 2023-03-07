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
    std::string path_out,
    const std::string &path_ref,
    cfloat32_t eps = 4.E-5,
    bool_t verbose = false)
{
    ASSERT_EQ(true, path_out.size() > 0);

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
        in[ch][0] = 1.0F; // dirac
        biquad.play(in, out);
        for (auto sample = 0; sample < bs; sample++)
        {
            for (auto ch = 0; ch < nch; ch++)
            {
                ofs << out[ch][sample] << ",";
            }
            ofs << std::endl;
        }

        in[ch][0] = 0.0F;
        for (auto i = 0; i < niter - 1; i++)
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
    }

    ASSERT_EQ(true, fs::exists(path_ref));

    for (auto ch = 0; ch < nch; ch++)
    {
        delete[] in[ch];
        delete[] out[ch];
    }
    delete in;
    delete out;

    // BOOST_REQUIRE_LE(abs(buf_ref[j] - buf[j]), eps);
}

//=============================================================
// Test cases
//=============================================================

TEST(Biquad, Bypass)
{
    cint32_t bs = 64;

    auto path_out = fs::path(__FILE__).parent_path() / fs::path("out/biquad_bypass_ir.txt");
    auto path_ref = fs::path(__FILE__).parent_path() / fs::path("ref/biquad_bypass_ir.txt");

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
        path_out.string(),
        path_ref.string());
}

TEST(Biquad, LPF_500Hz_0707q_0dB)
{
    cint32_t bs = 64;

    auto path_out = fs::path(__FILE__).parent_path() / fs::path("out/biquad_lpf_500Hz_0707q_0dB_ir.txt");
    auto path_ref = fs::path(__FILE__).parent_path() / fs::path("ref/biquad_lpf_500Hz_0707q_0dB_ir.txt");

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
        path_out.string(),
        path_ref.string());
}
