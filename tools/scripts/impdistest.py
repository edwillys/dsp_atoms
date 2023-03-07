#!/usr/bin/env python
import numpy as np
from scipy import signal
import matplotlib.pyplot as plt
from dbfft import dbfft as dbfft

class impdistest():
    def __init__(self, fs: float, f1: float, f2: float, g: float, T: float, tail_sec: float) -> None:
        # Generate the ESS function x(t), accordingto Farina's paper:
        # f(t) = K * (e ^ (t/L) - 1)
        # x(t) = sin(f(t))
        # where
        #  K = L * w1
        #  L = T / ln(w2/w1)
        #  T being the duration of the signal is seconds

        # First let us extend T, so that the sine weep ends at exactly 0,
        # hence f(T) = 2*pi*k, where k is an integer
        # At f(T), we have:
        # f(T) = 2*pi*T*(f2-f1)/ln(f2/f1)
        k = np.ceil(T * (f2 - f1) / np.log(f2 / f1))
        T = k / (f2 - f1) * np.log(f2 / f1)

        w1 = 2.0 * np.pi * f1
        w2 = 2.0 * np.pi * f2
        L = T / np.log(w2 / w1)
        K = L * w1
        t = np.arange(0, fs * T)/fs
        e = np.exp(t / L)
        self.ess = np.sin(K * (e - 1.0))
        e2 = np.power(f2/f1, t / T)
        self.ess_inv = 10.0 ** (g / 20.0) * np.flipud(self.ess) / e2
        # append zeros
        n_zeros = int(np.ceil(fs * tail_sec))
        if n_zeros > 0:
            self.ess_meas = np.concatenate((self.ess, np.zeros(n_zeros)))
        else:
            self.ess_meas = self.ess
        self.ess_meas *= 10.0 ** (g / 20.0)
        # scale factor (adapted from Holters2009)
        # num_samples = np.ceil(fs * T)
        # self.scale_fact = num_samples * np.pi * (w1 / w2 - 1.0) / \
        #     (2.0 * (w2 - w1) * np.log(w1 / w2))
        # Above doesn't work. We take the average of the amplitudes
        # of the transfer function at the frequencies of interest
        # and heuristically compute the scale factor
        dirac = signal.fftconvolve(self.ess, self.ess_inv)
        dirac = dirac[len(self.ess) - 1:]
        sp = np.fft.rfft(dirac)
        N = len(dirac)
        freq = np.arange((N // 2) + 1) / (float(N) / fs)
        inds = np.where((freq >= f1) & (freq <= f2 ))
        self.scale_fact = 2 * np.mean(np.abs(sp[inds])) / N

    def signal(self):
        return self.ess_meas

    def estimate(self, y: np.ndarray):
        ir = signal.fftconvolve(self.ess_inv, y)
        return ir[len(self.ess) - 1:]

def test():
    fs = 48000.0
    fc = 500.0
    f1 = 1
    f2 = fs/2

    idm = impdistest(fs, f1, f2, 0.0, 4, 1.0)

    sos = signal.butter(4, fc, 'hp', fs=fs, output='sos')
    filtered = signal.sosfilt(sos, idm.signal())
    ir = idm.estimate(filtered)
    ir /= idm.scale_fact

    b, a = signal.butter(4, fc, 'hp', fs=fs)
    w, h = signal.freqz(b, a)
    f, Xdb = dbfft(h, fs)
    
    plt.figure()
    plt.title('Butterworth filter frequency response')
    plt.subplot(211)
    plt.semilogx(w / (2 * np.pi) * fs, 20 * np.log10(abs(h)))
    plt.xlabel('Frequency [Hz]')
    plt.ylabel('Amplitude [dB]')
    plt.xlim((f1, f2))
    plt.ylim((-100, 5))
    plt.margins(0, 0.1)
    plt.grid(which='both', axis='both')

    f, Xdb = dbfft(ir, fs)
    plt.subplot(212)
    plt.semilogx(f, Xdb)
    plt.title('Spectrum')
    plt.xlabel('Frequency [Hz]')
    plt.ylabel('Amplitude [dBFS]')
    plt.xlim((f1, f2))
    plt.ylim((-100, 5))
    plt.margins(0, 0.1)
    plt.grid(which='both', axis='both')

    plt.show()


if __name__ == '__main__':
    test()
