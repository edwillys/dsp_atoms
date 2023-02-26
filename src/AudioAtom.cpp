#include "AudioAtom.h"
#include <algorithm>

void CAudioAtom::mute(float32_t ** const out)
{
    if (NULL != out)
    {
        for(int32_t ch = 0; ch < m_Props.m_NumChOut; ch++)
        {
            for(int32_t i = 0; i < m_Props.m_BlockSize; i++)
                out[ch][i] = 0.0F;
        }
    }
}

void CAudioAtom::bypass(float32_t ** const in, float32_t ** const out)
{
    if (NULL != out)
    {
        if ( NULL == in )
        {
            mute(out);
        }
        else
        {
            int32_t ind_max = std::max(m_Props.m_NumChOut, m_Props.m_NumChIn);
            for(int32_t ch = 0; ch < ind_max; ch++)
            {
                for(int32_t i = 0; i < m_Props.m_BlockSize; i++)
                    out[ch][i] = in[ch][i];
            }
            for(int32_t ch = ind_max; ch < m_Props.m_NumChOut; ch++)
            {
                for(int32_t i = 0; i < m_Props.m_BlockSize; i++)
                    out[ch][i] = 0.0F;
            }
        }
    }
}

