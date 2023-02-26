#include "Sample.h"
#include <string>
#include <iostream>
#include <algorithm>
#include <math.h>
#include <sndfile.h>

CSample::CSample(CSampleParams const& params, cint32_t us, cint32_t ds)
{
    // initialize everyone to default first of all, in case things go weird
    setDefaultValues();
    
    // proceed to WAV file...
    SF_INFO info;
    SNDFILE* const sndfile = sf_open(params.path.c_str(), SFM_READ, &info);
    if (!sndfile || !info.frames || (info.channels != 1) || info.samplerate != params.fs) 
    {
        std::cerr << "Failed to open sample " << params.path << std::endl;
        sf_close(sndfile);
    }
    else
    {
        // Read data
        float32_t * const data = new float32_t[info.frames];
        if (!data) 
        {
            std::cerr << "Failed to allocate memory for sample" << std::endl;
            sf_close(sndfile);
        }
        else
        {
            sf_seek(sndfile, 0ul, SEEK_SET);
            sf_read_float(sndfile, data, info.frames);
            sf_close(sndfile);
            //std::cout << "Adding sample from " << params.path << std::endl;
            m_Buffer.resize(info.frames);
            m_PBuffer = &m_Buffer;
            for(int32_t i = 0; i < info.frames; i++)
            {
                float32_t sample = data[i];
                if( sample > 0.0F )
                    m_Buffer[i] = (int16_t)( ((float32_t)INT16_MAX * sample) + 0.5F );
                else
                    m_Buffer[i] = (int16_t)( ((float32_t)INT16_MIN * -sample) - 0.5F );
            }

            m_ShouldAppend = params.append;

            // override some default values for class members
            m_LoKey = params.lokey;
            m_HiKey = params.hikey;
            m_LoVel = params.lovel;
            m_HiVel = params.hivel;
            m_AmpVelTrack = params.ampveltrack;
            m_Fs = params.fs;

            CADSR adsr;
            adsr.att_s = params.attack_s;
            adsr.rel_s = params.release_s;
            adsr.dec_s = params.decay_s;
            adsr.sus_lvl = params.decay_gain;
            setADSR(adsr);

            m_Resampler.init(us, ds, 0);
            //setBlocksize(m_ResamplerBlocksize, true);
            delete data;
        }
    }
}

CSample::CSample(CSample const& sample, cint32_t lokey, cint32_t hikey, cint32_t us, cint32_t ds)
{
    // set baseline
    setDefaultValues();

    // overwrite needed params
    m_ShouldAppend = sample.m_ShouldAppend;
    
    m_LoKey = (lokey > 0) ? lokey : sample.m_LoKey;
    m_HiKey = (hikey > 0) ? hikey : sample.m_HiKey;
    m_LoVel = sample.m_LoVel;
    m_HiVel = sample.m_HiVel;
    m_AmpVelTrack = sample.m_AmpVelTrack;
    m_Fs = sample.m_Fs;

    m_StatesADSRNew = sample.m_StatesADSRNew;
    m_StatesADSR = sample.m_StatesADSR;

    m_Buffer.resize(0);
    m_PBuffer = sample.m_PBuffer;
    m_Resampler.init(us, ds, 0);
}

float32_t CSample::samplesToFactor(cint32_t samples, cfloat32_t delta, cfloat32_t fallback)
{
    float32_t factor = fallback;
    if (samples > 0 && delta > 0.0)
    {
        auto exponent = EXP_PCT_FACTOR / (float32_t)(samples + 1);
        factor = exp( exponent );
    }
    return factor;
}

float32_t CSample::secondsToFactor(cfloat32_t secs, cfloat32_t delta)
{
    float32_t factor = 1.0F;
    if (secs > 0.0F)
    {
        cfloat32_t period = 1.0F / (float32_t)m_Fs;
        factor = exp(- period / secs);
    }
    return factor;
}

int32_t CSample::secondsToSamples(float32_t secs)
{
    return (int32_t)round((secs * (float32_t)m_Fs));
}

void CSample::setDefaultValues(void) 
{
    m_LenInSeq = {0};
    m_LenInSeqCnt = 0;
    m_LoKey = 0;
    m_HiKey = 0;
    m_LoVel = 0;
    m_HiVel = 0;
    m_AmpVelTrack = 0;
    m_Fs = 48000;
    m_CurrInd = 0;
    m_SamplesCnt = 0;
    m_State = READY, 
    m_Gain = 0.0F;
    m_StatesADSR.att_fac = 1.0F;
    m_StatesADSR.rel_fac = 1.0F;
    m_StatesADSR.att_fac = 1.0F;
    m_StatesADSR.dec_fac = 1.0F;
    m_StatesADSR.rel_fac = 1.0F;
    m_StatesADSR.att_smp = 0;
    m_StatesADSR.dec_smp = 0;
    m_StatesADSR.rel_smp = 0;
    m_StatesADSR.sus_smp = 0;
    m_StatesADSR.sus_lvl = 1.0F;
    m_StatesADSR.num_smp = 0;
    m_FadeoutFac = 0.0F;
    m_FadeoutSmp = 0;
    m_ShouldAppend = true;
}

