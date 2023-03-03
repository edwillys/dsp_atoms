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
    typedef struct
    {
        int32_t ch;
        int32_t el;
        int32_t type;
        float32_t freq;
        float32_t q;
        float32_t gainDb;
    }tAtomBiquadParams;
    /**
     * @brief Construct a new Biquad Atom object
     * 
     */
    CAtomBiquad(){};

    /**
     * @brief Destroy the Biquad Atom object
     * 
     */
    ~CAtomBiquad(){};

    /**
     * @brief 
     * 
     * @param props 
     * @return int32_t 
     */
    int32_t init(const CQuarkProps& props) override;

    /**
     * @brief 
     * 
     * @param in 
     * @param out 
     */
    void play(float32_t ** const in, float32_t ** const out) override;
    
    /**
     * @brief 
     * 
     */
    virtual void idle(void){};
    
    /**
     * @brief 
     * 
     * @param params 
     * @param len 
     */
    void set(void * params, cint32_t len) override;
};
