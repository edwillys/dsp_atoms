#include "AtomBiquad.h"

int32_t CAtomBiquad::init(const CQuarkProps &props)
{
    setProps(props);

    m_States = reinterpret_cast<float32_t *>(
        new tAtomBiquadStates[props.m_NumEl * props.m_NumChOut]());

    m_Coeffs = reinterpret_cast<float32_t *>(
        new tAtomBiquadCoeffs[props.m_NumEl * props.m_NumChOut]());

    return 0;
}

void CAtomBiquad::play(float32_t **const in, float32_t **const out)
{
    if (NULL != out && NULL != in)
    {
    }
}

void CAtomBiquad::set(void *params, cint32_t len)
{
    if (NULL != params && len > 0)
    {
        tAtomBiquadParams *p_biq_params = reinterpret_cast<tAtomBiquadParams *>(params);
    }
}

void CAtomBiquad::calculateCoeffsCookbook(const tAtomBiquadParams &params)
{
    if (nullptr != m_Coeffs &&
        params.ch < m_Props.m_NumChIn &&
        params.el < m_Props.m_NumEl && params.type < NUM_BIQT)
    {
        // intermediate variables
        float32_t A;
        float32_t a0;
        tAtomBiquadCoeffs *coeffs = reinterpret_cast<tAtomBiquadCoeffs *>(m_Coeffs);

        if (params.type == BIQT_HSH || params.type == BIQT_LSH || params.type == BIQT_PEAK)
        {
            A = sqrtf(powf(10.F, (params.gainDb * 0.025F)));
        }
        else
        {
            A = sqrtf(powf(10.F, (params.gainDb * 0.05F)));
        }

        float32_t w0 = 2.F * (float32_t)M_PI * params.freq / m_Props.m_Fs;
        float32_t cosw0 = cosf(w0);
        float32_t sinw0 = sinf(w0);
        float32_t alpha = sinw0 / (2.F * params.q);

        switch (params.type)
        {
        case BIQT_BYPASS:
        {
            a0 = 1.0F;
            coeffs->a1 = 0.0F;
            coeffs->a2 = 0.0F;
            coeffs->b0 = 1.0F;
            coeffs->b1 = 0.0F;
            coeffs->b2 = 0.0F;
        }
        break;
        case BIQT_LPF_6DB:
        {
            float32_t K = sinw0 / (1.F + cosw0); // tan(w0/2)
            a0 = K + 1.F;
            coeffs->a1 = -(K + 1.F);
            coeffs->a2 = 0.0F;
            coeffs->b0 = 1.0F;
            coeffs->b1 = 1.0F;
            coeffs->b2 = 0.0F;
        }
        break;
        case BIQT_LPF:
        {
            coeffs->b0 = (1.F - cosw0) * 0.5F;
            coeffs->b1 = 1.F - cosw0;
            coeffs->b2 = (1.F - cosw0) * 0.5F;
            a0 = 1.F + alpha;
            coeffs->a1 = -2.F * cosw0;
            coeffs->a2 = 1.F - alpha;
        }
        break;
        case BIQT_HPF_6DB:
        {
            a0 = 1.0F;
            coeffs->a1 = 0.0F;
            coeffs->a2 = 0.0F;
            coeffs->b0 = 1.0F;
            coeffs->b1 = 0.0F;
            coeffs->b2 = 0.0F;
        }
        break;
        case BIQT_HPF:
        {
            coeffs->b0 = (1.F + cosw0) * 0.5F;
            coeffs->b1 = -(1.F + cosw0);
            coeffs->b2 = (1.F + cosw0) * 0.5F;
            a0 = 1.F + alpha;
            coeffs->a1 = -2.F * cosw0;
            coeffs->a2 = 1.F - alpha;
        }
        break;
        case BIQT_PEAK:
        {
            coeffs->b0 = 1.F + alpha * A;
            coeffs->b1 = -2.F * cosw0;
            coeffs->b2 = 1.F - alpha * A;
            a0 = 1.F + alpha / A;
            coeffs->a1 = -2.F * cosw0;
            coeffs->a2 = 1.F - alpha / A;
        }
        break;
        case BIQT_NOTCH:
        {
            coeffs->b0 = 1.F;
            coeffs->b1 = -2.F * cosw0;
            coeffs->b2 = 1.F;
            a0 = 1.F + alpha;
            coeffs->a1 = -2.F * cosw0;
            coeffs->a2 = 1.F - alpha;
        }
        break;
        case BIQT_BPF:
        {
            coeffs->b0 = alpha;
            coeffs->b1 = 0.F;
            coeffs->b2 = -alpha;
            a0 = 1.F + alpha;
            coeffs->a1 = -2.F * cosw0;
            coeffs->a2 = 1.F - alpha;
        }
        break;
        case BIQT_HSH:
        {
            float32_t Ap1 = A + 1.F;
            float32_t Am1 = A - 1.F;
            float32_t sqrtA = sqrtf(A);
            coeffs->b0 = A * (Ap1 + Am1 * cosw0 + 2.F * sqrtA * alpha);
            coeffs->b1 = -2.F * A * (Am1 + Ap1 * cosw0);
            coeffs->b2 = A * (Ap1 + Am1 * cosw0 - 2.F * sqrtA * alpha);
            a0 = Ap1 - Am1 * cosw0 + 2.F * sqrtA * alpha;
            coeffs->a1 = 2.F * (Am1 - Ap1 * cosw0);
            coeffs->a2 = Ap1 - Am1 * cosw0 - 2.F * sqrtA * alpha;
        }
        break;
        case BIQT_LSH:
        {
            float32_t Ap1 = A + 1.F;
            float32_t Am1 = A - 1.F;
            float32_t sqrtA = sqrtf(A);
            coeffs->b0 = A * (Ap1 - Am1 * cosw0 + 2.F * sqrtA * alpha);
            coeffs->b1 = 2.F * A * (Am1 - Ap1 * cosw0);
            coeffs->b2 = A * (Ap1 - Am1 * cosw0 - 2.F * sqrtA * alpha);
            a0 = Ap1 + Am1 * cosw0 + 2.F * sqrtA * alpha;
            coeffs->a1 = -2.F * (Am1 + Ap1 * cosw0);
            coeffs->a2 = Ap1 + Am1 * cosw0 - 2.F * sqrtA * alpha;
        }
        break;
        case BIQT_APF_180:
        {
            a0 = 1.0F;
            coeffs->a1 = 0.0F;
            coeffs->a2 = 0.0F;
            coeffs->b0 = 1.0F;
            coeffs->b1 = 0.0F;
            coeffs->b2 = 0.0F;
        }
        break;
        case BIQT_APF_360:
        {
            coeffs->b0 = 1.F - alpha;
            coeffs->b1 = -2.F * cosw0;
            coeffs->b2 = 1.F + alpha;
            a0 = 1.F + alpha;
            coeffs->a1 = -2.F * cosw0;
            coeffs->a2 = 1.F - alpha;
        }
        break;
        default:
            break;
        }

        coeffs->b0 /= a0;
        coeffs->b1 /= a0;
        coeffs->b2 /= a0;
        coeffs->a1 /= a0;
        coeffs->a2 /= a0;
    }
}