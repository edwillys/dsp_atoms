#!/usr/bin/env python
import numpy as np

def dbfft(x, fs, win=None):
    """ Helper function for calculating the FFT amplitude of a signal
    Adapted from:
    https://dsp.stackexchange.com/questions/41696/calculating-the-inverse-filter-for-the-exponential-sine-sweep-method

    Args:
        x : input signal
        fs : sampling frequency
        win: FFT window. Defaults to None.

    Raises:
        ValueError: if the window doesn't match to the input signal

    Returns:
        Tuple with the frequency axis and the amplitude in dBFS
    """
    N = len(x)  # Length of input sequence

    if win is None:
        win = np.ones(x.shape)
    if len(x) != len(win):
        raise ValueError('Signal and window must be of the same length')
    x = x * win

    # Calculate real FFT and frequency vector
    sp = np.fft.rfft(x)
    freq = np.arange((N // 2) + 1) / (float(N) / fs)

    # Scale the magnitude of FFT by window and factor of 2,
    # because we are using half of FFT spectrum.
    s_mag = np.abs(sp) * 2 / np.sum(win)

    # Convert to dBFS
    s_dbfs = 20.0 * np.log10(s_mag)

    return freq, s_dbfs
