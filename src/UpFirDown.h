#pragma once

#include "AudioTypes.h"
#include <vector>
#include <math.h>

/**
 * @brief Upsample, FIR filter, and downsample
 * 
 */
class CUpFirDown
{
protected:
    float32_t *m_Buffer = NULL;
    float32_t *m_BufferScratchIn = NULL;
    float32_t *m_BufferScratchOut = NULL;
    std::vector<float32_t> m_FirTrans;
    int32_t m_PhaseLen = 0;
    int32_t m_LenInMax = 0;
    int32_t m_UsFactor = 1;
    int32_t m_DsFactor = 1;
    int32_t m_LenOut = 0;
    int32_t m_OffsetOut = 0;
    int32_t m_OffsetOutMax = 0;
    bool_t m_Bypass = true;

public:
    /**
     * @brief Struct for length of input samples
     * 
     */
    typedef struct 
    {
        int32_t min;
        int32_t max;
        int32_t num_min;
        int32_t num_max;
    }tLenIn;

    /**
     * @brief Construct a new CUpFirDown object
     * 
     */
    CUpFirDown() {}

    /**
     * @brief Destroy the CUpFirDown object
     * 
     */
    ~CUpFirDown() { deinit(); };

    /**
     * @brief Apply upsampling, FIR and downsampling for a given number of input samples
     * 
     * @param in Pointer to input samples
     * @param numin Number of input samples
     * @return float32_t* Pointer to output buffer
     */
    float32_t *apply(cfloat32_t * const in, cint32_t numin );

    /**
     * @brief Same as apply(cfloat32_t * const, cint32_t ), but with fixed number of 
     *        input samples: m_Blocksize
     * 
     * @param in Pointer to input samples
     * @return float32_t* Pointer to output buffer
     */
    float32_t *apply(cfloat32_t * const in);

    /**
     * @brief Initialize internal buffers
     * 
     * @param us_factor Upsampling factor
     * @param ds_factor Downsampling factor
     * @param blocksize Number of output samples
     * @param fir FIR coefficients
     */
    void init(int32_t us_factor, int32_t ds_factor, int32_t blocksize, const std::vector<float32_t> &fir);

    /**
     * @brief De-initialize internal buffers. Protected against multiple calls.
     * 
     */
    void deinit(void);

    /**
     * @brief Get the Len Out object
     * 
     * @param us 
     * @param ds 
     * @param numin 
     * @return cint32_t 
     */
    static cint32_t getLenOut( int32_t us, int32_t ds, cint32_t numin);
    
    /**
     * @brief Get the input len struct, for a given upsampling/downsampling factor and 
     *        number of output samples
     * 
     * @param us Upsampling factor
     * @param ds Downsampling faxctor
     * @param numout Number of output samples
     * @return tLenIn const 
     */
    static tLenIn const getLenIn( int32_t us, int32_t ds, cint32_t numout);

    /**
     * @brief The non static version of above, using m_UsFactor and m_DsFactor
     * 
     * @param numout 
     * @return tLenIn const 
     */
    tLenIn const getLenIn( cint32_t numout);

    /**
     * @brief Binary GCD, straight algorithm from Wikipedia
     * 
     * @param u 
     * @param v 
     * @return uint32_t 
     */
    static uint32_t binaryGCD(uint32_t u, uint32_t v);

    /**
     * @brief Check if instance is configured with bypass parameters
     * 
     * @return bool_t 
     */
    bool_t isBypass(void) { return m_Bypass; };

};
