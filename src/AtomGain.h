#pragma once

#include "AudioAtom.h"

/**
 * @brief
 *
 */
class CAtomGain : public CAudioQuarkLinearMorph<float32_t>
{
public:
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

    float32_t *m_TargetGains = nullptr;
    float32_t *m_DeltaGains = nullptr;
    float32_t *m_Gains = nullptr;
};
