import numpy as np
import scipy.signal as signal
import scipy.io.wavfile as wavfile
from scipy.special import lambertw as lambertw
from scipy.special import wrightomega as wrightomega
import matplotlib.pyplot as plt
from os import path as osp
from PySpice.Probe.Plot import plot
from PySpice.Spice.Library import SpiceLibrary
from PySpice.Spice.Netlist import Circuit
from PySpice.Unit import *


class diode:
    def __init__(self, IS, N, RS, CJO, VJ, M, TT, type=np.float64) -> None:
        # Spice parameters
        self.type = type
        self.IS = type(IS)  # saturation current
        self.N = type(N)  # emission coeff
        self.RS = type(RS)  # internal parasitic resistance
        self.CJO = type(CJO)  # Depletion capacitance
        self.VJ = type(VJ)  # Junction voltage
        self.M = type(M)  # Junction gradient coeff
        self.TT = type(TT)  # transit time

    def clone(self, dtype):
        o = diode(
            self.IS,
            self.N,
            self.RS,
            self.CJO,
            self.VJ,
            self.M,
            self.TT,
            dtype
        )
        return o

    def calc(self, Vin, R=1000, temp_c=20.0):
        # Physical constants
        k = self.type(1.38e-23)  # Boltmann
        q = self.type(1.6e-19)  # electron charge
        Vt = k * (273 + self.type(temp_c)) / q

        # Intermediate variables
        a0 = self.IS * (self.type(R) + self.RS)
        a1 = self.type(1) / (self.N * Vt)

        # 1. Series resistance + 1 diode model
        y_1d = Vin + a0 - lambertw(a0 * a1 / np.exp(-a1 * (a0 + Vin))) / a1

        # 2. Series resistance + 2 diode model
        I2 = self.IS * (np.exp(-a1*np.maximum(Vin, 0)) - self.type(1))
        Vin_sign = np.sign(Vin)
        Vin_abs = np.abs(Vin)
        y_2d = Vin_abs + a0 + I2 - \
            lambertw(a0 * a1 * np.exp(a1 * (Vin_abs + a0 + I2))) / a1
        y_2d *= Vin_sign

        # 3. Simplified version of 2., without I2
        y_2d_simp = Vin_abs + a0 - \
            lambertw(a0 * a1 * np.exp(a1 * (Vin_abs + a0))) / a1
        y_2d_simp *= Vin_sign

        # Wright omega
        w = wrightomega(a1 * (Vin_abs + a0) + np.log(a0 * a1))
        y_2d_omega = Vin_abs + a0 - w / a1
        y_2d_omega *= Vin_sign

        # D'angelo approx
        alpha = self.type(-1.3142931498778e-3)
        beta = self.type(4.775931364975583e-2)
        gamma = self.type(3.631952663804445e-1)
        tau = self.type(6.313183464296682e-1)
        x1 = self.type(-3.341459552768620)
        x2 = self.type(8)
        def w4(x, w3): return w3 - (w3 - np.exp(x - w3)) / (w3 + 1)
        # y_2d_simp = Vin_abs + a0 - \
        #            lambertw(a0 * a1 * np.exp(a1 * (Vin_abs + a0))) / a1
        x = a1 * (Vin_abs + a0) + np.log(a0 * a1)
        w = np.zeros(x.shape)
        # middle part
        inds = (x > x1) & (x < x2)
        x_middle = x[inds]
        w3_middle = alpha * (x_middle**3) + beta * \
            (x_middle ** 2) + gamma * x_middle + tau
        w[inds] = w4(x_middle, w3_middle)
        # last part
        inds = x >= x2
        x_last = x[inds]
        w3_last = x[inds] - np.log(x[inds])
        w[inds] = w4(x_last, w3_last)

        y_2d_dangelo = Vin_abs + a0 - w / a1
        y_2d_dangelo *= Vin_sign

        # Observable values for analysis
        I = (Vin - y_2d) / (R + self.RS)
        CD = self.TT / Vt * I + self.CJO / ((1.0 - y_2d / self.VJ) ** self.M)

        dic_y = {
            "y_1d": y_1d,
            "y_2d": y_2d,
            "y_2d_simp": y_2d_simp,
            "y_2d_omega": y_2d_omega,
            "y_2d_dangelo": y_2d_dangelo
        }

        dic_extra = {
            "I": I,
            "tau": (self.RS+R)*CD,
        }

        return dic_y, dic_extra


normalize = False
write_wav = False
R = [0, 1, 10, 1000, 10000, 100000, 1000000]
# R = [1000]
FS = 48000  # Hz
temp_c = 20
A = 1 # amplitude, linear
T = 1  # seconds

# Ground truth: spice model
circuit = Circuit('Diode Distortion')
# Triangular wave
source = circuit.PieceWiseLinearVoltageSource(
    'in', 'input', circuit.gnd, values=[(0, 0), (0.25*T, A), (0.75*T, -A), (T, 0)])
circuit_r = circuit.R('R', 'input', 'output', 100@u_Ω)
fpath = osp.dirname(osp.abspath(__file__))
spice_library = SpiceLibrary(osp.join(fpath, "spicelib"))
circuit.include(spice_library['1N4148'])
circuit.Diode('D1', 'output', circuit.gnd, model='1N4148')
circuit.Diode('D2', circuit.gnd, 'output', model='1N4148')

