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
                        cint32_t ref_len, float32_t *ref, cfloat32_t eps = 4.E-5, std::string outwav = "", bool_t verbose = false)
{
    std::vector<float> content; // in case we are writing output WAV
    bool_t writeout = outwav.size() > 0;

    BOOST_REQUIRE_EQUAL(true, fs::exists(path));

    CSampleParams params;
    params.append = false;
    params.path = path;
    CSample sample(params, up, down);

    float32_t *buf = new float32_t[bs];

    int32_t ref_ind = 0, cnt = 0;
    sample.on();
    while (sample.getNumSamplesUntilFinished() > 0)
    {
        sample.play(buf, bs);
        auto n_iter = std::min(ref_len - ref_ind, bs);

        for (auto j = 0; j < n_iter; j++)
        {
            if (verbose)
                std::cout << "testing index=" << ref_ind << " " << ref[ref_ind] << " ?= " << buf[j] << std::endl;
            BOOST_REQUIRE_LE(abs(ref[ref_ind] - buf[j]), eps);

            ref_ind++;
        }

        if (writeout)
            content.insert(content.end(), buf, buf + n_iter);
    }

    // write output to wav
    if (writeout)
        write_wav(outwav, content);

    delete buf;
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
        NA4v16unisson::samples_per_ch,
        NA4v16unisson::samples[0]);
}

BOOST_AUTO_TEST_CASE(ResamplerFifthsBlocksize96)
{
    cint32_t bs = 96;
    auto path = fs::path(__FILE__).parent_path() / fs::path("in/A4v16.wav");

    helper_test(
        path.string(),
        2,
        3,
        bs,
        NA4v16up5th::samples_per_ch,
        NA4v16up5th::samples[0]);

    helper_test(
        path.string(),
        3,
        2,
        bs,
        NA4v16down5th::samples_per_ch,
        NA4v16down5th::samples[0]);
}

BOOST_AUTO_TEST_CASE(ResamplerFourthsBlocksize96)
{
    cint32_t bs = 96;
    auto path = fs::path(__FILE__).parent_path() / fs::path("in/A4v16.wav");

    helper_test(
        path.string(),
        3,
        4,
        bs,
        NA4v16up4th::samples_per_ch,
        NA4v16up4th::samples[0]);

    helper_test(
        path.string(),
        4,
        3,
        bs,
        NA4v16down4th::samples_per_ch,
        NA4v16down4th::samples[0]);
}

BOOST_AUTO_TEST_CASE(ResamplerFourthsBlocksize64)
{
    cint32_t bs = 64;
    auto path = fs::path(__FILE__).parent_path() / fs::path("in/A4v16.wav");

    helper_test(
        path.string(),
        3,
        4,
        bs,
        NA4v16up4th::samples_per_ch,
        NA4v16up4th::samples[0]);

    helper_test(
        path.string(),
        4,
        3,
        bs,
        NA4v16down4th::samples_per_ch,
        NA4v16down4th::samples[0]);
}

BOOST_AUTO_TEST_CASE(ResamplerFifthsBlocksize64)
{
    cint32_t bs = 64;
    auto path = fs::path(__FILE__).parent_path() / fs::path("in/A4v16.wav");

    helper_test(
        path.string(),
        2,
        3,
        bs,
        NA4v16up5th::samples_per_ch,
        NA4v16up5th::samples[0]);

    helper_test(
        path.string(),
        3,
        2,
        bs,
        NA4v16down5th::samples_per_ch,
        NA4v16down5th::samples[0]);
}

BOOST_AUTO_TEST_SUITE_END()

#endif // RESAMPLER_TESTS
