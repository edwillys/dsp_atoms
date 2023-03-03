#include "AtomBiquad.h"

int32_t CAtomBiquad::init(const CQuarkProps& props) 
{ 
    setProps(props); 
    
    return 0;  
}

void CAtomBiquad::play(float32_t ** const in, float32_t ** const out)
{
    if (NULL != out && NULL != in)
    {

    }
}

void CAtomBiquad::set(void * params, cint32_t len)
{
    if( NULL != params && len > 0)
    {
        tAtomBiquadParams *p_biq_params = reinterpret_cast<tAtomBiquadParams *>(params);
    }
}