# 1N4148
diode_1N4148_float64 = diode(
    IS=4.352E-9,
    N=1.906,
    RS=0.6458,
    CJO=7.048E-13,
    VJ=0.869,
    M=0.03,
    TT=3.48E-9,
    type=np.float64
)
diode_1N4148_float32 = diode_1N4148_float64.clone(np.float32)

plt.figure()
plt.title('Vout/Vin')

max_diff_model = 0
max_diff_dtype = 0
max_diff_omega = 0
min_w0 = np.inf

for r in R:
    # Spice model
    circuit_r.resistance = r
    simulator = circuit.simulator(
        temperature=temp_c, nominal_temperature=temp_c)
    # analysis = simulator.dc(Vin=Vsl)
    analysis = simulator.transient(step_time=1/FS, end_time=T)
    y_spice = np.array(analysis["output"])
    Vin = np.array(analysis["input"])
    t = np.array(analysis.time)

    y_f64, extra_f64 = diode_1N4148_float64.calc(Vin, r, temp_c)
    y_f32, extra_f32 = diode_1N4148_float32.calc(Vin, r, temp_c)

    for key in y_f64:
        y_f64[key] = np.real(y_f64[key])
        y_f32[key] = np.real(y_f32[key])

    if normalize:
        for key in y_f64:
            y_f64[key] = y_f64[key] / max(abs(y_f64[key]))
            y_f32[key] = y_f32[key] / max(abs(y_f32[key]))

    diff_model = max(abs(y_f64["y_2d_simp"]-y_spice))
    max_diff_model = max(diff_model, max_diff_model)

    diff_dtype = max(abs(y_f64["y_2d_simp"]-y_f32["y_2d_simp"]))
    max_diff_dtype = max(diff_dtype, max_diff_dtype)

    diff_omega = max(abs(y_f64["y_2d_simp"]-y_spice))
    max_diff_omega = max(diff_omega, max_diff_omega)

    w0 = np.abs(1.0 / (2.0 * np.pi * extra_f64["tau"]))
    min_w0 = min(min(w0), min_w0)

    if write_wav:
        amplitude = np.iinfo(np.int16).max
        data = amplitude * y_f32["y_2d_omega"]
        wavfile.write(
            f"AtomDiode_Triangle_1Hz_1s_0dB_Morph0ms_1ch_R{r}.wav", 48000, data.astype(np.int16))

    # plt.plot(t, y_f64["y_2d_simp"], label=f"Lambert@R={str(r)}")
    plt.plot(t, y_f64["y_2d_omega"], label=f"Omega@R={str(r)}")
    plt.plot(t, y_spice, label=f"Spice@R={str(r)}")
    # plt.plot(
    #    t, 20 * np.log10(abs(y_f64["y_2d_simp"] - y_spice)), label=f"Diff@R={str(r)}")

# Plot Vin after because it is the same for all Rs in the loop above
plt.plot(t, Vin, label="Vin")

if write_wav:
    amplitude = np.iinfo(np.int16).max
    data = amplitude * Vin
    wavfile.write(
        "Triangle_1Hz_1s_0dB.wav", 48000, data.astype(np.int16))

# TANH model
# k = 2.3
# y_tanh = np.tanh(k*Vin) / np.tanh(k)
# plt.plot(t, y_tanh, label="TANH")

# SIG
# k = 5
# y_sig = 2 / (1 + np.exp(-k * Vin)) - 1
# plt.plot(t, y_sig, label="SIG")

# ATAN
# k = 3.5
# y_atan = np.arctan(k*Vin) / np.arctan(k)
# plt.plot(t, y_atan, label="ATAN")

# FEXP1 (doesn't work)
# k = 2
# y_fexp1 = -np.sign(Vin) * (1 - np.exp(-abs(k * Vin))) / (1 - np.exp(k))
# q = Vin / np.abs(Vin)
# y_fexp1 = -Vin / abs(Vin) * (1 - np.exp(k * Vin * Vin / abs(Vin)))
# plt.plot(t, y_fexp1, label="FEXP1")

plt.xlabel('Vin [V]')
plt.ylabel('Vout [V]')
plt.margins(0, 0.1)
plt.grid(which='both', axis='both')
plt.legend()

print(
    f"Max diff model: {20.0 * np.log10(max_diff_model):.1f}dB")
print(
    f"Max diff dtype: {20.0 * np.log10(max_diff_dtype):.1f}dB")
print(
    f"Max diff omega: {20.0 * np.log10(max_diff_omega):.1f}dB")
print(
    f"Min cutoff freq: {min_w0:.1f}Hz")

if False:
    # Observable values for analysis
    I = (Vin - y_1d) / (R + RS)
    CD = TT / Vt * I + CJO / ((1.0 - y_1d / VJ) ** M)

    plt.figure()
    plt.title('W0/Vin')
    w0 = np.abs(1.0 / (2.0 * np.pi * CD * (R + RS)))
    print(f"Minimum cuttof frequency is {min(w0) / 1000.0:.1f}kHz")
    plt.plot(Vin, w0)
    plt.xlabel('Vin [V]')
    plt.ylabel('W0 [Hz]')
    plt.margins(0, 0.1)
    plt.grid(which='both', axis='both')

    plt.figure()
    plt.title('Id/Vin')
    plt.plot(Vin, I * 10e3)
    plt.xlabel('Vin [V]')
    plt.ylabel('Id [mA]')
    plt.margins(0, 0.1)
    plt.grid(which='both', axis='both')

plt.show()
