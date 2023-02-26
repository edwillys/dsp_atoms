#ifndef UPFIRDOWN_TESTS
#define UPFIRDOWN_TESTS
#include <boost/test/unit_test.hpp>
#include "UpFirDown.h"
#include <iostream>
#include <vector>
#include <algorithm>
#include <math.h>
#include "AudioTypes.h"
#include "TestUtils.h"

//=============================================================
// Reference data
//=============================================================
namespace TestFirCoeffs{
    #include "ResampleFIRCoeffs.h"
}

namespace SquareDown5th{
    #include "ref/square1k_at_48k_down5th.h"
}

namespace SquareUp5th{
    #include "ref/square1k_at_48k_up5th.h"
}

namespace Square{
    #include "ref/square1k_at_48k.h"
}

//=============================================================
// Defines
//=============================================================
BOOST_AUTO_TEST_SUITE (UpFirDownTests)

static void helper_test(cint32_t us, cint32_t ds, 
    const std::vector< std::vector<float32_t> >& in, 
    const std::vector<float32_t>& fir, const std::vector<float32_t>& ref,
    cfloat32_t eps = 0.F, cint32_t blocksize = 0, bool_t verbose = false );

static void helper_test_square(int32_t us, int32_t ds, std::vector<float32_t>& fir, 
    std::vector<float32_t>& ref, cint32_t blocksize = 0);

std::vector<float32_t> gen_square(cint32_t len, cfloat32_t freq, cfloat32_t fs, 
    cfloat32_t amplitude = 0.5F, cfloat32_t dutycycle = 0.5F);

//=============================================================
// Helper functions
//=============================================================

/**
 * @brief generic helper function for all tests
 * 
 * @param us Upsampling factor
 * @param ds Downsampling factor
 * @param in Input samples
 * @param fir FIR coefficients
 * @param ref Reference values
 * @param eps Epsilon for absolute value comparison
 * @param blocksize Number of wanted output sample split
 * @param verbose Flag for verbosity
 */
static void helper_test(cint32_t us, cint32_t ds, 
    const std::vector< std::vector<float32_t> >& in, 
    const std::vector<float32_t>& fir, const std::vector<float32_t>& ref,
    cfloat32_t eps, cint32_t blocksize, bool_t verbose )
{
    CUpFirDown ufp = CUpFirDown();
    float32_t *out;

    // check if input array is not empty
    BOOST_REQUIRE_GT(in.size(), 0);
    
    int32_t out_len;
    if ( 0 == blocksize )
        out_len = ufp.getLenOut(us, ds, in[0].size());
    else
        out_len = blocksize;
    
    ufp.init(us, ds, out_len, fir);

    int32_t offset = 0;
    for(auto it = in.begin(); it != in.end(); it++)
    {
        out = ufp.apply(it->data(), it->size());
        auto ind_end = std::min(out_len, (int32_t)ref.size() - offset);
        for( auto i = 0; i < ind_end; i++){
            if (verbose)
                std::cout << "testing index=" << offset+i << std::endl;
            BOOST_REQUIRE_LE( abs(out[i] - ref[offset + i]), eps );
        }
        offset += ind_end;
    }
}

/**
 * @brief Helper to test square wave
 * 
 * @param us Upsampling factor
 * @param ds Downsampling factor
 * @param fir FIR coefficients
 * @param ref Vector with reference values
 * @param blocksize Number of wanted output sample split
 */
static void helper_test_square(int32_t us, int32_t ds, std::vector<float32_t>& fir, 
    std::vector<float32_t>& ref, cint32_t blocksize)
{
    cint32_t nsamples = 750;
    auto square = gen_square(nsamples, 1000.F, 48000.F, 0.5F, 0.5F);

    auto gcd = CUpFirDown::binaryGCD(us, ds);
    us /= gcd;
    ds /= gcd;

    // multiply by upsampling factor
    for (auto it = fir.begin(); it != fir.end(); it++)
        (*it) *= (float32_t)us;
    ref.resize(nsamples * us / ds);
    
    if (blocksize == 0)
    {
        helper_test( us, ds,
            {
                square
            }, 
            fir, 
            ref,
            1.E-6
        );
    }
    else
    {
        CUpFirDown::tLenIn len_in = CUpFirDown::getLenIn(us, ds, blocksize);
        std::vector<std::vector<float32_t>> square_split;
        if (len_in.min != len_in.max)
        {
            std::vector<int32_t> split_n(len_in.num_min + len_in.num_max);
            for( auto i = 0; i < len_in.num_max; i++)
                split_n[i] = len_in.max;
            for( auto i = 0; i < len_in.num_min; i++)
                split_n[len_in.num_max + i] = len_in.min;
            square_split = split(square, split_n );
        }
        else
        {
            square_split = split(square, len_in.min );
        }
        helper_test( us, ds,
            square_split,
            fir, 
            ref,
            1.E-6,
            blocksize
        );
    }
}

