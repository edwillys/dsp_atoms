/**
 * @file Sample.h
 * @author Edgar Lubicz (edgarlubicz@gmail.com)
 * @brief 
 * @version 0.1
 * @date 2020-04-27
 * 
 * @copyright Copyright (c) 2020
 * 
 */

#pragma once

#include <string>
#include <vector>
#include <stdint.h>
#include <math.h>
#include "Resampler.h"

/**
 * @brief 
 * 
 */
class CSampleParams
{
public:
    std::string path;
    int32_t   lokey;
    int32_t   hikey;
    int32_t   lovel;
    int32_t   hivel;
    int32_t   ampveltrack;
    float32_t attack_s;
    float32_t decay_s;
    float32_t release_s;
    float32_t decay_gain;
    int32_t   fs;
    bool_t    append;

    /**
     * @brief Construct a new CSampleParams object
     * 
     */
    CSampleParams() : 
        lokey(0), hikey(0), lovel(0), hivel(0), decay_gain(1.0F),
        ampveltrack(0), attack_s(0.F), decay_s(0.0F), release_s(0.F), fs(48000),
        append(true)
        {};
    
    /**
     * @brief Destroy the CSampleParams object
     * 
     */
    ~CSampleParams() {};
};

class CSample
{
private:
    typedef enum
    {
        ATTACK = 0,
        DECAY,
        SUSTAIN,
        RELEASE,
        FADEOUT,
        FINISHED,
        READY
    }eSampleState;

    typedef struct
    {
        float32_t att_fac;
        float32_t dec_fac;
        float32_t rel_fac;
        int32_t att_smp;
        int32_t sus_smp;
        int32_t dec_smp;
        int32_t rel_smp;
        float32_t sus_lvl;
        int32_t num_smp;
    }tStatesADSR;
    
    std::vector<int16_t> m_Buffer;
    std::vector<int16_t> *m_PBuffer = NULL;
    int32_t m_CurrInd;
    float32_t m_Gain;

    int32_t m_SamplesCnt;

    int32_t m_Fs;

    int32_t m_LoKey;
    int32_t m_HiKey;
    int32_t m_LoVel;
    int32_t m_HiVel;
    int32_t m_AmpVelTrack;
    
    tStatesADSR m_StatesADSR;
    tStatesADSR m_StatesADSRNew;

    int32_t m_FadeoutSmp;
    float32_t m_FadeoutFac;

    eSampleState m_State;
    bool_t m_ShouldAppend;

    std::vector<float32_t> m_BufferResampler;
    CResampler m_Resampler;

    std::vector<int32_t> m_LenInSeq;
    int32_t m_LenInSeqCnt;

    /**
     * @brief Set the Default Values object
     * 
     */
    void setDefaultValues(void);

    /**
     * @brief 
     * 
     * @param secs 
     * @return int32_t 
     */
    int32_t secondsToSamples(cfloat32_t secs);

    /**
     * @brief 
     * 
     * @param secs 
     * @param delta 
     * @return float32_t 
     */
    float32_t secondsToFactor(cfloat32_t secs, cfloat32_t delta);

    /**
     * @brief 
     * 
     * @param samples 
     * @param delta 
     * @param fallback 
     * @return float32_t 
     */
    float32_t samplesToFactor(cint32_t samples, cfloat32_t delta, cfloat32_t fallback = 1.0F);

    /**
     * @brief 
     * 
     * @param p_blkout 
     * @param blocksize 
     * @return int32_t Number or processed samples
     */
    int32_t processBlockAttack(float32_t * const p_blkout, cint32_t blocksize);

    /**
     * @brief 
     * 
     * @param p_blkout 
     * @param blocksize 
     * @return int32_t Number or processed samples
     */
    int32_t processBlockSustain(float32_t * const p_blkout, cint32_t blocksize);

    /**
     * @brief 
     * 
     * @param p_blkout 
     * @param blocksize 
     * @return int32_t Number or processed samples
     */
    int32_t processBlockDecay(float32_t * const p_blkout, cint32_t blocksize);