int32_t CSample::getNumSamplesUntilFinished(void) 
{
    int32_t retval = m_StatesADSR.num_smp - m_CurrInd;
    // If the sample has finished earlier for some reason (for example: note turned off)
    if (m_State == FINISHED)
        retval = 0;

    return retval;
}

int32_t CSample::processBlockAttack(float32_t * const p_blkout, cint32_t blocksize) 
{
    int32_t bs = std::min(blocksize, m_StatesADSR.att_smp - m_SamplesCnt);
    processBlock(p_blkout, m_StatesADSR.att_fac, 1.0F, bs);

    if( m_CurrInd >= (*m_PBuffer).size())
    {
        m_SamplesCnt = 0;
        m_Gain = 0.0F;
        m_State = FINISHED;
    }
    else if( m_SamplesCnt >= m_StatesADSR.att_smp )
    {
        m_SamplesCnt = 0;
        if (m_StatesADSR.dec_smp > 0)
        {
            m_Gain = 1.0F;
            m_State = DECAY;
        }
        else if (m_StatesADSR.sus_smp > 0)
        {
            m_Gain = m_StatesADSR.sus_lvl;
            m_State = SUSTAIN;
        }
        else if (m_StatesADSR.rel_smp > 0)
        {
            m_Gain = m_StatesADSR.sus_lvl;
            m_State = RELEASE;
        }
        else
        {
            //m_Gain = 0.0F;
            m_State = FADEOUT;
        }
    }

    return bs;
}

int32_t CSample::processBlockDecay(float32_t * const p_blkout, cint32_t blocksize) 
{
    int32_t bs = std::min(blocksize, m_StatesADSR.dec_smp - m_SamplesCnt);
    processBlock(p_blkout, m_StatesADSR.dec_fac, m_StatesADSR.sus_lvl, bs);
    if( m_CurrInd >= (*m_PBuffer).size())
    {
        m_SamplesCnt = 0;
        m_Gain = 0.0F;
        m_State = FINISHED;
    }
    else if( m_SamplesCnt >= m_StatesADSR.dec_smp )
    {
        m_SamplesCnt = 0;
        if (m_StatesADSR.sus_smp > 0)
        {
            m_Gain = m_StatesADSR.sus_lvl;
            m_State = SUSTAIN;
        }
        else if (m_StatesADSR.rel_smp > 0)
        {
            m_Gain = m_StatesADSR.sus_lvl;
            m_State = RELEASE;
        }
        else if (m_FadeoutSmp > 0)
        {
            m_State = FADEOUT;
        }
        else
        {
            m_Gain = 0.0F;
            m_State = FINISHED;
        }
    }

    return bs;
}

int32_t CSample::processBlockSustain(float32_t * const p_blkout, cint32_t blocksize) 
{
    int32_t bs = std::min(blocksize, m_StatesADSR.sus_smp - m_SamplesCnt);
    processBlock(p_blkout, 1.0F, 0.0F, bs);

    if( m_CurrInd >= (*m_PBuffer).size())
    {
        m_SamplesCnt = 0;
        m_Gain = 0.0F;
        m_State = FINISHED;
    }
    else if( m_SamplesCnt >= m_StatesADSR.sus_smp )
    {
        m_SamplesCnt = 0;
        if (m_StatesADSR.rel_smp > 0)
        {
            m_Gain = m_StatesADSR.sus_lvl;
            m_State = RELEASE;
        }
        else if (m_FadeoutSmp > 0)
        {
            m_State = FADEOUT;
        }
        else
        {
            m_Gain = 0.0F;
            m_State = FINISHED;
        }
    }

    return bs;
}

int32_t CSample::processBlockRelease(float32_t * const p_blkout, cint32_t blocksize) 
{
    int32_t bs = std::min(blocksize, m_StatesADSR.rel_smp - m_SamplesCnt);
    processBlock(p_blkout, m_StatesADSR.rel_fac, 0.0F, bs);

    if( m_CurrInd >= (*m_PBuffer).size())
    {
        m_SamplesCnt = 0;
        m_Gain = 0.0F;
        m_State = FINISHED;
    }
    else if( m_SamplesCnt >= m_StatesADSR.rel_smp )
    {
        m_SamplesCnt = 0;
        if (m_FadeoutSmp > 0)
        {
            m_State = FADEOUT;
        }
        else
        {
            m_Gain = 0.0F;
            m_State = FINISHED;
        }
    }

    return bs;
}

