#pragma once

#include "AudioAtom.h"

/**
 * @brief
 *
 */
class CAtomBiquad : public CAudioQuarkLinearMorph<float32_t>
{
public:
    enum eBiquadType
    {
        BIQT_BYPASS = 0,
        BIQT_LPF_6DB,
        BIQT_LPF,
        BIQT_HPF_6DB,
        BIQT_HPF,
        BIQT_PEAK,
        BIQT_NOTCH,
        BIQT_BPF,
        BIQT_HSH,
        BIQT_LSH,
        BIQT_APF_180,
        BIQT_APF,
        NUM_BIQT,
    };

    typedef struct
    {
        int32_t ch;
        int32_t el;
        int32_t type;
        float32_t freq;
        float32_t q;
        float32_t gainDb;
    } tAtomBiquadParams;

    typedef struct tAtomBiquadCoeffs
    {
        float32_t b0;
        float32_t b1;
        float32_t b2;
        float32_t a1;
        float32_t a2;

        tAtomBiquadCoeffs &operator+=(const tAtomBiquadCoeffs &rhs)
        {
            this->a1 += rhs.a1;
            this->a2 += rhs.a2;
            this->b0 += rhs.b0;
            this->b1 += rhs.b1;
            this->b2 += rhs.b2;

            return *this; // return the result by reference
        }

        friend tAtomBiquadCoeffs operator-(tAtomBiquadCoeffs lhs,
                                           const tAtomBiquadCoeffs &rhs)
        {
            lhs.a1 -= rhs.a1;
            lhs.a2 -= rhs.a2;
            lhs.b0 -= rhs.b0;
            lhs.b1 -= rhs.b1;
            lhs.b2 -= rhs.b2;
            return lhs; // return the result by value (uses move constructor)
        }

        friend tAtomBiquadCoeffs operator/(tAtomBiquadCoeffs lhs,
                                           float32_t factor)
        {
            lhs.a1 /= factor;
            lhs.a2 /= factor;
            lhs.b0 /= factor;
            lhs.b1 /= factor;
            lhs.b2 /= factor;
            return lhs; // return the result by value (uses move constructor)
        }
    } tAtomBiquadCoeffs;

    typedef struct
    {
        float32_t s1;
        float32_t s2;
    } tAtomBiquadStates;

    /**
     * @brief See base class definition
     */
    int32_t init(const CQuarkProps &props) override;

    /**
     * @brief See base class definition
     */
    void play(float32_t **const in, float32_t **const out) override;

    /**
     * @brief See base class definition
     */
    void set(void *params, cint32_t len) override;

    /**
     * @brief Calculate the biquad ai bi coefficients
     *
     * @param params
     *
     * For all of the pre-calculated filter coefficients in the Audio EQ Cookbook
     * (https://webaudio.github.io/Audio-EQ-Cookbook/Audio-EQ-Cookbook.txt)
     * we apply the values directly. For the remainig ones (1-pole filters), we
     * calculate it in a similar fashion, by using the pre warped BLT substitution:
     *
     *              1         1 - z^-1
     * s  <--  ----------- * ----------
     *           tan(w0/2)     1 + z^-1
     *
     *   and the identity:
     *
     *               sin(w0)
     * tan(w0/2) = -------------
     *              1 + cos(w0)
     *
     */
    void calculateCoeffsCookbook(const tAtomBiquadParams &params);

protected:
    void calculateDeltas(void) override;

    tAtomBiquadCoeffs **m_TargetCoeffs = nullptr;
    tAtomBiquadCoeffs **m_DeltaCoeffs = nullptr;
    tAtomBiquadCoeffs **m_Coeffs = nullptr;
    tAtomBiquadStates **m_TargetStates = nullptr;
    tAtomBiquadStates **m_DeltaStates = nullptr;
    tAtomBiquadStates **m_States = nullptr;
};
