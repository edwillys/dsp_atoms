#!/usr/bin/env python
"""
Convert a WAV file to mono

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
import glob
import numpy as np
import os
from scipy import signal

__version__ = "0.1.0"
__author__ = "Edgar Lubicz"

def main(argv=None):
    parser = argparse.ArgumentParser()
    parser.add_argument("-i", "--input"  , help="Input WAV file(s)" , action='store', required=True)
    parser.add_argument("-o", "--output" , help="Output folder" , action='store', required=True)
    parser.add_argument("-f", "--format" , help="WAV format of output files: " + str(sf.available_subtypes('WAV').keys()), action='store', required=False)
    parser.add_argument("-s", "--fs"     , help="Sample rate of output files: 44100 or 48000" , action='store', required=False)
    pargs = parser.parse_args()

    curdir = os.getcwd()

    # open input WAV file
    infiles = glob.glob(os.path.join(curdir, pargs.input))
    
    outfolder = None
    # add trailing slash if not there
    if pargs.output[-1] != '/':
        pargs.output += '/'
    outfolder = os.path.dirname(os.path.join(curdir, pargs.output))
    if not os.path.exists(outfolder):
        os.mkdir(outfolder)

    out_fs = None
    if pargs.fs: 
        try:
            fs_int = int(pargs.fs)
            if fs_int == 44100 or fs_int == 48000:
                out_fs = fs_int
                print("Forcing FS={}".format(fs_int))
            else:
                print("FS {} not supported. Using the same as input files".format(fs_int))
        except:
            print("FS is not an integer. Using the same as input files".format(fs_int))
    
    out_fmt = None
    if pargs.format: 
        if pargs.format.upper() in sf.available_subtypes('WAV'):
            out_fmt = pargs.format.upper()
            print("Forcing Format={}".format(out_fmt))
        else:
            print("Format {} not supported. Using the same as input files".format(pargs.format))

    print("Writing mono files to {}".format(outfolder))
    for infile in infiles:
        data, samplerate = sf.read(infile)
        if len(data.shape) == 1:
            print("'{}' is already mono. Nothing to do".format(infile))
        else:
            outpath = os.path.join(outfolder, os.path.basename(infile))
            if os.path.realpath(outpath) != os.path.realpath(infile):
                if out_fmt:
                    fmt = out_fmt
                else:
                    fmt = sf.info(infile).subtype
                
                data = np.mean(data,1)
                
                # resample if needed
                if out_fs and out_fs != samplerate:
                    print("Resampling from {} to {}".format(samplerate, out_fs))
                    data = signal.resample_poly(data, out_fs, samplerate)
                    samplerate = out_fs
                
                sf.write(outpath, data, samplerate, fmt)
                print("'{}' done".format(os.path.basename(infile)))
            else:
                print("Cant overwrite the file we're reading")
                    

if __name__ == '__main__':
    main()
