#include "AtomBiquad.h"

int32_t CAtomBiquad::init(const CQuarkProps &props)
{
    setProps(props);

    m_TargetStates = new tAtomBiquadStates *[props.m_NumChOut];
    m_DeltaStates = new tAtomBiquadStates *[props.m_NumChOut];
    m_States = new tAtomBiquadStates *[props.m_NumChOut];
    m_TargetCoeffs = new tAtomBiquadCoeffs *[props.m_NumChOut];
    m_DeltaCoeffs = new tAtomBiquadCoeffs *[props.m_NumChOut];
    m_Coeffs = new tAtomBiquadCoeffs *[props.m_NumChOut];

    for (auto ch = 0; ch < props.m_NumChOut; ch++)
    {
        m_TargetStates[ch] = new tAtomBiquadStates[props.m_NumEl]();
        m_DeltaStates[ch] = new tAtomBiquadStates[props.m_NumEl]();
        m_States[ch] = new tAtomBiquadStates[props.m_NumEl]();
        m_TargetCoeffs[ch] = new tAtomBiquadCoeffs[props.m_NumEl]();
        m_DeltaCoeffs[ch] = new tAtomBiquadCoeffs[props.m_NumEl]();
        m_Coeffs[ch] = new tAtomBiquadCoeffs[props.m_NumEl]();
        for(auto el = 0; el < props.m_NumEl; el++)
            m_Coeffs[ch][el].b1 = 1.0F; // init state = bypass
        
    }

    return 0;
}

void CAtomBiquad::play(float32_t **const in, float32_t **const out)
{
    if (NULL != out && NULL != in)
    {
        if (m_MorphBlocksizeCnt > 0)
        {
            for (auto ch = 0; ch < m_Props.m_NumChOut; ch++)
            {
                float32_t *pIn = in[ch];
                float32_t *pOut = out[ch];
                for (auto el = 0; el < m_Props.m_NumEl; el++)
                {
                    tAtomBiquadCoeffs *pCoeffs = &m_Coeffs[ch][el];
                    tAtomBiquadCoeffs *pCoeffsDelta = &m_DeltaCoeffs[ch][el];
                    tAtomBiquadStates *pStates = &m_States[ch][el];
                    float32_t *pS2 = &pStates->s2;
                    float32_t *pS1 = &pStates->s1;
                    for (auto i = 0; i < m_Props.m_BlockSize; i++)
                    {
                        *pCoeffs += *pCoeffsDelta;
                        // TDF-II
                        float32_t x = pIn[i];
                        float32_t y = *pS1 + pCoeffs->b0 * x;
                        pOut[i] = y;
                        *pS1 = *pS2 + x * pCoeffs->b1 - pCoeffs->a1 * y;
                        *pS2 = x * pCoeffs->b2 - pCoeffs->a2 * y;
                    }
                    pIn = pOut;
                }
            }
            if (--m_MorphBlocksizeCnt <= 0)
            {
                for (auto ch = 0; ch < m_Props.m_NumChOut; ch++)
                {
                    for (auto el = 0; el < m_Props.m_NumEl; el++)
                    {
                        m_Coeffs[ch][el] = m_TargetCoeffs[ch][el];
                        m_DeltaCoeffs[ch][el] = {0};
                    }
                }
            }
        }
        else
        {
            for (auto ch = 0; ch < m_Props.m_NumChOut; ch++)
            {
                float32_t *pIn = in[ch];
                float32_t *pOut = out[ch];
                for (auto el = 0; el < m_Props.m_NumEl; el++)
                {
                    tAtomBiquadCoeffs *pCoeffs = &m_Coeffs[ch][el];
                    tAtomBiquadStates *pStates = &m_States[ch][el];
                    float32_t *pS2 = &pStates->s2;
                    float32_t *pS1 = &pStates->s1;
                    for (auto i = 0; i < m_Props.m_BlockSize; i++)
                    {
                        // TDF-II
                        float32_t x = pIn[i];
                        float32_t y = *pS1 + pCoeffs->b0 * x;
                        pOut[i] = y;
                        *pS1 = *pS2 + x * pCoeffs->b1 - pCoeffs->a1 * y;
                        *pS2 = x * pCoeffs->b2 - pCoeffs->a2 * y;
                    }
                    pIn = pOut;
                }
            }
        }
    }
}

