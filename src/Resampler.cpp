#include "Resampler.h"
#include <math.h>
#include <algorithm>

namespace FIRCoeffs
{
    #include "ResampleFIRCoeffs.h"
}

void CResampler::init(int32_t us_factor, int32_t ds_factor, int32_t blocksize) 
{
    m_Blocksize = blocksize;
    m_LenInMax = blocksize;
    if ( checkFactor(us_factor) && checkFactor(ds_factor) )
    {
        auto rate = std::max(us_factor, ds_factor);
        if (rate > 1)
        {
            struct
            {
                int32_t len;
                float32_t *p_coeffs;
            } map[] = {
                {sizeof(FIRCoeffs::FIR_RESAMPLE_FAC2) / sizeof(float32_t), FIRCoeffs::FIR_RESAMPLE_FAC2},
                {sizeof(FIRCoeffs::FIR_RESAMPLE_FAC3) / sizeof(float32_t), FIRCoeffs::FIR_RESAMPLE_FAC3},
                {sizeof(FIRCoeffs::FIR_RESAMPLE_FAC4) / sizeof(float32_t), FIRCoeffs::FIR_RESAMPLE_FAC4},
            };

            auto len = map[rate - 2].len;
            auto coeffs = map[rate - 2].p_coeffs;
            auto fir = std::vector<float32_t>(coeffs, coeffs + len);
            // multiply by upsampling factor
            for (auto it = fir.begin(); it != fir.end(); it++)
                (*it) *= (float32_t)us_factor;
            CUpFirDown::init(us_factor, ds_factor, blocksize, fir);
        }
        else
        {
            CUpFirDown::init(1, 1, blocksize, {1.F});
        }
    }
    else
    {
       CUpFirDown::init(1, 1, blocksize, {1.F});
    }
}

void CResampler::setBlocksize( int32_t blocksize, bool_t force )
{
    if ( force || blocksize != m_Blocksize )
    {
        deinit();
        init(m_UsFactor, m_DsFactor, blocksize);
        m_Blocksize = blocksize;
    }
}

bool_t CResampler::checkFactor(int32_t factor)
{
    return (factor >= 1) && (factor <= 4);
}
