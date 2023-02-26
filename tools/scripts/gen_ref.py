#!/usr/bin/env python
import adsr
import wav2h
import os
import soundfile as sf
import numpy as np
from scipy import signal
from gen_resample_firs import gen_fir

DEFAULT_FS = 48000

def resample(indata, outfile, us, ds, stype=None):
    subtype = stype
    if isinstance(indata, str):
        data, samplerate = sf.read(indata)
        if subtype is None:
            subtype = sf.info(indata).subtype

    elif isinstance(indata, (np.ndarray, list) ):
        data = indata
        samplerate = DEFAULT_FS
        if subtype is not None:
            subtype = stype
    else:
        return

    fir,_ = gen_fir(us, ds)
    fir = fir * us
    data = signal.upfirdn(fir, data, us, ds)
    #data = signal.resample_poly(data, us, ds)
    sf.write(outfile, data, samplerate, subtype)

def gen_square(nsamples, freq, fs, amp = 1.0, dutycycle=0.5):
    period = fs / freq
    npos = int(dutycycle * period)
    nneg = int(period - npos)
    iter = int(np.ceil(nsamples / period))
    out = [amp] * npos + [-amp] * nneg
    out = out * iter
    return out[:nsamples]    
    

curdir = os.path.dirname(os.path.realpath(__file__))

# generate WAV files
args = [
    ["{}/../bundles/simsam.lv2/sfz/SalamanderGrandPianoV3/48khz16bit_mono/A4v16.wav", "{}/../tests/ref/SalamanderGrandPianoV3/A4v16_unisson.wav", 1, 1, "FLOAT" ],
    ["{}/../bundles/simsam.lv2/sfz/SalamanderGrandPianoV3/48khz16bit_mono/A4v16.wav", "{}/../tests/ref/SalamanderGrandPianoV3/A4v16_up5th.wav"  , 2, 3, "FLOAT" ],
    ["{}/../bundles/simsam.lv2/sfz/SalamanderGrandPianoV3/48khz16bit_mono/A4v16.wav", "{}/../tests/ref/SalamanderGrandPianoV3/A4v16_down5th.wav", 3, 2, "FLOAT" ],
    ["{}/../bundles/simsam.lv2/sfz/SalamanderGrandPianoV3/48khz16bit_mono/A4v16.wav", "{}/../tests/ref/SalamanderGrandPianoV3/A4v16_up4th.wav"  , 3, 4, "FLOAT" ],
    ["{}/../bundles/simsam.lv2/sfz/SalamanderGrandPianoV3/48khz16bit_mono/A4v16.wav", "{}/../tests/ref/SalamanderGrandPianoV3/A4v16_down4th.wav", 4, 3, "FLOAT" ],
    [gen_square(750, 1000., DEFAULT_FS, 0.5)                     , "{}/../tests/ref/square1k_at_48k.wav"                     , 1, 1, "FLOAT" ],
    [gen_square(750, 1000., DEFAULT_FS, 0.5)                     , "{}/../tests/ref/square1k_at_48k_down5th.wav"             , 3, 2, "FLOAT" ],
    [gen_square(750, 1000., DEFAULT_FS, 0.5)                     , "{}/../tests/ref/square1k_at_48k_up5th.wav"               , 2, 3, "FLOAT" ],
]

for arg in args:
    if isinstance(arg[0], str):
        indata = arg[0].format(curdir)
    else:
        indata = arg[0]
    resample(indata, arg[1].format(curdir), arg[2], arg[3], arg[4])

