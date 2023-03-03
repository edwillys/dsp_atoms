#include <math.h>
#include <algorithm>
#include "UpFirDown.h"

void CUpFirDown::init(int32_t us_factor, int32_t ds_factor, int32_t numout, const std::vector<float32_t> &fir)
{
    m_UsFactor = std::max(us_factor, 1);
    m_DsFactor = std::max(ds_factor, 1);

    if (m_UsFactor == 1 && m_DsFactor == 1 &&
        (fir.size() == 0 || (fir.size() == 1 && abs(fir[0] - 1.F) <= 0.F)))
    {
        m_Bypass = true;
        m_LenInMax = numout;
        if (numout > 0)
            m_Buffer = new float32_t[numout]();
    }
    else
    {
        m_Bypass = false;
        // reduce fraction
        auto gcd = binaryGCD(m_UsFactor, m_DsFactor);
        m_UsFactor /= gcd;
        m_DsFactor /= gcd;

        // calculate FIR transposed for polyphase
        m_PhaseLen = (int32_t)ceil((float32_t)fir.size() / (float32_t)m_UsFactor);
        cint32_t firt_len = m_PhaseLen * m_UsFactor;
        cint32_t fir_size = (cint32_t)fir.size();
        m_FirTrans.resize(firt_len);
        for (auto i = 0; i < m_UsFactor; i++)
        {
            for (auto j = 0; j < m_PhaseLen; j++)
            {
                auto ind = i + m_UsFactor * j;
                if (ind < fir_size)
                    m_FirTrans[m_PhaseLen * i + j] = fir[ind];
                else
                    m_FirTrans[m_PhaseLen * i + j] = 0.0F;
            }
        }

        auto len_in = getLenIn(m_UsFactor, m_DsFactor, numout);
        // default blocksize as maximum
        m_LenInMax = len_in.max;

        // Offest variables in case we need the input to come with different number of samples at each iteraction
        m_OffsetOut = 0;
        m_OffsetOutMax = (getLenOut(m_UsFactor, m_DsFactor, len_in.max) - numout) * len_in.num_max;

        m_LenOut = numout;
        if (numout > 0)
        {
            auto len_us = (m_LenInMax * m_UsFactor);
            // allocate for worst case
            m_Buffer = new float32_t[numout + m_OffsetOutMax]();
            m_BufferScratchIn = new float32_t[m_LenInMax + m_FirTrans.size() - 1]();
            m_BufferScratchOut = new float32_t[len_us]();
        }
    }
}

void CUpFirDown::deinit(void)
{
    if (NULL != m_BufferScratchIn)
    {
        delete m_BufferScratchIn;
        m_BufferScratchIn = NULL;
    }
    if (NULL != m_BufferScratchOut)
    {
        delete m_BufferScratchOut;
        m_BufferScratchOut = NULL;
    }
    if (NULL != m_Buffer)
    {
        delete m_Buffer;
        m_Buffer = NULL;
    }
}

float32_t *CUpFirDown::apply(cfloat32_t *const in)
{
    return apply(in, m_LenInMax);
}

float32_t *CUpFirDown::apply(const float32_t *RESTRICT const in, cint32_t numin)
{
    if (NULL != in)
    {
        // Prepare optimized separated buffers
        // TODO: use __builtin_assume_aligned(..., 16)?
        float32_t *RESTRICT firt = m_FirTrans.data();
        float32_t *RESTRICT bufout = m_BufferScratchOut;
        float32_t *RESTRICT bufin = m_BufferScratchIn;
        float32_t *RESTRICT buf = m_Buffer;

        if (m_Bypass)
        {
            auto iter = std::min(numin, m_LenInMax);
            for (int32_t i = 0; i < iter; i++)
                buf[i] = in[i];
        }
        else
        {
            cint32_t firt_len = (cint32_t)m_FirTrans.size();
            cint32_t len_us = numin * m_UsFactor;
            cint32_t len_out = getLenOut(m_UsFactor, m_DsFactor, numin);

            // copy samples from previous iteration, if any
            for (int32_t i = 0; i < m_OffsetOut; i++)
                buf[i] = buf[i + m_LenOut];

            // Zero output buffer first.
            // Input buffer is not needed as it will be overwritten further below
            for (int32_t i = 0; i < len_us; i++)
            {
                bufout[i] = 0.0F;
            }

            // Copy input to input scratch buffer. TODO: avoid this?
            for (int32_t i = 0; i < numin; i++)
                bufin[firt_len - 1 + i] = in[i];

            // Filter convolution in a polyphased manner
            cint32_t bufin_off = firt_len - 1;
            for (auto i = 0; i < numin; i++)
            {
                int32_t firt_off = 0;
                cint32_t outind_off = i * m_UsFactor;
                for (auto j = 0; j < m_UsFactor; j++)
                {
                    // apply FIR
                    for (auto ph = 0; ph < m_PhaseLen; ph++)
                    {
                        bufout[outind_off + j] += firt[firt_off + ph] * bufin[bufin_off + i - ph];
                    }
                    firt_off += m_PhaseLen;
                }
            }
            // copy last chunk of m_BufferScratchIn to its input
            for (auto j = 0; j < firt_len - 1; j++)
                bufin[j] = bufin[numin + j];

            // Decimate
            for (int32_t i = 0; i < len_out; i++)
                buf[i + m_OffsetOut] = bufout[i * m_DsFactor];

            // Calculate eventual offset for the next iteration
            m_OffsetOut += (len_out - m_LenOut);
            // Dummy check: limit it to valid range to avoid wrong indexing
            m_OffsetOut = std::max(m_OffsetOut, 0);
            m_OffsetOut = std::min(m_OffsetOut, m_OffsetOutMax);
        }
        return m_Buffer;
    }
    else
    {
        return NULL;
    }
}

