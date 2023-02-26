#!/usr/bin/env python
"""
Generate WAV files with some signal

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

types = ["STEP", "DIRAC"]

def main(argv=None):
    parser = argparse.ArgumentParser()
    parser.add_argument("-o", "--output" , help="Input header folder" , action='store', required=True)
    parser.add_argument("-t", "--type" , help="Signal type: " + str(types), action='store', required=True)
    parser.add_argument("-d", "--duration" , help="Duration is seconds" , action='store', default='1.0')
    parser.add_argument("-f", "--fs" , help="Sampling frequency" , action='store', default='48000')
    
    pargs = parser.parse_args()

    typ = pargs.type.upper()
    if typ not in types:
        sys.exit("Allowed types are: " + str(types))

    outfolder = os.path.dirname(os.path.abspath(pargs.output))
    if not os.path.exists(outfolder):
        os.mkdir(outfolder)

    fs = int(pargs.fs)
    duration = float(pargs.duration)

    if typ == "STEP":
        data = np.ones(int(fs * duration))
    elif typ == "DIRAC":
        data = np.zeros(int(fs * duration))
        data[0] = 1.0
    
    print("Writing file to {}".format(pargs.output))
    sf.write(pargs.output, data, fs, 'PCM_16')

if __name__ == '__main__':
    main()
