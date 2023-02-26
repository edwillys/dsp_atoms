#!/usr/bin/env python
"""
Convert WAV file to C++ header file array of int16

Copyright (c) 2020 Edgar Lubicz

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

"""

import soundfile as sf
import argparse
import sys
import numpy as np
import os

__version__ = "0.1.0"
__author__ = "Edgar Lubicz"

def main(argv=None):
    parser = argparse.ArgumentParser()
    parser.add_argument("-i", "--input"  , help="Input WAV file" , action='store', required=True)
    parser.add_argument("-o", "--output" , help="Output header file" , action='store', required=False)
    pargs = parser.parse_args(argv)

    # open input WAV file
    info = sf.info(pargs.input)
    if info.subtype == 'PCM_U8':
        data, samplerate = sf.read(pargs.input, dtype='uint8')
        vartype = "int"
        NCOLS = 16
        VARFMT = ": <4"
    elif info.subtype == 'PCM_16':
        data, samplerate = sf.read(pargs.input, dtype='int16')
        vartype = "int"
        NCOLS = 16
        VARFMT = ": <6"
    elif info.subtype == 'PCM_24' or info.subtype == 'PCM_32':
        data, samplerate = sf.read(pargs.input, dtype='int32')
        vartype = "int"
        NCOLS = 8
        VARFMT = ": <11"
    else:
        data, samplerate = sf.read(pargs.input)
        vartype = "float"
        VARFMT = ": 1.8f"
        NCOLS = 8

    if pargs.output:
        path_fout = pargs.output
    else:
        path_fout = pargs.input.split(".wav")[0] + ".h"
    
    print("Converting '{}' to '{}'".format(os.path.basename(pargs.input), os.path.basename(path_fout)))
    with open(path_fout, 'w') as fout:
        if len(data.shape) == 1:
            nch = 1
            nsamples = data.shape[0]
        else:
            nch = data.shape[1]
            nsamples = data.shape[0]
        fout.write("int samples_per_ch = {};\n".format(nsamples))
        fout.write("int sample_rate = {};\n".format(samplerate))
        fout.write("\n")
        fout.write("{} samples[{}][{}] = {{\n".format(vartype, nch, nsamples))
        for ch in range(nch):
            if nch > 1:
                ch_data = data[:,ch]
            else:
                ch_data = data

            fout.write("  {\n")
            for i in range(int(np.ceil( len(ch_data) / NCOLS))):
                ind_end = int(np.min( [(i + 1) * NCOLS, len(ch_data) ]))
                line = ch_data[ i * NCOLS : ind_end]
                format_values = "{" + VARFMT + "}, "
                format_str = "    " + format_values * len(line) + "\n"
                fout.write(format_str.format(*line))
            fout.write("  },\n")

        fout.write("};\n")


if __name__ == '__main__':
    main()