/**
 * @brief Generate square wave 
 * 
 * @param len Number of output samples
 * @param freq Frequency in Hz
 * @param fs Sampling frequency in Hz
 * @param amplitude Linear amplitude gain
 * @param dutycycle Duty cycle between 0.0 and 1.0
 * @return std::vector<float32_t> Vector with square wave
 */
std::vector<float32_t> gen_square(cint32_t len, cfloat32_t freq, cfloat32_t fs, 
    cfloat32_t amplitude, cfloat32_t dutycycle)
{
    float32_t period = fs / freq;
    int32_t iter = ceil(len / period);
    std::vector<float32_t> ret = std::vector<float32_t>(iter * period);
    
    int32_t ind = 0;
    for(auto i = 0; i < iter; i++)
    {
        int32_t npos = period * dutycycle;
        int32_t nneg = period - npos;

        for(auto j = 0; j < npos; j++)
            ret[ind++] = amplitude;
        for(auto j = 0; j < nneg; j++)
            ret[ind++] = -amplitude;
    }
    ret.resize(len);

    return ret;
}

//=============================================================
// Test cases
//=============================================================

/**
 * @brief Test case: Bypass FIR, all at once
 * 
 */
BOOST_AUTO_TEST_CASE (UpFirDown_SimpleFir)
{
    helper_test( 1, 1,
        {
            {1.F, 1.F, 1.F}
        }, 
        {1.F, 1.F, 1.F }, 
        {1.F, 2.F, 3.F }
    );
    helper_test( 1, 1,
        {
            {1.F, 1.F, 1.F, 0.F, 0.F}
        }, 
        {1.F, 1.F, 1.F }, 
        {1.F, 2.F, 3.F, 2.F, 1.F }
    );
    helper_test( 1, 1,
        {
            {1.F, 1.F, 1.F},
            {1.F, 1.F, 1.F}
        }, 
        {1.F, 1.F, 1.F }, 
        {1.F, 2.F, 3.F, 3.F, 3.F, 3.F }
    );

    // square wave, all at once
    helper_test( 1, 1,
        {
            gen_square(750, 1000.F, 48000.F, 0.5F, 0.5F)
        }, 
        {1.F}, 
        std::vector<float32_t>(Square::samples[0], Square::samples[0] + Square::samples_per_ch)
    );
}

/**
 * @brief Test case: Bypass FIR with blocksize 64
 * 
 */
BOOST_AUTO_TEST_CASE (UpFirDown_SimpleFirBlocksize64)
{
    cint32_t blocksize = 64;
    cint32_t nsamples = 750;
    auto square = gen_square(nsamples, 1000.F, 48000.F, 0.5F, 0.5F);
    auto square_split = split(square, blocksize);
    auto ref = std::vector<float32_t>(Square::samples[0], Square::samples[0] + Square::samples_per_ch);
    ref.resize(square_split.size() * blocksize);
    
    helper_test( 1, 1,
        square_split,
        {1.F}, 
        ref
    );
}

/**
 * @brief Test case: Bypass FIR with blocksize 96
 * 
 */
BOOST_AUTO_TEST_CASE (UpFirDown_SimpleFirBlocksize96)
{
    cint32_t blocksize = 96;
    cint32_t nsamples = 750;
    auto square = gen_square(nsamples, 1000.F, 48000.F, 0.5F, 0.5F);
    auto square_split = split(square, blocksize);
    auto ref = std::vector<float32_t>(Square::samples[0], Square::samples[0] + Square::samples_per_ch);
    ref.resize(square_split.size() * blocksize);
    
    helper_test( 1, 1,
        square_split,
        {1.F}, 
        ref
    );
}