int32_t CSample::processBlockFadeout(float32_t * const p_blkout, cint32_t blocksize) 
{
    int32_t bs = std::min(blocksize, m_FadeoutSmp - m_SamplesCnt);
    processBlock(p_blkout, m_FadeoutFac, 0.0F, bs);

    if( m_CurrInd >= (*m_PBuffer).size() 
        ||  m_SamplesCnt >= m_FadeoutSmp )
    {
        m_SamplesCnt = 0;
        m_Gain = 0.0F;
        m_State = FINISHED;
    }

    return bs;
}

void CSample::processBlock(float32_t * const p_blkout, cfloat32_t factor, cfloat32_t target, 
    cint32_t blocksize) 
{
    int16_t *p_blkin = &(*m_PBuffer)[m_CurrInd];
    cfloat32_t factor_minus1 =  (1.0F - factor);

    if(m_ShouldAppend)
    {
        for(auto i = 0; i < blocksize; i++)
        {
            cint16_t samplei16 = p_blkin[i];
            // += since each sample is added to the polyphony
            cfloat32_t neg = ((uint16_t)samplei16 & 0x8000U) >> 15U;
            cfloat32_t pos = !neg;

            float32_t sample = pos * (float32_t)p_blkin[i] / (float32_t)INT16_MAX + 
                            neg * (float32_t)p_blkin[i] / ((float32_t)-INT16_MIN);
            p_blkout[i] += sample * m_Gain;
            m_Gain = factor * m_Gain + factor_minus1 * target;
        }
    }
    else
    {
        for(auto i = 0; i < blocksize; i++)
        {
            cint16_t samplei16 = p_blkin[i];
            // += since each sample is added to the polyphony
            cfloat32_t neg = ((uint16_t)samplei16 & 0x8000U) >> 15U;
            cfloat32_t pos = !neg;

            float32_t sample = pos * (float32_t)p_blkin[i] / (float32_t)INT16_MAX + 
                            neg * (float32_t)p_blkin[i] / ((float32_t)-INT16_MIN);
            p_blkout[i] = sample * m_Gain;
            m_Gain = factor * m_Gain + factor_minus1 * target;
        }
    }
    
    m_SamplesCnt += blocksize;
    m_CurrInd += blocksize;
}

void CSample::setBlocksize( int32_t blocksize, bool_t force )
{
    if ( force || blocksize != m_Resampler.getBlocksize() )
    {
        auto len_in = m_Resampler.getLenIn(blocksize);

        if (len_in.min != len_in.max)
        {
            m_LenInSeq.resize(len_in.num_min + len_in.num_max);
            for( auto i = 0; i < len_in.num_max; i++)
                m_LenInSeq[i] = len_in.max;
            for( auto i = 0; i < len_in.num_min; i++)
                m_LenInSeq[len_in.num_max + i] = len_in.min;
        }
        else
        {
            m_LenInSeq = {len_in.min};
        }
        m_LenInSeqCnt = 0;
        m_BufferResampler.resize(len_in.max);
        m_Resampler.setBlocksize(blocksize, force);
    }
}

