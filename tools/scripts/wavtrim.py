#!/usr/bin/env python
"""
Trim zeros from WAV file

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
import glob
from scipy import signal

__version__ = "0.1.0"
__author__ = "Edgar Lubicz"

def main(argv=None):
    parser = argparse.ArgumentParser()
    parser.add_argument("-i", "--input"      , help="Input WAV file(s)" , action='store', required=True)
    parser.add_argument("-o", "--output"     , help="Output folder for trimmed WAV file(s)" , action='store', required=False)
    parser.add_argument("-e", "--epsilon"    , help="Epsilon for trimming" , action='store', default="0.0")
    parser.add_argument("-l", "--left-only"  , help="Trim on left side only" , action='store_true', required=False)
    parser.add_argument("-r", "--right-only" , help="Trim on right side only" , action='store_true', required=False)
    parser.add_argument("-x", "--zero-cross" , help="Trim on zero cross before or after threshold" , action='store_true', required=False)
    parser.add_argument("-f", "--filter-dc"  , help="Filter DC content before everything" , action='store_true', required=False)
    pargs = parser.parse_args(argv)

    curdir = os.getcwd()

    # open input WAV file
    infiles = glob.glob(os.path.join(curdir, pargs.input))

    # output folder
    if pargs.output:
        # add trailing slash if not there
        if pargs.output[-1] != '/':
            pargs.output += '/'
        outfolder = os.path.dirname(os.path.join(curdir, pargs.output))
        if not os.path.exists(outfolder):
            os.mkdir(outfolder)
    else:
        outfolder = "."
    
    if outfolder == os.path.dirname(infiles[0]):
        sys.exit("Please specify an output folder different from the one form the input files")

    for infile in infiles:
        # open input WAV file
        data, samplerate = sf.read(infile)
        fmt = sf.info(infile).subtype
        fs = sf.info(infile).samplerate
        if pargs.filter_dc:
            b, a = signal.ellip(4, 0.1, 100, 20.0 / (fs * 0.5) , 'high', analog=False)
            #b, a = signal.butter(4, 20.0 / (fs * 0.5), 'high', analog=False )
            data = signal.filtfilt(b, a, data)
        # get the indices where the data is larger than threshold
        indices = np.where(np.abs(data) > float(pargs.epsilon))
        try:
            ind_start = np.min(indices)
            ind_end = np.max(indices) + 1
            if pargs.zero_cross:
                # get last zero crossing before ind_start, if any
                zero_x_left = np.where(np.diff(np.sign(data[:ind_start + 1])))[0]
                if len(zero_x_left) > 0:
                    ind_start = zero_x_left[-1] # replace
                # get first zero crossing after ind_end, if any
                zero_x_right = np.where(np.diff(np.sign(data[ind_end - 1:])))[0]
                if len(zero_x_right) > 0:
                    ind_end += zero_x_right[0] # add
        except:
            ind_start = 0
            ind_end = data.shape[0]
        
        if pargs.left_only:
            ind_end = data.shape[0]
        if pargs.right_only:
            ind_start = 0

        if ind_start != 0 or ind_end != data.shape[0]:
            outfile = os.path.join(outfolder, os.path.basename(infile))
            print("Trimming to [{},-{}] from '{}' into '{}'".format(ind_start, data.shape[0]- ind_end, 
                os.path.basename(infile), os.path.basename(outfile)))
            if len(data.shape) > 1:
                data = data[ind_start:ind_end,:]
            else:
                data = data[ind_start:ind_end]
            sf.write(outfile, data, samplerate, fmt)    
        else:
            print("Nothing to trim for '{}'".format(os.path.basename(infile)))


if __name__ == '__main__':
    main()