# generate WAV files
args = [
    "-i {}/../tests/in/allones.wav -o {}/../tests/ref/adsr.wav",
    "-i {}/../tests/in/allones.wav -o {}/../tests/ref/adsr1ms.wav -a 0.151 -r 0.149 -d 0.049",
    "-i {}/../tests/in/allones.wav -o {}/../tests/ref/adsr_attack_decay.wav -r 0",
    "-i {}/../tests/in/allones.wav -o {}/../tests/ref/adsr_attack_only.wav -r 0 -d 0",
    "-i {}/../tests/in/allones.wav -o {}/../tests/ref/adsr_attack_release.wav -d 0 -g 1",
    "-i {}/../tests/in/allones.wav -o {}/../tests/ref/adsr_decay_only.wav -a 0 -r 0",
    "-i {}/../tests/in/allones.wav -o {}/../tests/ref/adsr_decay_release.wav -a 0",
    "-i {}/../tests/in/allones.wav -o {}/../tests/ref/adsr_release_only.wav -a 0 -d 0 -g 1",
    "-i {}/../tests/in/allones.wav -o {}/../tests/ref/adsr_attack_toolarge.wav -a 1.2",
    "-i {}/../tests/in/allones.wav -o {}/../tests/ref/adsr_release_toolarge.wav -a 0 -d 0 -g 1 -r 1.2",
    "-i {}/../tests/in/allones.wav -o {}/../tests/ref/adsr_attack_release_toolarge.wav -a 0.6 -d 0 -g 1 -r 0.6"
]

for arg in args:
    adsr.main(filter(None,arg.format(curdir,curdir).split()))