cint32_t CUpFirDown::getLenOut(int32_t us, int32_t ds, cint32_t numin)
{
    // reduce fraction
    auto gcd = binaryGCD(us, ds);
    us /= gcd;
    ds /= gcd;
    return (cint32_t)ceil(numin / (float32_t)ds) * us;
}

CUpFirDown::tLenIn const CUpFirDown::getLenIn(int32_t us, int32_t ds, cint32_t numout)
{
    tLenIn retval;

    // reduce fraction
    auto gcd = binaryGCD(us, ds);
    us /= gcd;
    ds /= gcd;

    // maximum and minimum number of required input length
    retval.max = (int32_t)ceil(numout / (float32_t)us) * ds;
    retval.min = (int32_t)floor(numout / (float32_t)us) * ds;

    if (retval.max != retval.min)
    {
        // number of times that the input should come with maximum length
        retval.num_min = getLenOut(us, ds, retval.max) - numout;
        // number of times that the input should come with minimum length
        retval.num_max = numout - getLenOut(us, ds, retval.min);
    }
    else
    {
        // maximum and minimum are the same. Assign values to 1, instead of what would be calculated as 0
        retval.num_min = 1;
        retval.num_max = 1;
    }

    return retval;
}

CUpFirDown::tLenIn const CUpFirDown::getLenIn(cint32_t numout)
{
    tLenIn retval;

    // maximum and minimum number of required input length
    retval.max = (int32_t)ceil(numout / (float32_t)m_UsFactor) * m_DsFactor;
    retval.min = (int32_t)floor(numout / (float32_t)m_UsFactor) * m_DsFactor;

    if (retval.max != retval.min)
    {
        // number of times that the input should come with maximum length
        retval.num_min = getLenOut(m_UsFactor, m_DsFactor, retval.max) - numout;
        // number of times that the input should come with minimum length
        retval.num_max = numout - getLenOut(m_UsFactor, m_DsFactor, retval.min);
    }
    else
    {
        // maximum and minimum are the same. Assign values to 1, instead of what would be calculated as 0
        retval.num_min = 1;
        retval.num_max = 1;
    }

    return retval;
}

uint32_t CUpFirDown::binaryGCD(uint32_t u, uint32_t v)
{
    uint32_t shift = 0U;

    /* GCD(0,v) == v; GCD(u,0) == u, GCD(0,0) == 0 */
    if (u == 0U)
        return v;
    if (v == 0U)
        return u;

    /* Let shift := lg K, where K is the greatest power of 2
        dividing both u and v. */
    while (((u | v) & 1U) == 0U)
    {
        shift++;
        u >>= 1U;
        v >>= 1U;
    }

    while ((u & 1U) == 0U)
        u >>= 1U;

    /* From here on, u is always odd. */
    do
    {
        /*
            Remove all factors of 2 in v -- they are not common
            note: v is not zero, so while will terminate
        */
        while ((v & 1U) == 0U)
            v >>= 1U;

        /*
           Now u and v are both odd. Swap if necessary so u <= v,
           then set v = v - u (which is even). For bignums, the
           swapping is just pointer movement, and the subtraction
           can be done in-place.
        */
        if (u > v)
        {
            // Swap u and v.
            uint32_t t = v;
            v = u;
            u = t;
        }

        v -= u; // Here v >= u.
    } while (v != 0U);

    /* restore common factors of 2 */
    return u << shift;
}
