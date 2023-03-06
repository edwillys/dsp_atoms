#pragma once

#include <stdio.h>
#include <string.h>
#include "AudioTypes.h"

#define SET_ALL_CH_IND (-1)
#define SET_ALL_EL_IND (-1)

class CQuarkProps
{
public:
    int32_t m_Fs;
    int32_t m_BlockSize;
    int32_t m_NumChIn;
    int32_t m_NumChOut;
    int32_t m_NumCtrlIn;
    int32_t m_NumCtrlOut;
    int32_t m_NumEl;

    CQuarkProps(
        int32_t fs = 48000,
        int32_t blockSize = 64,
        int32_t numChIn = 0,
        int32_t numChOut = 0,
        int32_t numCtrlIn = 0,
        int32_t numCtrlOut = 0,
        int32_t numEl = 0
    ) : 
        m_Fs(fs),
        m_BlockSize(blockSize),
        m_NumChIn(numChIn),
        m_NumChOut(numChOut),
        m_NumCtrlIn(numCtrlIn),
        m_NumCtrlOut(numCtrlOut),
        m_NumEl(numEl)
    {}

    ~CQuarkProps() {}
};

/**
 * @brief
 *
 */
template <class T>
class CAudioQuark
{
protected:
    CQuarkProps m_Props;

public:
    /**
     * @brief Construct a new Audio Atom object
     *
     */
    CAudioQuark(){};

    /**
     * @brief Destroy the Audio Atom object
     *
     */
    ~CAudioQuark(){};

    /**
     * @brief
     *
     * @param props
     * @return int32_t
     */
    virtual int32_t init(const CQuarkProps &props)
    {
        setProps(props);
        return 0;
    };

    /**
     * @brief
     *
     * @param in
     * @param out
     */
    virtual void play(T **const in, T **const out) = 0;

    /**
     * @brief
     *
     * @param out
     */
    virtual void mute(T **const out)
    {
        if (NULL != out)
        {
            for (int32_t ch = 0; ch < m_Props.m_NumChOut; ch++)
            {
                for (int32_t i = 0; i < m_Props.m_BlockSize; i++)
                    out[ch][i] = static_cast<T>(0);
            }
        }
    };

    /**
     * @brief
     *
     * @param in
     * @param out
     */
    virtual void bypass(T **const in, T **const out)
    {
        if (NULL != out)
        {
            if (NULL == in)
            {
                mute(out);
            }
            else
            {
                int32_t ind_max = MAX(m_Props.m_NumChOut, m_Props.m_NumChIn);
                for (int32_t ch = 0; ch < ind_max; ch++)
                {
                    for (int32_t i = 0; i < m_Props.m_BlockSize; i++)
                        out[ch][i] = in[ch][i];
                }
                for (int32_t ch = ind_max; ch < m_Props.m_NumChOut; ch++)
                {
                    for (int32_t i = 0; i < m_Props.m_BlockSize; i++)
                        out[ch][i] = static_cast<T>(0);
                }
            }
        }
    };

    /**
     * @brief
     *
     * @param ch
     * @param el
     * @param value
     */
    virtual void set(cint32_t ch, cint32_t el, cint32_t value){};

    /**
     * @brief
     *
     * @param ch
     * @param el
     * @param value
     */
    virtual void set(cint32_t ch, cint32_t el, cfloat32_t value){};

    /**
     * @brief
     *
     * @param params
     * @param len
     */
    virtual void set(void *params, cint32_t len){};

    /**
     * @brief Get the object properties
     *
     * @return const CQuarkProps&
     */
    const CQuarkProps &getProps(void) { return m_Props; };

    /**
     * @brief Set the object properties
     *
     * @param props
     */
    void setProps(const CQuarkProps &props) { m_Props = props; };
};

template <class T>
class CAudioQuarkLinearMorph : public CAudioQuark<T>
{
public:
    void setMorphMs(const float32_t morphTimeMs)
    {
        m_MorphBlocksizeTotal = 0;
        m_MorphBlocksizeMs = 0.F;
        if (morphTimeMs > 0.F)
        {
            float32_t blockMs = (m_Props.m_BlockSize / (float32_t)m_Props.m_Fs);
            int32_t numBlocks = (int32_t)(morphTimeMs / blockMs);

            if (numBlocks > 0)
            {
                m_MorphBlocksizeTotal = numBlocks;
                m_MorphBlocksizeMs = numBlocks * blockMs;
            }
        }

        m_MorphBlocksizeCnt = 0;

        calculateDeltas();
    }

protected:
    virtual void calculateDeltas(void) = 0;

    int32_t m_MorphBlocksizeCnt = 0;
    int32_t m_MorphBlocksizeTotal = 0;
    float32_t m_MorphBlocksizeMs = 0.F;
};