## generate header files
args = [
    "-i {}/../tests/ref/square1k_at_48k.wav",
    "-i {}/../tests/ref/square1k_at_48k_up5th.wav",
    "-i {}/../tests/ref/square1k_at_48k_down5th.wav",
    "-i {}/../tests/ref/SalamanderGrandPianoV3/A4v16_unisson.wav",
    "-i {}/../tests/ref/SalamanderGrandPianoV3/A4v16_up5th.wav",
    "-i {}/../tests/ref/SalamanderGrandPianoV3/A4v16_down5th.wav",
    "-i {}/../tests/ref/SalamanderGrandPianoV3/A4v16_up4th.wav",
    "-i {}/../tests/ref/SalamanderGrandPianoV3/A4v16_down4th.wav",
    "-i {}/../tests/ref/adsr.wav",
    "-i {}/../tests/ref/adsr1ms.wav",
    "-i {}/../tests/ref/adsr_attack_decay.wav",
    "-i {}/../tests/ref/adsr_attack_only.wav",
    "-i {}/../tests/ref/adsr_attack_release.wav",
    "-i {}/../tests/ref/adsr_decay_only.wav",
    "-i {}/../tests/ref/adsr_decay_release.wav",
    "-i {}/../tests/ref/adsr_release_only.wav",
    "-i {}/../tests/ref/adsr_attack_toolarge.wav",
    "-i {}/../tests/ref/adsr_release_toolarge.wav",
    "-i {}/../tests/ref/adsr_attack_release_toolarge.wav",
    "-i {}/../bundles/simsam.lv2/sfz/SalamanderGrandPianoV3/48khz16bit_mono/A0v1.wav   -o {}/../tests/ref/SalamanderGrandPianoV3/A0v1.h",
    "-i {}/../bundles/simsam.lv2/sfz/SalamanderGrandPianoV3/48khz16bit_mono/A0v16.wav  -o {}/../tests/ref/SalamanderGrandPianoV3/A0v16.h",
    "-i {}/../bundles/simsam.lv2/sfz/SalamanderGrandPianoV3/48khz16bit_mono/C1v1.wav   -o {}/../tests/ref/SalamanderGrandPianoV3/C1v1.h",
    "-i {}/../bundles/simsam.lv2/sfz/SalamanderGrandPianoV3/48khz16bit_mono/C1v16.wav  -o {}/../tests/ref/SalamanderGrandPianoV3/C1v16.h",  
    "-i {}/../bundles/simsam.lv2/sfz/SalamanderGrandPianoV3/48khz16bit_mono/D#1v1.wav  -o {}/../tests/ref/SalamanderGrandPianoV3/DSharp1v1.h",
    "-i {}/../bundles/simsam.lv2/sfz/SalamanderGrandPianoV3/48khz16bit_mono/D#1v16.wav -o {}/../tests/ref/SalamanderGrandPianoV3/DSharp1v16.h",
    "-i {}/../bundles/simsam.lv2/sfz/SalamanderGrandPianoV3/48khz16bit_mono/F#1v1.wav  -o {}/../tests/ref/SalamanderGrandPianoV3/FSharp1v1.h",
    "-i {}/../bundles/simsam.lv2/sfz/SalamanderGrandPianoV3/48khz16bit_mono/F#1v16.wav -o {}/../tests/ref/SalamanderGrandPianoV3/FSharp1v16.h",
    "-i {}/../bundles/simsam.lv2/sfz/SalamanderGrandPianoV3/48khz16bit_mono/A1v1.wav   -o {}/../tests/ref/SalamanderGrandPianoV3/A1v1.h",
    "-i {}/../bundles/simsam.lv2/sfz/SalamanderGrandPianoV3/48khz16bit_mono/A1v16.wav  -o {}/../tests/ref/SalamanderGrandPianoV3/A1v16.h",
    "-i {}/../bundles/simsam.lv2/sfz/SalamanderGrandPianoV3/48khz16bit_mono/C2v1.wav   -o {}/../tests/ref/SalamanderGrandPianoV3/C2v1.h",
    "-i {}/../bundles/simsam.lv2/sfz/SalamanderGrandPianoV3/48khz16bit_mono/C2v16.wav  -o {}/../tests/ref/SalamanderGrandPianoV3/C2v16.h",
    "-i {}/../bundles/simsam.lv2/sfz/SalamanderGrandPianoV3/48khz16bit_mono/D#2v1.wav  -o {}/../tests/ref/SalamanderGrandPianoV3/DSharp2v1.h",
    "-i {}/../bundles/simsam.lv2/sfz/SalamanderGrandPianoV3/48khz16bit_mono/D#2v16.wav -o {}/../tests/ref/SalamanderGrandPianoV3/DSharp2v16.h",
    "-i {}/../bundles/simsam.lv2/sfz/SalamanderGrandPianoV3/48khz16bit_mono/F#2v1.wav  -o {}/../tests/ref/SalamanderGrandPianoV3/FSharp2v1.h",
    "-i {}/../bundles/simsam.lv2/sfz/SalamanderGrandPianoV3/48khz16bit_mono/F#2v16.wav -o {}/../tests/ref/SalamanderGrandPianoV3/FSharp2v16.h",
    "-i {}/../bundles/simsam.lv2/sfz/SalamanderGrandPianoV3/48khz16bit_mono/A2v1.wav   -o {}/../tests/ref/SalamanderGrandPianoV3/A2v1.h",
    "-i {}/../bundles/simsam.lv2/sfz/SalamanderGrandPianoV3/48khz16bit_mono/A2v16.wav  -o {}/../tests/ref/SalamanderGrandPianoV3/A2v16.h",
    "-i {}/../bundles/simsam.lv2/sfz/SalamanderGrandPianoV3/48khz16bit_mono/C3v1.wav   -o {}/../tests/ref/SalamanderGrandPianoV3/C3v1.h",
    "-i {}/../bundles/simsam.lv2/sfz/SalamanderGrandPianoV3/48khz16bit_mono/C3v16.wav  -o {}/../tests/ref/SalamanderGrandPianoV3/C3v16.h",
    "-i {}/../bundles/simsam.lv2/sfz/SalamanderGrandPianoV3/48khz16bit_mono/D#3v1.wav  -o {}/../tests/ref/SalamanderGrandPianoV3/DSharp3v1.h",
    "-i {}/../bundles/simsam.lv2/sfz/SalamanderGrandPianoV3/48khz16bit_mono/D#3v16.wav -o {}/../tests/ref/SalamanderGrandPianoV3/DSharp3v16.h",
    "-i {}/../bundles/simsam.lv2/sfz/SalamanderGrandPianoV3/48khz16bit_mono/F#3v1.wav  -o {}/../tests/ref/SalamanderGrandPianoV3/FSharp3v1.h",
    "-i {}/../bundles/simsam.lv2/sfz/SalamanderGrandPianoV3/48khz16bit_mono/F#3v16.wav -o {}/../tests/ref/SalamanderGrandPianoV3/FSharp3v16.h",
    "-i {}/../bundles/simsam.lv2/sfz/SalamanderGrandPianoV3/48khz16bit_mono/A3v1.wav   -o {}/../tests/ref/SalamanderGrandPianoV3/A3v1.h",
    "-i {}/../bundles/simsam.lv2/sfz/SalamanderGrandPianoV3/48khz16bit_mono/A3v16.wav  -o {}/../tests/ref/SalamanderGrandPianoV3/A3v16.h",
    "-i {}/../bundles/simsam.lv2/sfz/SalamanderGrandPianoV3/48khz16bit_mono/C4v1.wav   -o {}/../tests/ref/SalamanderGrandPianoV3/C4v1.h",
    "-i {}/../bundles/simsam.lv2/sfz/SalamanderGrandPianoV3/48khz16bit_mono/C4v16.wav  -o {}/../tests/ref/SalamanderGrandPianoV3/C4v16.h",
    "-i {}/../bundles/simsam.lv2/sfz/SalamanderGrandPianoV3/48khz16bit_mono/D#4v1.wav  -o {}/../tests/ref/SalamanderGrandPianoV3/DSharp4v1.h",
    "-i {}/../bundles/simsam.lv2/sfz/SalamanderGrandPianoV3/48khz16bit_mono/D#4v16.wav -o {}/../tests/ref/SalamanderGrandPianoV3/DSharp4v16.h",
    "-i {}/../bundles/simsam.lv2/sfz/SalamanderGrandPianoV3/48khz16bit_mono/F#4v1.wav  -o {}/../tests/ref/SalamanderGrandPianoV3/FSharp4v1.h",
    "-i {}/../bundles/simsam.lv2/sfz/SalamanderGrandPianoV3/48khz16bit_mono/F#4v16.wav -o {}/../tests/ref/SalamanderGrandPianoV3/FSharp4v16.h",
    "-i {}/../bundles/simsam.lv2/sfz/SalamanderGrandPianoV3/48khz16bit_mono/A4v1.wav   -o {}/../tests/ref/SalamanderGrandPianoV3/A4v1.h",
    "-i {}/../bundles/simsam.lv2/sfz/SalamanderGrandPianoV3/48khz16bit_mono/A4v16.wav  -o {}/../tests/ref/SalamanderGrandPianoV3/A4v16.h",
    "-i {}/../bundles/simsam.lv2/sfz/SalamanderGrandPianoV3/48khz16bit_mono/C5v1.wav   -o {}/../tests/ref/SalamanderGrandPianoV3/C5v1.h",
    "-i {}/../bundles/simsam.lv2/sfz/SalamanderGrandPianoV3/48khz16bit_mono/C5v16.wav  -o {}/../tests/ref/SalamanderGrandPianoV3/C5v16.h",
    "-i {}/../bundles/simsam.lv2/sfz/SalamanderGrandPianoV3/48khz16bit_mono/D#5v1.wav  -o {}/../tests/ref/SalamanderGrandPianoV3/DSharp5v1.h",
    "-i {}/../bundles/simsam.lv2/sfz/SalamanderGrandPianoV3/48khz16bit_mono/D#5v16.wav -o {}/../tests/ref/SalamanderGrandPianoV3/DSharp5v16.h",
    "-i {}/../bundles/simsam.lv2/sfz/SalamanderGrandPianoV3/48khz16bit_mono/F#5v1.wav  -o {}/../tests/ref/SalamanderGrandPianoV3/FSharp5v1.h",
    "-i {}/../bundles/simsam.lv2/sfz/SalamanderGrandPianoV3/48khz16bit_mono/F#5v16.wav -o {}/../tests/ref/SalamanderGrandPianoV3/FSharp5v16.h",
    "-i {}/../bundles/simsam.lv2/sfz/SalamanderGrandPianoV3/48khz16bit_mono/A5v1.wav   -o {}/../tests/ref/SalamanderGrandPianoV3/A5v1.h",
    "-i {}/../bundles/simsam.lv2/sfz/SalamanderGrandPianoV3/48khz16bit_mono/A5v16.wav  -o {}/../tests/ref/SalamanderGrandPianoV3/A5v16.h",
    "-i {}/../bundles/simsam.lv2/sfz/SalamanderGrandPianoV3/48khz16bit_mono/C6v1.wav   -o {}/../tests/ref/SalamanderGrandPianoV3/C6v1.h",
    "-i {}/../bundles/simsam.lv2/sfz/SalamanderGrandPianoV3/48khz16bit_mono/C6v16.wav  -o {}/../tests/ref/SalamanderGrandPianoV3/C6v16.h",
    "-i {}/../bundles/simsam.lv2/sfz/SalamanderGrandPianoV3/48khz16bit_mono/D#6v1.wav  -o {}/../tests/ref/SalamanderGrandPianoV3/DSharp6v1.h",
    "-i {}/../bundles/simsam.lv2/sfz/SalamanderGrandPianoV3/48khz16bit_mono/D#6v16.wav -o {}/../tests/ref/SalamanderGrandPianoV3/DSharp6v16.h",
    "-i {}/../bundles/simsam.lv2/sfz/SalamanderGrandPianoV3/48khz16bit_mono/F#6v1.wav  -o {}/../tests/ref/SalamanderGrandPianoV3/FSharp6v1.h",
    "-i {}/../bundles/simsam.lv2/sfz/SalamanderGrandPianoV3/48khz16bit_mono/F#6v16.wav -o {}/../tests/ref/SalamanderGrandPianoV3/FSharp6v16.h",
    "-i {}/../bundles/simsam.lv2/sfz/SalamanderGrandPianoV3/48khz16bit_mono/A6v1.wav   -o {}/../tests/ref/SalamanderGrandPianoV3/A6v1.h",
    "-i {}/../bundles/simsam.lv2/sfz/SalamanderGrandPianoV3/48khz16bit_mono/A6v16.wav  -o {}/../tests/ref/SalamanderGrandPianoV3/A6v16.h",
    "-i {}/../bundles/simsam.lv2/sfz/SalamanderGrandPianoV3/48khz16bit_mono/C7v1.wav   -o {}/../tests/ref/SalamanderGrandPianoV3/C7v1.h",
    "-i {}/../bundles/simsam.lv2/sfz/SalamanderGrandPianoV3/48khz16bit_mono/C7v16.wav  -o {}/../tests/ref/SalamanderGrandPianoV3/C7v16.h",
    "-i {}/../bundles/simsam.lv2/sfz/SalamanderGrandPianoV3/48khz16bit_mono/D#7v1.wav  -o {}/../tests/ref/SalamanderGrandPianoV3/DSharp7v1.h",
    "-i {}/../bundles/simsam.lv2/sfz/SalamanderGrandPianoV3/48khz16bit_mono/D#7v16.wav -o {}/../tests/ref/SalamanderGrandPianoV3/DSharp7v16.h",
    "-i {}/../bundles/simsam.lv2/sfz/SalamanderGrandPianoV3/48khz16bit_mono/F#7v1.wav  -o {}/../tests/ref/SalamanderGrandPianoV3/FSharp7v1.h",
    "-i {}/../bundles/simsam.lv2/sfz/SalamanderGrandPianoV3/48khz16bit_mono/F#7v16.wav -o {}/../tests/ref/SalamanderGrandPianoV3/FSharp7v16.h",
    "-i {}/../bundles/simsam.lv2/sfz/SalamanderGrandPianoV3/48khz16bit_mono/A7v1.wav   -o {}/../tests/ref/SalamanderGrandPianoV3/A7v1.h",
    "-i {}/../bundles/simsam.lv2/sfz/SalamanderGrandPianoV3/48khz16bit_mono/A7v16.wav  -o {}/../tests/ref/SalamanderGrandPianoV3/A7v16.h",
    "-i {}/../bundles/simsam.lv2/sfz/SalamanderGrandPianoV3/48khz16bit_mono/C8v1.wav   -o {}/../tests/ref/SalamanderGrandPianoV3/C8v1.h",
    "-i {}/../bundles/simsam.lv2/sfz/SalamanderGrandPianoV3/48khz16bit_mono/C8v16.wav  -o {}/../tests/ref/SalamanderGrandPianoV3/C8v16.h"
]

for arg in args:
    wav2h.main(filter(None,arg.format(curdir, curdir).split()))
