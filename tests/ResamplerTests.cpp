#ifndef RESAMPLER_TESTS
#define RESAMPLER_TESTS
#include <boost/test/unit_test.hpp>
#include "Resampler.h"
#include <iostream>
#include <vector>
#include <algorithm>
#include <filesystem>
#include "AudioTypes.h"
#include "Sample.h"
#include "TestUtils.h"
#include <sndfile.h>

namespace fs = std::filesystem;
//=============================================================
// Reference audio files
//=============================================================
namespace NA4v16up5th
{
#include "ref/A4v16_up5th.h"
}
namespace NA4v16down5th
{
#include "ref/A4v16_down5th.h"
}
namespace NA4v16up4th
{
#include "ref/A4v16_up4th.h"
}
namespace NA4v16down4th
{
#include "ref/A4v16_down4th.h"
}
namespace NA4v16unisson
{
#include "ref/A4v16_unisson.h"
}

//=============================================================
// Defines
//=============================================================

BOOST_AUTO_TEST_SUITE(ResamplerTests)

//=============================================================
// Helper functions
//=============================================================
static void helper_test(const std::string &path, cint32_t up, cint32_t down, cint32_t bs,
                        const std::string &path_ref, std::string outwav = "", cfloat32_t eps = 4.E-5, bool_t verbose = false)
{
    bool_t writeout = outwav.size() > 0;

    BOOST_REQUIRE_EQUAL(true, fs::exists(path));

    CSampleParams params;
    params.append = false;
    params.path = path;
    CSample sample(params, up, down);

    std::vector<float32_t> buf;
    buf.resize(bs);

    // write output to wav
    if (writeout)
    {
        std::vector<float> content; // in case we are writing output WAV
        sample.on();
        while (sample.getNumSamplesUntilFinished() > 0)
        {
            sample.play(&buf[0], bs);

            content.insert(content.end(), buf.begin(), buf.end());
        }
        sample.off();
        write_wav(outwav, content);
    }

    BOOST_REQUIRE_EQUAL(true, fs::exists(path_ref));

    SF_INFO info;
    SNDFILE* const sndfile = sf_open(path_ref.c_str(), SFM_READ, &info);
    if (!sndfile || !info.frames || (info.channels != 1) || info.samplerate != params.fs) 
    {
        std::cerr << "Failed to open sample " << params.path << std::endl;
        sf_close(sndfile);
    }
    else
    {
        // Read data
        std::vector<float32_t> buf_ref;
        buf_ref.resize(bs);

        sf_seek(sndfile, 0ul, SEEK_SET);
        
        int32_t ref_ind = 0, cnt = 0;
        sample.on();
        while (sample.getNumSamplesUntilFinished() > 0)
        {
            sample.play(&buf[0], bs);
            sf_read_float(sndfile, &buf_ref[0], bs);
            auto n_iter = std::min((int32_t)(info.frames - ref_ind), bs);

            for (auto j = 0; j < n_iter; j++)
            {
                if (verbose)
                    std::cout << "testing index=" << ref_ind + j << " " << buf_ref[j] << " ?= " << buf[j] << std::endl;
                BOOST_REQUIRE_LE(abs(buf_ref[j] - buf[j]), eps);
            }
            ref_ind += bs;
        }
        
        sf_close(sndfile);
    }
}

//=============================================================
// Test cases
//=============================================================

BOOST_AUTO_TEST_CASE(ResamplerUnisson)
{
    cint32_t bs = 64;

    auto path = fs::path(__FILE__).parent_path() / fs::path("in/A4v16.wav");

    helper_test(
        path.string(),
        1,
        1,
        bs,
        path.string());
}

BOOST_AUTO_TEST_CASE(ResamplerFifthsBlocksize96)
{
    cint32_t bs = 96;
    auto path = fs::path(__FILE__).parent_path() / fs::path("in/A4v16.wav");
    auto ref_path_5thup = fs::path(__FILE__).parent_path() / "ref/A4v16_up5th.wav";
    auto ref_path_5thdown = fs::path(__FILE__).parent_path() / "ref/A4v16_down5th.wav";

    helper_test(
        path.string(),
        2,
        3,
        bs,
        ref_path_5thup.string());

    helper_test(
        path.string(),
        3,
        2,
        bs,
        ref_path_5thdown.string());
}

BOOST_AUTO_TEST_CASE(ResamplerFourthsBlocksize96)
{
    cint32_t bs = 96;
    auto path = fs::path(__FILE__).parent_path() / fs::path("in/A4v16.wav");
    auto ref_path_4thup = fs::path(__FILE__).parent_path() / "ref/A4v16_up4th.wav";
    auto ref_path_4thdown = fs::path(__FILE__).parent_path() / "ref/A4v16_down4th.wav";

    helper_test(
        path.string(),
        3,
        4,
        bs,
        ref_path_4thup.string());

    helper_test(
        path.string(),
        4,
        3,
        bs,
        ref_path_4thdown.string());
}

BOOST_AUTO_TEST_CASE(ResamplerFourthsBlocksize64)
{
    cint32_t bs = 64;
    auto path = fs::path(__FILE__).parent_path() / fs::path("in/A4v16.wav");
    auto ref_path_4thup = fs::path(__FILE__).parent_path() / "ref/A4v16_up4th.wav";
    auto ref_path_4thdown = fs::path(__FILE__).parent_path() / "ref/A4v16_down4th.wav";

    helper_test(
        path.string(),
        3,
        4,
        bs,
        ref_path_4thup.string());

    helper_test(
        path.string(),
        4,
        3,
        bs,
        ref_path_4thdown.string());
}

BOOST_AUTO_TEST_CASE(ResamplerFifthsBlocksize64)
{
    cint32_t bs = 64;
    auto path = fs::path(__FILE__).parent_path() / fs::path("in/A4v16.wav");
    auto ref_path_5thup = fs::path(__FILE__).parent_path() / "ref/A4v16_up5th.wav";
    auto ref_path_5thdown = fs::path(__FILE__).parent_path() / "ref/A4v16_down5th.wav";

    helper_test(
        path.string(),
        2,
        3,
        bs,
        ref_path_5thup.string());

    helper_test(
        path.string(),
        3,
        2,
        bs,
        ref_path_5thdown.string());
}

BOOST_AUTO_TEST_SUITE_END()

#endif // RESAMPLER_TESTS