/**
 * @brief Test case: Simple upsampling, all at once
 * 
 */
BOOST_AUTO_TEST_CASE (UpFirDown_Upsample)
{
    // upsampling with zeros insertion
    helper_test( 3, 1,
        {
            {1.F, 2.F, 3.F}
        }, 
        {1.F }, 
        {1.F, 0.F, 0.F, 2.F, 0.F, 0.F, 3.F, 0.F, 0.F }
    );

    // upsampling with sample and hold
    helper_test( 3, 1,
        {
            {1.F, 2.F, 3.F}
        }, 
        {1.F, 1.F, 1.F }, 
        {1.F, 1.F, 1.F, 2.F, 2.F, 2.F, 3.F, 3.F, 3.F }
    );

    // upsampling with linear interpolation
    helper_test( 2, 1,
        {
            {1.F, 1.F, 1.F}
        }, 
        {0.5F, 1.F, 0.5F }, 
        {0.5F, 1.F, 1.F, 1.F, 1.F, 1.F }
    );

    // upsampling with linear interpolation w/ zero padding
    helper_test( 2, 1,
        {
            {1.F, 1.F, 1.F, 0.}
        }, 
        {0.5F, 1.F, 0.5F }, 
        {0.5F, 1.F, 1.F, 1.F, 1.F, 1.F, 0.5F, 0.F }
    );
}

/**
 * @brief Test case: Simple downsampling, all at once
 * 
 */
BOOST_AUTO_TEST_CASE (UpFirDown_Downsample)
{
    // decimation by 3
    helper_test( 1, 3,
        {
            {0.F, 1.F, 2.F, 3.F, 4.F, 5.F, 6.F, 7.F, 8.F, 9.F, 10.F, 11.F}
        }, 
        {1.F}, 
        {0.F, 3.F, 6.F, 9.F }
    );

    // decimation by 3 "missing" 2 samples at the end
    helper_test( 1, 3,
        {
            {0.F, 1.F, 2.F, 3.F, 4.F, 5.F, 6.F, 7.F, 8.F, 9.F}
        }, 
        {1.F}, 
        {0.F, 3.F, 6.F, 9.F }
    );
}

/**
 * @brief Test case: Complete case of upsampling/downsampling, all at once
 * 
 */
BOOST_AUTO_TEST_CASE (UpFirDown_UpAndDownsample)
{
    // linear interp, rate 2/3
    helper_test( 2, 3,
        {
            {0.F, 1.F, 2.F, 3.F, 4.F, 5.F, 6.F, 7.F, 8.F, 9.F, 10.F, 11.F}
        }, 
        {0.5F, 1.F, 0.5F}, 
        {0.F, 1.F, 2.5F, 4.F, 5.5F, 7.F, 8.5F, 10.F }
    );

    // linear interp, rate 2/3, "missing" 2 samples
    helper_test( 2, 3,
        {
            {0.F, 1.F, 2.F, 3.F, 4.F, 5.F, 6.F, 7.F, 8.F, 9.F}
        }, 
        {0.5F, 1.F, 0.5F}, 
        {0.F, 1.F, 2.5F, 4.F, 5.5F, 7.F, 8.5F}
    );

    std::vector<float32_t> fir, ref;

    // Square wave up by rate 3/2, all at once
    fir = std::vector<float32_t>(TestFirCoeffs::FIR_RESAMPLE_FAC3, 
            TestFirCoeffs::FIR_RESAMPLE_FAC3 + sizeof(TestFirCoeffs::FIR_RESAMPLE_FAC3) / sizeof(float32_t));
    ref = std::vector<float32_t>(SquareDown5th::samples[0], SquareDown5th::samples[0] + SquareDown5th::samples_per_ch);
    helper_test_square( 3, 2, fir, ref);

        
    // Square wave down by rate 2/3, all at once
    fir = std::vector<float32_t>(TestFirCoeffs::FIR_RESAMPLE_FAC3, 
            TestFirCoeffs::FIR_RESAMPLE_FAC3 + sizeof(TestFirCoeffs::FIR_RESAMPLE_FAC3) / sizeof(float32_t));
    ref = std::vector<float32_t>(SquareUp5th::samples[0], SquareUp5th::samples[0] + SquareUp5th::samples_per_ch);
    helper_test_square( 2, 3, fir, ref);

    // Square wave up by rate 6/4, all at once
    fir = std::vector<float32_t>(TestFirCoeffs::FIR_RESAMPLE_FAC3, 
            TestFirCoeffs::FIR_RESAMPLE_FAC3 + sizeof(TestFirCoeffs::FIR_RESAMPLE_FAC3) / sizeof(float32_t));
    ref = std::vector<float32_t>(SquareDown5th::samples[0], SquareDown5th::samples[0] + SquareDown5th::samples_per_ch);
    helper_test_square( 6, 4, fir, ref);

        
    // Square wave down by rate 16/24, all at once
    fir = std::vector<float32_t>(TestFirCoeffs::FIR_RESAMPLE_FAC3, 
            TestFirCoeffs::FIR_RESAMPLE_FAC3 + sizeof(TestFirCoeffs::FIR_RESAMPLE_FAC3) / sizeof(float32_t));
    ref = std::vector<float32_t>(SquareUp5th::samples[0], SquareUp5th::samples[0] + SquareUp5th::samples_per_ch);
    helper_test_square( 16, 24, fir, ref);
}

