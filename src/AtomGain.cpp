#include "AtomGain.h"

int32_t CAtomGain::init(const CQuarkProps &props)
{
    setProps(props);

    m_TargetGains = new float32_t[props.m_NumChOut]();
    m_DeltaGains = new float32_t[props.m_NumChOut]();
    m_Gains = new float32_t[props.m_NumChOut]();

    return 0;
}

void CAtomGain::play(float32_t **const in, float32_t **const out)
{
    if (NULL != out && NULL != in)
    {
        if (m_MorphBlocksizeCnt > 0)
        {
            for (auto ch = 0; ch < m_Props.m_NumChOut; ch++)
            {
                float32_t *pIn = in[ch];
                float32_t *pOut = out[ch];
                float32_t *pGain = &m_Gains[ch];
                float32_t *pDeltaGain = &m_DeltaGains[ch];

                for (auto i = 0; i < m_Props.m_BlockSize; i++)
                {
                    *pGain += *pDeltaGain;
                    pOut[i] = pIn[i] * *pGain;
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
                float32_t gain = m_Gains[ch];

                for (auto i = 0; i < m_Props.m_BlockSize; i++)
                {
                    pOut[i] = pIn[i] * gain;
                }
            }
        }
    }
}

void CAtomGain::set(cint32_t ch, cint32_t el, cfloat32_t value)
{
    float32_t gainDb = CLIP(value, MUTE_DB_FS, 50.0F);
    float32_t targetGainLin = gainDb <= MUTE_DB_FS ? 0.0F : powf(10.F, gainDb * 0.05F);

    if (ch == SET_ALL_CH_IND)
    {
        for (auto c = 0; c < m_Props.m_NumChOut; c++)
        {
            m_TargetGains[c] = targetGainLin;
        }
    }
    else
    {
        m_TargetGains[ch] = targetGainLin;
    }
    calculateDeltas();
    startMorph();
}

void CAtomGain::calculateDeltas(void)
{
    if (m_MorphBlocksizeTotal > 0.F)
    {
        for (auto c = 0; c < m_Props.m_NumChOut; c++)
        {
            m_DeltaGains[c] = (m_TargetGains[c] - m_Gains[c]) / (m_MorphBlocksizeTotal * m_Props.m_BlockSize);
        }
    }
    else
    {
        for (auto c = 0; c < m_Props.m_NumChOut; c++)
        {
            m_DeltaGains[c] = 0.0F;
            m_Gains[c] = m_TargetGains[c];
        }
    }
}
