#pragma once

#include "AudioAtom.h"

/**
 * @brief
 *
 */
class CAtomBiquad : public CQuark<float32_t>
{
protected:


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
        BIQT_APF_360,
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

    typedef struct
    {
        float32_t b0;
        float32_t b1;
        float32_t b2;
        float32_t a1;
        float32_t a2;
    } tAtomBiquadCoeffs;

    typedef struct
    {
        float32_t z_1;
        float32_t z_2;
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
    virtual void idle(void){};

    /**
     * @brief See base class definition
     */
    void set(void *params, cint32_t len) override;

    void calculateCoeffsCookbook(const tAtomBiquadParams &params);
};
