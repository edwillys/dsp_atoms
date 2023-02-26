#pragma once

#include "AudioTypes.h"
#include <vector>

class CAudioAtomProps
{
public:
    int32_t m_Fs;
    int32_t m_BlockSize;
    int32_t m_NumChIn;
    int32_t m_NumChOut;
    int32_t m_NumCtrlIn;
    int32_t m_NumCtrlOut;
    
    CAudioAtomProps(void)
    {
        m_Fs = 48000;
        m_BlockSize = 64;
        m_NumChIn = 0;
        m_NumChOut = 0;
        m_NumCtrlIn = 0;
        m_NumCtrlOut = 0;
    }
    
    ~CAudioAtomProps(){}
};

/**
 * @brief 
 * 
 */
//template <class T>
class CAudioAtom
{
protected:
    CAudioAtomProps m_Props;

public:

    /**
     * @brief Construct a new Audio Atom object
     * 
     */
    CAudioAtom(){};

    /**
     * @brief Destroy the Audio Atom object
     * 
     */
    ~CAudioAtom(){};

    /**
     * @brief 
     * 
     * @param props 
     * @return int32_t 
     */
    virtual int32_t init(const CAudioAtomProps& props) { setProps(props); return 0; };
    
    /**
     * @brief 
     * 
     * @param in 
     * @param out 
     */
    virtual void play(float32_t ** const in, float32_t ** const out) = 0;
    
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
    virtual void mute(float32_t ** const out);
    
    /**
     * @brief 
     * 
     * @param in 
     * @param out 
     */
    virtual void bypass(float32_t ** const in, float32_t ** const out);

    /**
     * @brief 
     * 
     * @param index 
     * @param value 
     */
    virtual void set(cint32_t index, cint32_t value){};
    
    /**
     * @brief 
     * 
     * @param index 
     * @param values 
     */
    virtual void set(cint32_t index, const std::vector<int32_t>& values){};
    
    /**
     * @brief 
     * 
     * @param index 
     * @param value 
     */
    virtual void set(cint32_t index, cfloat32_t value){};

    /**
     * @brief 
     * 
     * @param index 
     * @param values 
     */
    virtual void set(cint32_t index, const std::vector<float32_t>& values){};

    /**
     * @brief Get the object properties
     * 
     * @return const CAudioAtomProps& 
     */
    const CAudioAtomProps& getProps(void) {return m_Props; };
    
    /**
     * @brief Set the object properties
     * 
     * @param props 
     */
    void setProps(const CAudioAtomProps& props) { m_Props = props; };
};