    /**
     * @brief 
     * 
     * @param p_blkout 
     * @param blocksize  
     * @return int32_t Number or processed samples
     */
    int32_t processBlockRelease(float32_t * const p_blkout, cint32_t blocksize);

    /**
     * @brief 
     * 
     * @param p_blkout 
     * @param blocksize  
     * @return int32_t Number or processed samples
     */
    int32_t processBlockFadeout(float32_t * const p_blkout, cint32_t blocksize);
    
    /**
     * @brief 
     * 
     * @param p_blkout 
     * @param factor 
     * @param target 
     * @param blocksize
     */
    void processBlock(float32_t * const p_blkout, cfloat32_t factor, cfloat32_t target, 
        cint32_t blocksize);

public:
    class CADSR
    {
        public:
            float32_t att_s; 
            float32_t dec_s;
            float32_t rel_s;
            float32_t sus_lvl;

            CADSR() : 
                att_s(-1.0F), dec_s(-1.0F),
                rel_s(-1.0F), sus_lvl(-1.0F)
            {};
            CADSR(float32_t a, float32_t d, float32_t s, float32_t r) : 
                att_s(a), dec_s(d),
                sus_lvl(s), rel_s(r)
            {};
            
            ~CADSR(){};
    };

    /**
     * @brief 
     * 
     */
    cfloat32_t EXP_PCT = 0.99F;
    
    /**
     * @brief 
     * 
     */
    cfloat32_t EXP_PCT_FACTOR = log(1.0F - EXP_PCT);

    /**
     * @brief Construct a new CSample object
     * 
     */
    CSample() { setDefaultValues();};

    /**
     * @brief Construct a new CSample object
     * 
     * @param params 
     */
    CSample(CSampleParams const& params, cint32_t us = 1, cint32_t ds = 1);

    /**
     * @brief Construct a new CSample object
     * 
     * @param sample 
     * @param lokey 
     * @param hikey 
     * @param us 
     * @param ds 
     */
    CSample(CSample const& sample, cint32_t lokey=0, cint32_t hikey=0, cint32_t us=1, cint32_t ds=1);

    /**
     * @brief Destroy the CSample object
     * 
     */
    ~CSample(){};

    /**
     * @brief "Less than" operator overload to allow for sorting
     * 
     * @param sample 
     * @return true 
     * @return false 
     */
    inline bool operator < (const CSample& sample) const
    {
        return (m_LoVel < sample.m_LoVel);
    }

    /**
     * @brief 
     * 
     * @param sample 
     * @return true 
     * @return false 
     */
    inline bool operator == (const CSample& sample ) const
    { 
        return (m_LoKey == sample.m_LoKey) && (m_HiKey == sample.m_HiKey);
    }

    /**
     * @brief Get the Lo Key value
     * 
     * @return int32_t 
     */
    const int32_t getLowKey(void){ return m_LoKey;};


    /**
     * @brief Get the High Key value
     * 
     * @return int32_t 
     */
    const int32_t getHighKey(void){ return m_HiKey;};
    
    /**
     * @brief Get the Low Vel value
     * 
     * @return int32_t 
     */
    int32_t getLowVel(void){ return m_LoVel;};
    
    /**
     * @brief Get the High Vel value
     * 
     * @return int32_t 
     */
    int32_t getHighVel(void){ return m_HiVel;};

    /**
     * @brief Get how many samples are still to play
     * 
     * @return int32_t 
     */
    int32_t getNumSamplesUntilFinished(void);

    /**
     * @brief Play the current block and get the pointer to it
     * 
     * @param p_blk 
     * @param blocksize 
     */
    void play(float32_t * const p_blk, cint32_t blocksize);

    /**
     * @brief Turn off sample
     * 
     */
    void off(void);
    
    /**
     * @brief 
     * 
     */
    void on(void);

    /**
     * @brief 
     * 
     * @param adsr 
     */
    void setADSR(const CADSR& adsr);

    /**
     * @brief Set the fade out parameter
     * 
     * @param fade_s Fade out in seconds
     */
    void setFadeout(cfloat32_t fade_s);

    /**
     * @brief Set the Blocksize object
     * 
     * @param blocksize 
     * @param force 
     */
    void setBlocksize( int32_t blocksize, bool_t force = false );
};