/**
 * @brief Test case: Complete case of upsampling/downsampling, split in 96 sample blocks
 * 
 */
BOOST_AUTO_TEST_CASE (UpFirDown_UpAndDownsampleBlocksize96)
{
    std::vector<float32_t> fir, ref;

    // Square wave up by rate 3/2, all at once
    fir = std::vector<float32_t>(TestFirCoeffs::FIR_RESAMPLE_FAC3, 
            TestFirCoeffs::FIR_RESAMPLE_FAC3 + sizeof(TestFirCoeffs::FIR_RESAMPLE_FAC3) / sizeof(float32_t));
    ref = std::vector<float32_t>(SquareDown5th::samples[0], SquareDown5th::samples[0] + SquareDown5th::samples_per_ch);
    helper_test_square( 3, 2, fir, ref, 96);

        
    // Square wave down by rate 2/3, all at once
    fir = std::vector<float32_t>(TestFirCoeffs::FIR_RESAMPLE_FAC3, 
            TestFirCoeffs::FIR_RESAMPLE_FAC3 + sizeof(TestFirCoeffs::FIR_RESAMPLE_FAC3) / sizeof(float32_t));
    ref = std::vector<float32_t>(SquareUp5th::samples[0], SquareUp5th::samples[0] + SquareUp5th::samples_per_ch);
    helper_test_square( 2, 3, fir, ref, 96);
}

/**
 * @brief Test case: Complete case of upsampling/downsampling, split in 64 sample blocks
 * 
 */
BOOST_AUTO_TEST_CASE (UpFirDown_UpAndDownsampleBlocksize64)
{
    std::vector<float32_t> fir, ref;

    // Square wave up by rate 3/2, all at once
    fir = std::vector<float32_t>(TestFirCoeffs::FIR_RESAMPLE_FAC3, 
            TestFirCoeffs::FIR_RESAMPLE_FAC3 + sizeof(TestFirCoeffs::FIR_RESAMPLE_FAC3) / sizeof(float32_t));
    ref = std::vector<float32_t>(SquareDown5th::samples[0], SquareDown5th::samples[0] + SquareDown5th::samples_per_ch);
    helper_test_square( 3, 2, fir, ref, 64);

        
    // Square wave down by rate 2/3, all at once
    fir = std::vector<float32_t>(TestFirCoeffs::FIR_RESAMPLE_FAC3, 
            TestFirCoeffs::FIR_RESAMPLE_FAC3 + sizeof(TestFirCoeffs::FIR_RESAMPLE_FAC3) / sizeof(float32_t));
    ref = std::vector<float32_t>(SquareUp5th::samples[0], SquareUp5th::samples[0] + SquareUp5th::samples_per_ch);
    helper_test_square( 2, 3, fir, ref, 64);
}

BOOST_AUTO_TEST_SUITE_END()

#endif //UPFIRDOWN_TESTS