void CSample::play(float32_t * const p_out, cint32_t blocksize) 
{
    if( NULL != p_out && NULL != m_PBuffer)
    {
        int32_t total_bs = 0;
        int32_t should_append = m_ShouldAppend;
        int32_t len_in;
        float32_t *p_out_int = p_out;
        if (!m_Resampler.isBypass()){
            setBlocksize(blocksize);
            len_in = m_LenInSeq[m_LenInSeqCnt++ % m_LenInSeq.size()];
            p_out_int = m_BufferResampler.data();
            m_ShouldAppend = false; // this will be done at the end
        } else {
            len_in = blocksize;
        }
        while (total_bs < len_in)
        {
            int32_t bs = len_in - total_bs;
            float32_t *p_out_this = &p_out_int[total_bs];
            switch (m_State)
            {
                case READY:
                {
                    if( m_StatesADSR.att_smp > 0)
                    {
                        m_Gain = 0.0F;
                        m_State = ATTACK;
                        total_bs += processBlockAttack(p_out_this, bs);
                    }
                    else if( m_StatesADSR.dec_smp > 0)
                    {
                        m_Gain = 1.0;
                        m_State = DECAY;
                        total_bs += processBlockDecay(p_out_this, bs);
                    }
                    else if( m_StatesADSR.sus_smp > 0)
                    {
                        m_Gain = m_StatesADSR.sus_lvl;
                        m_State = SUSTAIN;
                        total_bs += processBlockSustain(p_out_this, bs);
                    }
                    else if( m_StatesADSR.rel_smp > 0)
                    {
                        m_Gain = m_StatesADSR.sus_lvl;
                        m_State = RELEASE;
                        total_bs += processBlockRelease(p_out_this, bs);
                    }
                    else
                    {
                        m_Gain = 0.0F;
                        m_State = FINISHED;
                        // force our way out of the loop, as there are no more samples to write
                        total_bs = len_in;
                    }
                }
                break;

                case ATTACK:
                {
                    total_bs += processBlockAttack(p_out_this, bs);
                }
                break;

                case DECAY:
                {
                    total_bs += processBlockDecay(p_out_this, bs);
                }
                break;

                case SUSTAIN:
                {
                    total_bs += processBlockSustain(p_out_this, bs);
                }
                break;

                case RELEASE:
                {
                    total_bs += processBlockRelease(p_out_this, bs);
                }
                break;

                case FADEOUT:
                {
                    total_bs += processBlockFadeout(p_out_this, bs);
                }
                break;

                case FINISHED:
                default:
                {
                    // force our way out of the loop, as there are no more samples to write
                    total_bs = len_in;
                }
                break;
            }
        }
        if (!m_Resampler.isBypass()){
            m_ShouldAppend = should_append; // recover flag
            auto out_resample = m_Resampler.apply(p_out_int, len_in);
            if(m_ShouldAppend){
                for(auto i = 0; i < blocksize; i++)
                    p_out[i] += out_resample[i];
            } else {
                for(auto i = 0; i < blocksize; i++)
                    p_out[i] = out_resample[i];
            }
        }
    }
    else
    {
        m_State = FINISHED;
    }
}

void CSample::on(void) 
{
    // assign new ADSR values
    m_StatesADSR = m_StatesADSRNew;
    // restart index
    m_CurrInd = 0;
    m_SamplesCnt = 0;
    m_State = READY;
}

void CSample::off(void) 
{
    m_SamplesCnt = 0;
    if (m_FadeoutSmp > 0)
        m_State = FADEOUT;
    else
        m_State = FINISHED;
}

void CSample::setFadeout(cfloat32_t fade_s)
{
    if (fade_s >= 0.0F)
    {
        m_FadeoutSmp = secondsToSamples(fade_s);
        m_FadeoutFac = samplesToFactor(m_FadeoutSmp, 1.0);
    }
}

void CSample::setADSR(const CADSR& adsr)
{
    // init with curr values
    tStatesADSR next_st_adsr = m_StatesADSR;
    
    if (adsr.att_s >= 0.0F)
        next_st_adsr.att_smp = secondsToSamples(adsr.att_s);
    if (adsr.dec_s >= 0.0F)
        next_st_adsr.dec_smp = secondsToSamples(adsr.dec_s);
    if (adsr.rel_s >= 0.0F)
        next_st_adsr.rel_smp = secondsToSamples(adsr.rel_s);
    if (adsr.sus_lvl >= 0.0F)
        next_st_adsr.sus_lvl = adsr.sus_lvl;
    

    next_st_adsr.sus_lvl = std::min(std::max(next_st_adsr.sus_lvl, 0.0F), 1.0F);
    next_st_adsr.att_fac = samplesToFactor(next_st_adsr.att_smp, 1.0F);
    next_st_adsr.dec_fac = samplesToFactor(next_st_adsr.dec_smp, 1.0F - next_st_adsr.sus_lvl);
    next_st_adsr.rel_fac = samplesToFactor(next_st_adsr.rel_smp, next_st_adsr.sus_lvl, 0.0F );
    
    /* 
     * Overwrite sus_smp based on what is left from attack, decay and release.
     * Note that if we're in "release only" case there is no sustain.
     * Moreover, sus_smp will be a multiple of m_Blocksize, as att_smp, dec_smp 
     * and rel_smp are all multiple of m_Blocksize.
     */

    if ( ( next_st_adsr.att_smp + next_st_adsr.dec_smp ) == 0 && (next_st_adsr.rel_smp > 0) )
    {
        next_st_adsr.sus_smp = 0;
        next_st_adsr.num_smp = std::min(next_st_adsr.rel_smp, (int32_t)(*m_PBuffer).size());
    }
    else if(NULL != m_PBuffer)
    {
        next_st_adsr.sus_smp =  (*m_PBuffer).size() - next_st_adsr.att_smp - next_st_adsr.dec_smp - next_st_adsr.rel_smp;
        next_st_adsr.sus_smp = std::max(next_st_adsr.sus_smp, 0);
        next_st_adsr.num_smp = (*m_PBuffer).size();
    }

    // finally copy it to class member
    m_StatesADSRNew = next_st_adsr;
}
