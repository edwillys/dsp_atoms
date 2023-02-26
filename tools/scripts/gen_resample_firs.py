#!/usr/bin/env python
from scipy import signal
from scipy.signal.fir_filter_design import firwin
import os
import math
import numpy as np

curdir = os.path.dirname(os.path.realpath(__file__))
outfile = os.path.join(curdir, "../src/ResampleFIRCoeffs.h")

resampling_factors = [(2,1), (2,3) , (3,4)]
NCOLS = 8
VARFMT = ": 1.12E"

def gen_fir(up, down, window = ('kaiser', 5.0)):
    max_rate = max(up, down)
    # Determine our up and down factors
    # Use a rational approximation to save computation time on really long
    # signals
    g_ = math.gcd(up, down)
    up //= g_
    down //= g_
    if up == down == 1:
        return np.array([1.]), max_rate
    
    #n_in = x.shape[axis]
    #n_out = n_in * up
    #n_out = n_out // down + bool(n_out % down)

    # Design a linear-phase low-pass FIR filter
    f_c = 1. / max_rate  # cutoff of FIR filter (rel. to Nyquist)
    half_len = 10 * max_rate  # reasonable cutoff for our sinc-like function
    h = firwin(2 * half_len + 1, f_c, window=window)

    # h *= up # done in the code

    # Zero-pad our filter to put the output samples at the center
    #n_pre_pad = (down - half_len % down)
    #n_post_pad = 0
    #n_pre_remove = (half_len + n_pre_pad) // down
    
    ## We should rarely need to do this given our filter lengths...
    #while signal._output_len(len(h) + n_pre_pad + n_post_pad, n_in,
    #                  up, down) < n_out + n_pre_remove:
    #    n_post_pad += 1
    
    #h = np.concatenate((np.zeros(n_pre_pad, dtype=h.dtype), h,
    #                    np.zeros(n_post_pad, dtype=h.dtype)))
    #n_pre_remove_end = n_pre_remove + n_out

    return h, max_rate

with open(outfile, 'w') as fout:
    rates = set()
    for factor in resampling_factors:
        h, rate = gen_fir(factor[0], factor[1])
        h = h.astype(np.float32)
        
        if rate not in rates:
            rates.add(rate)
            fout.write("float FIR_RESAMPLE_FAC{}[{}] = {{\n".format(rate, len(h)))
            for i in range(int(np.ceil( len(h) / NCOLS))):
                ind_end = int(np.min( [(i + 1) * NCOLS, len(h) ]))
                line = h[ i * NCOLS : ind_end]
                format_values = "{" + VARFMT + "}, "
                format_str = "    " + format_values * len(line) + "\n"
                fout.write(format_str.format(*line))
            
            fout.write("};\n\n")