void CAtomBiquad::set(void *params, cint32_t len)
{
    if (NULL != params && len > 0)
    {
        tAtomBiquadParams *p_biq_params = reinterpret_cast<tAtomBiquadParams *>(params);
        int32_t num = len / sizeof(tAtomBiquadParams);

        for (auto i = 0; i < num; i++)
        {
            calculateCoeffsCookbook(*p_biq_params);
            p_biq_params++;
        }

        startMorph();
    }
}

void CAtomBiquad::calculateDeltas(void)
{
    if (m_MorphBlocksizeTotal > 0)
    {
        for (auto ch = 0; ch < m_Props.m_NumChOut; ch++)
        {
            for (auto el = 0; el < m_Props.m_NumEl; el++)
            {
                tAtomBiquadCoeffs *c = &m_Coeffs[ch][el];
                tAtomBiquadCoeffs *tc = &m_TargetCoeffs[ch][el];
                tAtomBiquadCoeffs *dc = &m_DeltaCoeffs[ch][el];

                *dc = (*tc - *c) / (float32_t)(m_MorphBlocksizeTotal * m_Props.m_BlockSize);
            }
        }
    }
    else
    {
        for (auto ch = 0; ch < m_Props.m_NumChOut; ch++)
        {
            for (auto el = 0; el < m_Props.m_NumEl; el++)
            {
                m_Coeffs[ch][el] = m_TargetCoeffs[ch][el];
                memset(&m_DeltaCoeffs[ch][el], 0, sizeof(tAtomBiquadCoeffs));
            }
        }
    }
}

