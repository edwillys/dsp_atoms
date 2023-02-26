#!/usr/bin/env python
"""
Apply ADSR to a WAV file

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

__version__ = "0.1.0"
__author__ = "Edgar Lubicz"

def apply_exp(y0, y1, duration, nsamples, accuracy=0.99, inverse = False):
    x = np.linspace(0.0, duration, nsamples)
    B = np.log(1.0 - accuracy) / duration
    if inverse:
        data = y0 + (y1 - y0) * (1.0 - np.exp( B * x ))
    else:
        data = y1 + (y0 - y1) * np.exp( B * x )
    return data

def main(argv=None):
    parser = argparse.ArgumentParser()
    parser.add_argument("-i", "--input"     , help="Input WAV folder" , action='store', required=True )
    parser.add_argument("-o", "--output"    , help="Input WAV folder" , action='store', required=False )
    parser.add_argument("-a", "--attack"    , help="Attack time (s)"     , action='store', default='0.1' )
    parser.add_argument("-d", "--decay"     , help="Decay time (s)"      , action='store', default='0.1' )
    parser.add_argument("-r", "--release"   , help="Release time (s)"    , action='store', default='0.2' )
    parser.add_argument("-g", "--decaygain" , help="Decay gain (linear)" , action='store', default='0.8' )
    
    pargs = parser.parse_args(argv)

    if pargs.output:
        path_fout = pargs.output
        # create the folder if it doesn't exist
        outfolder = os.path.dirname(os.path.abspath(path_fout))
        if not os.path.exists(outfolder):
            os.mkdir(outfolder)
    else:
        path_fout = pargs.input.split(".wav")[0] + "_asdr.wav"

    
    wavindata, fs = sf.read(pargs.input)
    
    t_attack  = float(pargs.attack )
    t_decay   = float(pargs.decay )
    t_release = float(pargs.release )
    
    # construct ADSR envelope
    n_attack  = int(t_attack * fs)
    n_decay   = int(t_decay * fs)
    n_release = int(t_release * fs)

    if ((n_attack + n_decay) == 0) and n_release > 0:
        # release only case
        n_sustain = 0
        n_remaining = len(wavindata) - n_release
        n_remaining = max(n_remaining, 0)
    else:
        n_sustain = len(wavindata) - (n_attack + n_decay + n_release)
        n_sustain = max(n_sustain, 0)
        n_remaining = 0
    g_decay   = min(max(0.0, float(pargs.decaygain)), 1.0)
     
    # attack
    data_asdr = np.array([])
    if n_attack > 0:
        data = apply_exp(0.0, 1.0, t_attack, n_attack, inverse=True)
        data_asdr = np.append(data_asdr, data)
        y0 = 1.0
    
    y0 = 1.0
    # decay
    if n_decay > 0:
        data = apply_exp(y0, g_decay, t_decay, n_decay)
        data_asdr = np.append(data_asdr, data)
        y0 = g_decay
    # sustain
    if n_sustain > 0:
        data_asdr = np.append(data_asdr, y0 * np.ones(n_sustain))
    # release
    if n_release > 0:
        data = apply_exp(y0, 0.0, t_release, n_release)
        data_asdr = np.append(data_asdr, data)
        y0 = 0.0
    # eventual remaining samples
    if n_remaining > 0:
        data_asdr = np.append(data_asdr, y0 * np.ones(n_remaining))

    # cut if we have passed the length
    data_asdr = data_asdr[:len(wavindata)]

    print("Writing ADSR of '{}' to '{}'".format(os.path.basename(pargs.input), os.path.basename(path_fout)))
    sf.write(path_fout, data_asdr * wavindata, fs, 'PCM_16')

if __name__ == '__main__':
    main()
