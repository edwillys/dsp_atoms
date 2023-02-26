#!/usr/bin/env python
"""
Remove duplicate tracks in a WAV file

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
import itertools
import numpy as np
import os

__version__ = "0.1.0"
__author__ = "Edgar Lubicz"

def main(argv=None):
    parser = argparse.ArgumentParser()
    parser.add_argument("-i", "--input"  , help="Input WAV file(s)" , action='store', required=True)
    parser.add_argument("-o", "--output" , help="Input header folder" , action='store', required=False)
    parser.add_argument("-f", "--format" , help="WAV format to save output files" , action='store', required=False)
    pargs = parser.parse_args()

    # open input WAV file
    infiles = glob.glob(pargs.input)
    
    outfolder = None
    if pargs.output:
        outfolder = os.path.dirname(pargs.output)
        if not os.path.exists(outfolder):
            os.mkdir(outfolder)

    for infile in infiles:
        data, samplerate = sf.read(infile)
        if len(data.shape) == 1:
            print("'{}' is mono".format(infile))
        else:
            close_inds = []
            diff_inds = set()
            # get all the possible combinations of channel index pairs (i, j), sorted ascending
            # for example: [ (0,1), (0,2), (0,3), (1,2), (1,3), (2,3) ]
            combs = itertools.combinations(list(range(data.shape[1])),2)
            for comb in combs:
                if np.isclose(data[:,comb[0]], data[:,comb[1]]).all():
                    close_inds += [comb]


            if len(close_inds) > 0:
                print("'{}' has channels with same content: {}".format(os.path.basename(infile), close_inds))
            else:
                print("'{}' has every channel with unique content".format(os.path.basename(infile)))

            if outfolder:
                outpath = os.path.join(outfolder, os.path.basename(infile))
                if os.path.realpath(outpath) != os.path.realpath(infile):
                    # add unique channels that are not duplicated
                    unique_chs = {}
                    flatten_close_inds = set([ ind for ind in close_ind for close_ind in close_inds])
                    for i in range(data.shape[1]):
                        if i in flatten_close_inds:
                            unique_chs[i] = {}
                    # now add unique channels that are duplicated: only add the lowest index (key)
                    # create a dictionary of a set, in which we insert the lowest index of the duplicated 
                    # channels # as key and the remaining indices as values. This way, if we have 
                    # close_inds = [(0,1), (1,2), (2,3)], it will yield
                    # unique_chs = { 0 : {1, 2, 3} }
                    for cind in close_inds:
                        if cind[0] in unique_chs.keys():
                            unique_chs[cind[0]].add(cind[1])
                        else:
                            unique_chs[cind[0]] = {cind[1]}
                    fmt = sf.info(infile).subtype
                    if pargs.format: 
                        if pargs.format.to_upper() in sf.available_subtypes('WAV'):
                            fmt = pargs.format.to_upper()
                            
                    t_unique_chs = tuple(unique_chs.keys())
                    print("Creating file with unique channels {}".format(t_unique_chs))
                    with sf.SoundFile(outpath, 'w', samplerate, len(t_unique_chs), fmt) as f:
                        f.write(data[:,t_unique_chs])
                else:
                    print("Cant overwrite the file we're reading")
                    

if __name__ == '__main__':
    main()
