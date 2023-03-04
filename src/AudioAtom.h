#pragma once

#include "AudioTypes.h"

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

    CQuarkProps(void)
    {
        m_Fs = 48000;
        m_BlockSize = 64;
        m_NumChIn = 0;
        m_NumChOut = 0;
        m_NumCtrlIn = 0;
        m_NumCtrlOut = 0;
        m_NumEl = 0;
    }

    ~CQuarkProps() {}
};

/**
 * @brief
 *
 */
template <class T>
class CQuark
{
protected:
    CQuarkProps m_Props;

public:
    /**
     * @brief Construct a new Audio Atom object
     *
     */
    CQuark(){};

    /**
     * @brief Destroy the Audio Atom object
     *
     */
    ~CQuark(){};

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
     */
    virtual void idle(void){};

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
