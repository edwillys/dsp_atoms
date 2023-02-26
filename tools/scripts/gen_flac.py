#!/usr/bin/env python
import glob
import os
import subprocess

curdir = os.path.dirname(os.path.realpath(__file__))
outdir = os.path.join(curdir, "../bundles/simsam.lv2/sfz/SalamanderGrandPianoV3/48khz16bitMonoFLAC")
# open input WAV file
infiles = glob.glob(os.path.join(curdir, "../bundles/simsam.lv2/sfz/SalamanderGrandPianoV3/48khz16bit_mono/*.wav"))

# generate FLAC files
for inpath in infiles:
    infile = os.path.basename(inpath)
    outfile = infile.split(".wav")[0] + ".flac"
    outpath = os.path.join(outdir, outfile)
    print("Processing " + infile + " into " + outfile)
    ret = subprocess.check_call(["sox", inpath, outpath])
