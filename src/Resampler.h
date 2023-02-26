#pragma once

#include "AudioTypes.h"
#include "UpFirDown.h"

class CResampler: public CUpFirDown 
{
private:
    int32_t m_Blocksize;

    /**
     * @brief Internal sanity check
     * 
     * @return bool_t 
     */
    bool_t checkFactor(int32_t);
public:
    /**
     * @brief Construct a new CResampler object. It is empty, as the one from 
     * CUpFirDown is taken
     */
    CResampler() {};

    /**
     * @brief Destroy the CResampler object. It is empty, as the one from 
     * CUpFirDown is taken
     */
    ~CResampler() {};

    /**
     * @brief Initialize
     * 
     * @param us_factor 
     * @param ds_factor 
     * @param blocksize 
     */
    void init(int32_t us_factor, int32_t ds_factor, int32_t blocksize);

    /**
     * @brief Sets the amount of samples per block
     * 
     * @param blocksize Amount of samples per block
     * @param force Force the operation, even if the current blocksize is the same
     */
    void setBlocksize( int32_t blocksize, bool_t force = false );

    /**
     * @brief Get the number of samples per block
     * 
     * @return int32_t number of samples per block
     */
    int32_t getBlocksize(void) { return m_Blocksize; };
};