void CAtomBiquad::calculateCoeffsCookbook(const tAtomBiquadParams &params)
{
    if (
        nullptr != m_Coeffs &&
        params.ch < m_Props.m_NumChIn &&
        params.el < m_Props.m_NumEl &&
        params.type < NUM_BIQT)
    {
        // clipping values
        float32_t gainDb = CLIP(params.gainDb, MUTE_DB_FS, 50.F);
        float32_t f0 = CLIP(params.freq, 0.F, m_Props.m_Fs * 0.5F);
        float32_t q = CLIP(params.q, 0.01F, 50.0F);
        // intermediate variables
        float32_t A = sqrtf(powf(10.F, (gainDb * 0.05F)));
        float32_t a0;
        tAtomBiquadCoeffs *tc = &m_TargetCoeffs[params.ch][params.el];

        float32_t w0 = 2.F * (float32_t)M_PI * f0 / m_Props.m_Fs;
        float32_t cosw0 = cosf(w0);
        float32_t sinw0 = sinf(w0);
        float32_t alpha = sinw0 / (2.F * q);

        switch (params.type)
        {
        case BIQT_LPF_6DB:
        {
            // H(s) = 1 / (s + 1)
            float32_t K = (1.F + cosw0) / sinw0; // 1 / tan(w0/2)
            a0 = K + 1.F;
            tc->a1 = (1.F - K);
            tc->a2 = 0.0F;
            tc->b0 = 1.0F;
            tc->b1 = 1.0F;
            tc->b2 = 0.0F;
        }
        break;
        case BIQT_LPF:
        {
            tc->b0 = (1.F - cosw0) * 0.5F;
            tc->b1 = 1.F - cosw0;
            tc->b2 = (1.F - cosw0) * 0.5F;
            a0 = 1.F + alpha;
            tc->a1 = -2.F * cosw0;
            tc->a2 = 1.F - alpha;
        }
        break;
        case BIQT_HPF_6DB:
        {
            // H(s) = s / (s + 1)
            float32_t K = (1.F + cosw0) / sinw0; // 1 / tan(w0/2)
            a0 = K + 1.F;
            tc->a1 = 1.F - K;
            tc->b0 = K;
            tc->b1 = -K;
            tc->b2 = 0.0F;
        }
        break;
        case BIQT_HPF:
        {
            tc->b0 = (1.F + cosw0) * 0.5F;
            tc->b1 = -(1.F + cosw0);
            tc->b2 = (1.F + cosw0) * 0.5F;
            a0 = 1.F + alpha;
            tc->a1 = -2.F * cosw0;
            tc->a2 = 1.F - alpha;
        }
        break;
        case BIQT_PEAK:
        {
            tc->b0 = 1.F + alpha * A;
            tc->b1 = -2.F * cosw0;
            tc->b2 = 1.F - alpha * A;
            a0 = 1.F + alpha / A;
            tc->a1 = -2.F * cosw0;
            tc->a2 = 1.F - alpha / A;
        }
        break;
        case BIQT_NOTCH:
        {
            tc->b0 = 1.F;
            tc->b1 = -2.F * cosw0;
            tc->b2 = 1.F;
            a0 = 1.F + alpha;
            tc->a1 = -2.F * cosw0;
            tc->a2 = 1.F - alpha;
        }
        break;
        case BIQT_BPF:
        {
            tc->b0 = alpha;
            tc->b1 = 0.F;
            tc->b2 = -alpha;
            a0 = 1.F + alpha;
            tc->a1 = -2.F * cosw0;
            tc->a2 = 1.F - alpha;
        }
        break;
        case BIQT_HSH:
        {
            float32_t Ap1 = A + 1.F;
            float32_t Am1 = A - 1.F;
            float32_t sqrtA = sqrtf(A);
            tc->b0 = A * (Ap1 + Am1 * cosw0 + 2.F * sqrtA * alpha);
            tc->b1 = -2.F * A * (Am1 + Ap1 * cosw0);
            tc->b2 = A * (Ap1 + Am1 * cosw0 - 2.F * sqrtA * alpha);
            a0 = Ap1 - Am1 * cosw0 + 2.F * sqrtA * alpha;
            tc->a1 = 2.F * (Am1 - Ap1 * cosw0);
            tc->a2 = Ap1 - Am1 * cosw0 - 2.F * sqrtA * alpha;
        }
        break;
        case BIQT_LSH:
        {
            float32_t Ap1 = A + 1.F;
            float32_t Am1 = A - 1.F;
            float32_t sqrtA = sqrtf(A);
            tc->b0 = A * (Ap1 - Am1 * cosw0 + 2.F * sqrtA * alpha);
            tc->b1 = 2.F * A * (Am1 - Ap1 * cosw0);
            tc->b2 = A * (Ap1 - Am1 * cosw0 - 2.F * sqrtA * alpha);
            a0 = Ap1 + Am1 * cosw0 + 2.F * sqrtA * alpha;
            tc->a1 = -2.F * (Am1 + Ap1 * cosw0);
            tc->a2 = Ap1 + Am1 * cosw0 - 2.F * sqrtA * alpha;
        }
        break;
#if 0 //  TODO: below is not stable
        case BIQT_APF_180:
        {
            // H(s) = (s - 1) / (s + 1)
            float32_t K = (1.F + cosw0) / sinw0; // 1 / tan(w0/2)
            a0 = K - 1.0F;
            tc->a1 = -(K + 1.0F);
            tc->a2 = 0.0F;
            tc->b0 = (K + 1.0F);
            tc->b1 = (1.0F - K);
            tc->b2 = 0.0F;
        }
        break;
#endif
        case BIQT_APF:
        {
            tc->b0 = 1.F - alpha;
            tc->b1 = -2.F * cosw0;
            tc->b2 = 1.F + alpha;
            a0 = 1.F + alpha;
            tc->a1 = -2.F * cosw0;
            tc->a2 = 1.F - alpha;
        }
        break;

        case BIQT_BYPASS:
        default:
        {
            a0 = 1.0F;
            tc->a1 = 0.0F;
            tc->a2 = 0.0F;
            tc->b0 = 1.0F;
            tc->b1 = 0.0F;
            tc->b2 = 0.0F;
        }
        break;
        }

        tc->b0 /= a0;
        tc->b1 /= a0;
        tc->b2 /= a0;
        tc->a1 /= a0;
        tc->a2 /= a0;

        tAtomBiquadCoeffs *c = &m_Coeffs[params.ch][params.el];
        if (m_MorphBlocksizeTotal > 0)
        {
            m_DeltaCoeffs[params.ch][params.el] = (*tc - *c) / (float32_t)(m_MorphBlocksizeTotal * m_Props.m_BlockSize);
        }
        else
        {
            *c = *tc;
        }
    }
}
