#include "AtomDiode.h"
#include "AtomHelper.h"

const CAtomDiode::tAtomDiodeSpiceParams CAtomDiode::m_DiodeParams[NUM_DIODE_T] = {
    // 1N4148
    {
        4.352E-9F,  // IS
        1.906F,     // N
        0.6458F,    // RS
        7.048E-13F, // CJO
        0.869F,     // VJ
        0.03F,      // M
        3.48E-9F,   // TT
    }};

int32_t CAtomDiode::init(const CQuarkProps &props)
{
    setProps(props);

    const tAtomDiodeSpiceParams *pParams = &m_DiodeParams[m_Mode];
    float32_t a1 = 1.F / (pParams->N * m_Vt);
    float32_t a0 = pParams->IS * (0.F + pParams->RS); // 0 explicit here to indicate R=0 at init
    float32_t logA0A1 = LOG(a0 * a1);

    m_TargetA0 = new float32_t[props.m_NumChOut];
    m_DeltaA0 = new float32_t[props.m_NumChOut]();
    m_A0 = new float32_t[props.m_NumChOut];
    m_TargetLogA0A1 = new float32_t[props.m_NumChOut];
    m_DeltaLogA0A1 = new float32_t[props.m_NumChOut]();
    m_LogA0A1 = new float32_t[props.m_NumChOut];
    for (auto i = 0; i < props.m_NumChOut; i++)
    {
        m_A0[i] = a0;
        m_LogA0A1[i] = logA0A1;
        m_TargetA0[i] = a0;
        m_TargetLogA0A1[i] = logA0A1;
    }

    m_MakeUpDeltaGains = new float32_t[props.m_NumChOut]();
    m_MakeUpTargetGains = new float32_t[props.m_NumChOut];
    m_MakeUpGains = new float32_t[props.m_NumChOut];
    for (auto i = 0; i < props.m_NumChOut; i++)
    {
        m_MakeUpGains[i] = 1.0F;
        m_MakeUpTargetGains[i] = 1.0F;
    }

    return 0;
}

void CAtomDiode::play(float32_t **const in, float32_t **const out)
{
    const tAtomDiodeSpiceParams *pParams = &m_DiodeParams[m_Mode];

    float32_t a1 = 1.F / (pParams->N * m_Vt);

    if (NULL != out && NULL != in)
    {
        if (m_MorphBlocksizeCnt > 0)
        {
            // morphing
            for (auto ch = 0; ch < m_Props.m_NumChOut; ch++)
            {
                float32_t *pIn = in[ch];
                float32_t *pOut = out[ch];
                float32_t *pA0 = &m_A0[ch];
                float32_t *pDeltaA0 = &m_DeltaA0[ch];
                float32_t *pLogA0A1 = &m_LogA0A1[ch];
                float32_t *pDeltaLogA0A1 = &m_DeltaLogA0A1[ch];
                float32_t *pMupGain = &m_MakeUpGains[ch];
                float32_t *pMupDeltaGain = &m_MakeUpDeltaGains[ch];

                for (auto i = 0; i < m_Props.m_BlockSize; i++)
                {
                    *pA0 += *pDeltaA0;
                    *pLogA0A1 += *pDeltaLogA0A1;
                    *pMupGain += *pMupDeltaGain;
                    float32_t absIn = fabs(pIn[i]);
                    float32_t w = NAtomHelper::WrightOmegaReal<float32_t>(a1 * (absIn + *pA0) + *pLogA0A1);
                    pOut[i] = absIn + *pA0 - w / a1;
                    if (pIn[i] < 0.F)
                        pOut[i] *= -*pMupGain;
                    else
                        pOut[i] *= *pMupGain;
                }
            }
            if (--m_MorphBlocksizeCnt <= 0)
            {
                for (auto ch = 0; ch < m_Props.m_NumChOut; ch++)
                {
                    m_A0[ch] = m_TargetA0[ch];
                    m_LogA0A1[ch] = m_TargetLogA0A1[ch];
                }
            }
        }
        else
        {
            for (auto ch = 0; ch < m_Props.m_NumChOut; ch++)
            {
                float32_t *pIn = in[ch];
                float32_t *pOut = out[ch];
                float32_t *pMupGain = &m_MakeUpGains[ch];
                float32_t a0 = m_A0[ch];
                float32_t logA0A1 = m_LogA0A1[ch];
                for (auto i = 0; i < m_Props.m_BlockSize; i++)
                {
                    float32_t absIn = fabs(pIn[i]);
                    float32_t w = NAtomHelper::WrightOmegaReal<float32_t>(a1 * (absIn + a0) + logA0A1);
                    pOut[i] = absIn + a0 - w / a1;
                    if (pIn[i] < 0.F)
                        pOut[i] *= -*pMupGain;
                    else
                        pOut[i] *= *pMupGain;
                }
            }
        }
    }
}

void CAtomDiode::set(cint32_t ch, cint32_t el, cfloat32_t value)
{
    float32_t targetGain = CLIP(value, 0.F, 10000000.F);
    float32_t mupTargetGain = 1.0F;
    const tAtomDiodeSpiceParams *pParams = &m_DiodeParams[m_Mode];
    float32_t a1 = 1.F / (pParams->N * m_Vt);
    float32_t a0 = pParams->IS * (targetGain + pParams->RS);
    float32_t logA0A1 = LOG(a0 * a1);

    if (m_Normalize)
    {
        float32_t absIn = 1.0F;
        float32_t w = NAtomHelper::WrightOmegaReal<float32_t>(a1 * (absIn + a0) + logA0A1);
        float32_t out = absIn + a0 - w / a1;
        mupTargetGain = 1.F / out;
    }

    if (ch == SET_ALL_CH_IND)
    {
        for (auto c = 0; c < m_Props.m_NumChOut; c++)
        {
            m_TargetA0[c] = a0;
            m_TargetLogA0A1[c] = logA0A1;
            m_MakeUpTargetGains[c] = mupTargetGain;
        }
    }
    else
    {
        m_TargetA0[ch] = a0;
        m_TargetLogA0A1[ch] = logA0A1;
        m_MakeUpTargetGains[ch] = mupTargetGain;
    }

    calculateDeltas();
    startMorph();
}

void CAtomDiode::calculateDeltas(void)
{
    if (m_MorphBlocksizeTotal > 0.F)
    {
        const auto den = (m_MorphBlocksizeTotal * m_Props.m_BlockSize);
        for (auto c = 0; c < m_Props.m_NumChOut; c++)
        {
            m_DeltaA0[c] = (m_TargetA0[c] - m_A0[c]) / den;
            m_DeltaLogA0A1[c] = (m_TargetLogA0A1[c] - m_LogA0A1[c]) / den;
            m_MakeUpDeltaGains[c] = (m_MakeUpTargetGains[c] - m_MakeUpGains[c]) / den;
        }
    }
    else
    {
        for (auto c = 0; c < m_Props.m_NumChOut; c++)
        {
            m_DeltaA0[c] = 0.0F;
            m_A0[c] = m_TargetA0[c];

            m_DeltaLogA0A1[c] = 0.0F;
            m_LogA0A1[c] = m_TargetLogA0A1[c];

            m_MakeUpDeltaGains[c] = 0.0F;
            m_MakeUpGains[c] = m_MakeUpTargetGains[c];
        }
    }
}
