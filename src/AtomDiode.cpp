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

    m_TargetGains = new float32_t[props.m_NumChOut]();
    m_DeltaGains = new float32_t[props.m_NumChOut]();
    m_Gains = new float32_t[props.m_NumChOut]();

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
                float32_t *pGain = &m_Gains[ch];
                float32_t *pDeltaGain = &m_DeltaGains[ch];
                float32_t *pMupGain = &m_MakeUpGains[ch];
                float32_t *pMupDeltaGain = &m_MakeUpDeltaGains[ch];

                for (auto i = 0; i < m_Props.m_BlockSize; i++)
                {
                    *pGain += *pDeltaGain;
                    *pMupGain += *pMupDeltaGain;
                    float32_t a0 = pParams->IS * (*pGain + pParams->RS);
                    float32_t a2 = LOG(a0 * a1); // TODO: avoid this
                    float32_t absIn = fabs(pIn[i]);
                    float32_t w = NAtomHelper::WrightOmegaReal<float32_t>(a1 * (absIn + a0) + a2);
                    pOut[i] = absIn + a0 - w / a1;
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
                    m_Gains[ch] = m_TargetGains[ch];
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
                float32_t a0 = pParams->IS * (m_Gains[ch] + pParams->RS);
                float32_t a2 = LOG(a0 * a1);
                for (auto i = 0; i < m_Props.m_BlockSize; i++)
                {
                    float32_t absIn = fabs(pIn[i]);
                    float32_t w = NAtomHelper::WrightOmegaReal<float32_t>(a1 * (absIn + a0) + a2);
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
    float32_t targetGain = CLIP(value, 10.F, 10000000.F);

    float32_t mupTargetGain = 1.0F;
    if (m_Normalize)
    {
        const tAtomDiodeSpiceParams *pParams = &m_DiodeParams[m_Mode];
        float32_t a1 = 1.F / (pParams->N * m_Vt);
        float32_t a0 = pParams->IS * (targetGain + pParams->RS);
        float32_t a2 = LOG(a0 * a1);
        float32_t absIn = 1.0F;
        float32_t w = NAtomHelper::WrightOmegaReal<float32_t>(a1 * (absIn + a0) + a2);
        float32_t out = absIn + a0 - w / a1;
        mupTargetGain = 1.F / out;
    }

    if (ch == SET_ALL_CH_IND)
    {
        for (auto c = 0; c < m_Props.m_NumChOut; c++)
        {
            m_TargetGains[c] = targetGain;
            m_MakeUpTargetGains[c] = mupTargetGain;
        }
    }
    else
    {
        m_TargetGains[ch] = targetGain;
        m_MakeUpTargetGains[ch] = mupTargetGain;
    }

    calculateDeltas();
    startMorph();
}

void CAtomDiode::calculateDeltas(void)
{
    if (m_MorphBlocksizeTotal > 0.F)
    {
        for (auto c = 0; c < m_Props.m_NumChOut; c++)
        {
            m_DeltaGains[c] = (m_TargetGains[c] - m_Gains[c]) / (m_MorphBlocksizeTotal * m_Props.m_BlockSize);
            m_MakeUpDeltaGains[c] = (m_MakeUpTargetGains[c] - m_MakeUpGains[c]) / (m_MorphBlocksizeTotal * m_Props.m_BlockSize);
        }
    }
    else
    {
        for (auto c = 0; c < m_Props.m_NumChOut; c++)
        {
            m_DeltaGains[c] = 0.0F;
            m_Gains[c] = m_TargetGains[c];

            m_MakeUpDeltaGains[c] = 0.0F;
            m_MakeUpGains[c] = m_MakeUpTargetGains[c];
        }
    }
}
