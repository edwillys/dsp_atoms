#pragma once

#include "AudioAtom.h"

/**
 * @brief
 *
 */
class CAtomDiode : public CAudioQuarkLinearMorph<float32_t>
{
public:
    enum eDiodeTypes
    {
        DIODE_T_1N4148 = 0,
        NUM_DIODE_T,
    };

    typedef struct
    {
        float32_t IS;
        float32_t N;
        float32_t RS;
        float32_t CJO;
        float32_t VJ;
        float32_t M;
        float32_t TT;
    } tAtomDiodeSpiceParams;

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
    void set(cint32_t ch, cint32_t el, cfloat32_t value) override;

protected:
    void calculateDeltas(void) override;

    // Diode parameters
    static const tAtomDiodeSpiceParams m_DiodeParams[NUM_DIODE_T];

    // Physical constants
    const float32_t m_k = 1.38e-23F; // Boltzmann
    const float32_t m_q = 1.6e-19F;  // electron charge
    const float32_t m_TempC = 20.F;  // Temperature in Celcius
    const float32_t m_Vt = m_k * (273.F + m_TempC) / m_q;

    float32_t *m_TargetA0 = nullptr;
    float32_t *m_DeltaA0 = nullptr;
    float32_t *m_A0 = nullptr;
    float32_t *m_TargetLogA0A1 = nullptr;
    float32_t *m_DeltaLogA0A1 = nullptr;
    float32_t *m_LogA0A1 = nullptr;

    float32_t *m_MakeUpGains = nullptr;
    float32_t *m_MakeUpDeltaGains = nullptr;
    float32_t *m_MakeUpTargetGains = nullptr;

    eDiodeTypes m_Mode = DIODE_T_1N4148;
    bool_t m_Normalize = false;
};
