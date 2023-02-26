#include "TestUtils.h"
#include <sndfile.h>
#include <iostream>

//=============================================================
// Helper functions
//=============================================================

void write_wav(const std::string& outwav, const std::vector<float32_t>& content, 
    cint32_t fs, cint32_t bps, cint32_t nch )
{
    // write that same content to another file
    SF_INFO info;
    info.channels = nch;
    info.samplerate = fs;
    info.format = SF_FORMAT_WAV | SF_FORMAT_PCM_16;
    SNDFILE* const sndfile = sf_open(outwav.c_str(), SFM_WRITE, &info);
    
    if (sndfile == NULL) {
        std::cout << "Something went wrong in out open" << std::endl;
    }
    else
    {
        sf_count_t count = sf_write_float(sndfile, content.data(), content.size()) ;
        sf_write_sync(sndfile);
        if (count != content.size()) {
            std::cout << "Something went wrong in write" << std::endl;
        }
        sf_close(sndfile);
    }
}

