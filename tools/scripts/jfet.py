import numpy as np
from os import path as osp
import scipy.io.wavfile as wavfile
import matplotlib.pyplot as plt
from PySpice.Spice.Library import SpiceLibrary
from PySpice.Spice.Netlist import Circuit
from PySpice.Spice.Parser import SpiceParser
from prefixed import Float
from sympy import symbols, solve, simplify, poly_from_expr, diff


class jfet:
    def __init__(self, BETA, LAMBDA, VTO, RD, RS, VDD, type=np.float64) -> None:
        self.ch_len_mod = True
        # Spice parameters
        self.type = type
        BETA = type(BETA)  # Transconductance coefficient (A/V^2)
        LAMBDA = type(LAMBDA)  # Channel-length modulation (1/V)
        VTO = type(VTO)  # Thresold voltage (V)
        RD = type(RD)  # Drain ohmic resistance (Ohm)
        RS = type(RS)  # Source ohmic resistance (Ohm)
        VDD = type(VDD)  # Power supply (V)
        RDS = RD + RS

        # 3rd degree polynomial coeffs for the Id in the triode (linear) region
        self.a_triode = [
            # a0
            BETA*LAMBDA*RDS**2*(-RDS + 2*RS),
            # a1
            BETA*RDS*(3*LAMBDA*RDS*VDD - 4*LAMBDA*RS*VDD + RDS - 2*RS),
            # a2
            -3*BETA*LAMBDA*RDS*VDD**2 + 2*BETA*LAMBDA*RS*VDD**2 - 2*BETA*RDS*VDD + 2*BETA*RS*VDD + 1,
            # a3
            BETA*VDD**2*(LAMBDA*VDD + 1)
        ]
        # factor to be multiplied with Vgt and added to each a_triode coefficient
        self.a_triode_vgt_mult = [
            0,
            -2*BETA*LAMBDA*RDS**2,
            2*BETA*RDS*(2*LAMBDA*VDD + 1),
            2*BETA*VDD*(-LAMBDA*VDD - 1)
        ]
        # 2nd degree polynomial coeffs for the first derivative Id' in the triode region
        self.a_triode_diff = [
            # a0
            3*BETA*LAMBDA*RDS**2*(-RDS + 2*RS),
            # a1
            2*BETA*RDS*(3*LAMBDA*RDS*VDD - 4*LAMBDA*RS*VDD + RDS - 2*RS),
            # a2
            -3*BETA*LAMBDA*RDS*VDD**2 + 2*BETA*LAMBDA*RS*VDD**2 - 2*BETA*RDS*VDD + 2*BETA*RS*VDD + 1,
        ]
        # factor to be multiplied with Vgt and added to each a_triode coefficient
        self.a_triode_diff_vgt_mult = [
            0,
            -4*BETA*LAMBDA*RDS**2,
            2*BETA*RDS*(2*LAMBDA*VDD + 1),
        ]

        # 3rd degree polynomial coeffs for the Id in the saturation region
        self.a_sat = [
            # a0
            BETA*LAMBDA*RDS*RS**2,
            # a1
            BETA*RS**2*(-LAMBDA*VDD - 1),
            # a2
            1,
            # a3
            0
        ]
        # factor to be multiplied with Vgt and added to each a_sat coefficient
        self.a_sat_vgt_mult = [
            0,
            -2*BETA*LAMBDA*RDS*RS,
            2*BETA*RS*(LAMBDA*VDD + 1),
            0
        ]
        # factor to be multiplied with Vgt**2 and added to each a_sat coefficient
        self.a_sat_vgt2_mult = [
            0,
            0,
            BETA*LAMBDA*RDS,
            BETA*(-LAMBDA*VDD - 1)
        ]
        # 2nd degree polynomial coeffs for the first derivative Id' in the saturation region
        self.a_sat_diff = [
            # a0
            3*BETA*LAMBDA*RDS*RS**2,
            # a1
            -2*BETA*RS*(LAMBDA*RS*VDD + RS),
            # a2
            1
        ]
        # factor to be multiplied with Vgt and added to each a_sat_diff coefficient
        self.a_sat_diff_vgt_mult = [
            0,
            -4*BETA*RS*LAMBDA*RDS,
            2*BETA*RS*(LAMBDA*VDD + 1)
        ]
        # factor to be multiplied with Vgt**2 and added to each a_sat_diff coefficient
        self.a_sat_diff_vgt2_mult = [
            0,
            0,
            BETA*LAMBDA*RDS
        ]

        self.BETA = BETA
        self.LAMBDA = LAMBDA
        self.VTO = VTO
        self.RD = RD
        self.RS = RS
        self.VDD = VDD

    def clone(self, dtype):
        o = jfet(
            self.BETA,
            self.LAMBDA,
            self.VTO,
            self.RD,
            self.RS,
            self.VDD,
            dtype
        )
        return o

    def calc(self, Vin, temp_c=20.0):
        Vgt = Vin - self.VTO
        Id = np.zeros(len(Vgt))

        # Triode
        Id_triode = np.zeros([2, len(Vgt)])
        beta_rs = self.BETA * self.RS
        rds = (self.RD + self.RS)
        beta_rds = self.BETA * rds
        beta_sq = self.BETA ** 2
        rds_sq = rds ** 2

        delta = 4*beta_sq*rds_sq*Vgt**2 - 8*beta_sq*rds*self.RS*self.VDD*Vgt + 4*beta_sq * \
            self.RS**2*self.VDD**2 - 4*beta_rds*self.VDD + \
            4*beta_rds*Vgt + 4*beta_rs*self.VDD + 1
        Id_triode[0] = (beta_rds*self.VDD - beta_rds*Vgt - beta_rs *
                        self.VDD - np.sqrt(delta)/2 - 1/2)/(beta_rds*(rds - 2*self.RS))
        Id_triode[1] = (beta_rds*self.VDD - beta_rds*Vgt - beta_rs *
                        self.VDD + np.sqrt(delta)/2 - 1/2)/(beta_rds*(rds - 2*self.RS))
        Vs_triode = Id_triode * self.RS
        Vgs_m_Vt_triode = Vin - Vs_triode - self.VTO
        Vds_m_Vt_triode = self.VDD - Id_triode * rds

        # Saturation
        Id_sat = np.zeros([2, len(Vgt)])
        beta_2 = 2 * self.BETA
        beta_2_rs_vgt = beta_2 * self.RS * Vgt
        delta = 2 * beta_2_rs_vgt + 1
        Id_sat[0] = (beta_2_rs_vgt - np.sqrt(delta) + 1) / \
            (beta_2*self.RS**2)
        Id_sat[1] = (beta_2_rs_vgt + np.sqrt(delta) + 1) / \
            (beta_2*self.RS**2)
        Vs_sat = Id_sat * self.RS
        Vgs_m_Vt_sat = Vin - Vs_sat - self.VTO
        Vds_m_Vt_sat = self.VDD - Id_sat * rds

        if self.ch_len_mod:
            # Newton–Raphson optimization
            # Triode
            a = self.a_triode
            for i, a_vgt_mult in enumerate(self.a_triode_vgt_mult):
                a[i] += a_vgt_mult * Vgt

            a_diff = self.a_triode_diff
            for i, a_vgt_mult in enumerate(self.a_triode_diff_vgt_mult):
                a_diff[i] += a_vgt_mult * Vgt

            f_x0 = np.polyval(a, Id_triode)
            f_x0_diff = np.polyval(a_diff, Id_triode)
            Id_triode = Id_triode - f_x0 / f_x0_diff
            
            # Saturation
            a = self.a_sat
            for i, (a_vgt_mult, a_vgt2_mult) in enumerate(zip(self.a_sat_vgt_mult, self.a_sat_vgt2_mult)):
                a[i] += a_vgt_mult * Vgt
                a[i] += a_vgt2_mult * Vgt**2

            a_diff = self.a_sat_diff
            for i, (a_vgt_mult, a_vgt2_mult) in enumerate(zip(self.a_sat_diff_vgt_mult, self.a_sat_diff_vgt2_mult)):
                a_diff[i] += a_vgt_mult * Vgt
                a_diff[i] += a_vgt2_mult * Vgt**2

            f_x0 = np.polyval(a, Id_sat)
            f_x0_diff = np.polyval(a_diff, Id_sat)
            Id_sat = Id_sat - f_x0 / f_x0_diff

        ind_triode = (Vds_m_Vt_triode <= Vgs_m_Vt_triode) & (
            Vgs_m_Vt_triode > 0)
        ind_sat = (Vds_m_Vt_sat >= Vgs_m_Vt_sat) & (Vgs_m_Vt_sat > 0)

        # Triode: only take the indexes, in which physics make sense
        if ind_triode[0].any():
            Id[ind_triode[0]] = Id_triode[0, ind_triode[0]]
        elif ind_triode[1].any():
            Id[ind_triode[1]] = Id_triode[1, ind_triode[1]]
        # Saturation: only take the indexes, in which physics make sense
        if ind_sat[0].any():
            Id[ind_sat[0]] = Id_sat[0, ind_sat[0]]
        elif ind_sat[1].any():
            Id[ind_sat[1]] = Id_sat[1, ind_sat[1]]

        y = self.VDD - Id * self.RD

        dic_y = {
            "y_id": Id,
            "y_vout": y
        }

        return dic_y


def main():
    calc_jfet_coeffs = True
    write_wav = False
    F0 = 20  # [Hz]
    FS = 48000  # [Hz]
    temp_c = 20  # [celsius]
    A = 1.3  # [V] amplitude, linear
    T = 0.1  # [seconds]

    # Ground truth: spice model
    # model = 'J2SK170'
    model = 'LSK189A'
    fpath = osp.dirname(osp.abspath(__file__))
    spice_library = SpiceLibrary(osp.join(fpath, "spicelib"), recurse=True)
    circuit = Circuit('JFET Distortion')
    circuit.include(spice_library[model])
    # component values
    Vdd = 9.0
    Rg1 = 0
    Rg2 = 1E6
    Rd = 4.4E3
    Rload = 10E3
    # C1 = 10E-6
    Cout = 10E-6
    # Rgains = [1E3, 5E3]
    Rgains = [1E3]

    # Netlist
    circuit.V('Vdd', 'vdd', circuit.gnd, Vdd)
    circuit.SinusoidalVoltageSource(
        'in', 'input', circuit.gnd, amplitude=A, frequency=F0)
    circuit.R('Rg1', 'input', 'gate', Rg1)
    circuit.R('Rg2', 'gate', circuit.gnd, Rg2)
    circuit.R('Rd', 'vdd', 'drain', Rd)
    Rgain = circuit.R('Rs', 'source', circuit.gnd, Rgains[0])
    # circuit.C('C1', 'source', 'rgain', C1)
    # Rgain = circuit.R('Rgain', 'rgain', circuit.gnd, Rgains[0])
    # circuit.C('Cout', 'drain', 'output', Cout)
    # circuit.R('Rload', 'output', circuit.gnd, Rload)
    circuit.J('JFET', 'drain', 'gate', 'source', model=model)

    parsed_model_jfet = next(x for x in SpiceParser(
        spice_library[model]).models if x.name == model)
    jfet_spice_params = {
        p.upper(): parsed_model_jfet._parameters[p]
        for p in parsed_model_jfet._parameters
    }

    if calc_jfet_coeffs:
        Svdd, Sid, Srds, Svgt, Srs, Sbeta, Slambda = symbols(
            'VDD ID RDS VGT RS BETA LAMBDA')
        Svds = Svdd - Sid * Srds
        SChLenMod = 1 + Slambda * Svds

        # Triode region
        # Complete equation, from which the derivative is taken and
        # will be used in the Newton - Raphson optimization later
        print("--------------------------------------------------------------------------------")
        expr_triode = Sid - 2 * Sbeta * SChLenMod * \
            ((Svgt - Sid * Srs) * Svds - (Svds ** 2) / 2)
        expr_triode = simplify(expr_triode)
        expr_triode, _ = poly_from_expr(expr_triode, Sid)
        print("Id_triode coeffs:")
        for i, c in enumerate(expr_triode.all_coeffs()):
            print(f"  [{i}]")
            poly_vgt, _ = poly_from_expr(c, Svgt)
            for j, cc in enumerate(poly_vgt.all_coeffs()[::-1]):
                print(f"    [Vgt^{j}]: {simplify(cc)}")

        expr_triode_diff = simplify(diff(expr_triode, Sid))
        expr_triode_diff, _ = poly_from_expr(expr_triode_diff, Sid)
        print("\nId_triode' coeffs:")
        for i, c in enumerate(expr_triode_diff.all_coeffs()):
            print(f"  [{i}]")
            poly_vgt, _ = poly_from_expr(c, Svgt)
            for j, cc in enumerate(poly_vgt.all_coeffs()[::-1]):
                print(f"    [Vgt^{j}]: {simplify(cc)}")

        # Ignoring channel effect, thus simplifying the polynomial into a second degree
        expr_triode_simp = Sid - 2 * Sbeta * \
            ((Svgt - Sid * Srs) * Svds - (Svds ** 2) / 2)
        expr_triode_simp = simplify(expr_triode_simp)
        expr_triode_simp, _ = poly_from_expr(expr_triode_simp, Sid)
        sol = solve(expr_triode_simp, Sid)
        print("\nId_triode roots:")
        for i, s in enumerate(sol):
            print(f"  [{i}]: {simplify(s)}")

        # Saturation region:
        # Complete equation, from which the derivative is taken and
        # will be used in the Newton - Raphson optimization later
        print("\n--------------------------------------------------------------------------------")
        expr_sat = Sid - Sbeta * SChLenMod * ((Svgt - Sid * Srs) ** 2)
        expr_sat = simplify(expr_sat)
        expr_sat, _ = poly_from_expr(expr_sat, Sid)
        print("Id_sat coeffs:")
        for i, c in enumerate(expr_sat.all_coeffs()):
            print(f"  [{i}]")
            poly_vgt, _ = poly_from_expr(c, Svgt)
            for j, cc in enumerate(poly_vgt.all_coeffs()[::-1]):
                print(f"    [Vgt^{j}]: {simplify(cc)}")

        expr_sat_diff = simplify(diff(expr_sat, Sid))
        expr_sat_diff, _ = poly_from_expr(expr_sat_diff, Sid)
        print("\nId_sat' coeffs:")
        for i, c in enumerate(expr_sat_diff.all_coeffs()):
            print(f"  [{i}]")
            poly_vgt, _ = poly_from_expr(c, Svgt)
            for j, cc in enumerate(poly_vgt.all_coeffs()[::-1]):
                print(f"    [Vgt^{j}]: {simplify(cc)}")

        # Ignoring channel effect, thus simplifying the polynomial into a second degree
        expr_sat_simp = Sid - Sbeta * ((Svgt - Sid * Srs) ** 2)
        expr_sat_simp = simplify(expr_sat_simp)
        expr_sat_simp, _ = poly_from_expr(expr_sat_simp, Sid)
        sol = solve(expr_sat_simp, Sid)
        print("\nId_sat roots:")
        for i, s in enumerate(sol):
            print(f"  [{i}]: {simplify(s)}")

        # limit region
        # Vds = Vgs - Vt
        # Vds = Vdd - Id * Rds
        print("\n--------------------------------------------------------------------------------")
        expr_limit = Sid - Sbeta * (Svdd - Sid * Srds)**2
        print("Id at the limit region:")
        sol = solve(expr_limit, Sid)
        for i, s in enumerate(sol):
            test_id = s.subs([
                (Sbeta, Float(jfet_spice_params["BETA"])),
                (Srds, Rd+Rgains[0]),
                (Svdd, Vdd
                 )])
            print(f"{i}: {s}")
            print(f"  Id -> {test_id}")
            test_vd = Vdd - test_id * Rd
            test_vg = test_vd + Float(jfet_spice_params["VTO"])
            test_vs = test_id * Rgains[0]
            print(f"  Vg -> {test_vg}")
            print(f"  Vgs -> {test_vg-test_vs}")
            print(f"  Vds -> {test_vd-test_vs}")
        print("\n--------------------------------------------------------------------------------\n")

    plt.figure()
    max_diff_model_id = 0
    max_diff_model_vout = 0
    max_diff_model_id_norm = 0
    max_diff_dtype = 0

    for r in Rgains:
        # Spice model
        Rgain.resistance = r
        simulator = circuit.simulator(
            temperature=temp_c, nominal_temperature=temp_c)
        # analysis = simulator.dc(Vin=Vsl)
        analysis = simulator.transient(step_time=1/FS, end_time=T)
        y_vout_spice = np.array(analysis["drain"])
        y_id_spice = (np.array(analysis["vdd"]) -
                      np.array(analysis["drain"])) / Rd
        # if > 0 then saturation, else triode
        y_vgt_spice = np.array(
            analysis["gate"]) - np.array(analysis["source"]) - Float(jfet_spice_params["VTO"])
        y_vds_spice = np.array(
            analysis["drain"]) - np.array(analysis["source"])
        y_limit_spice = y_vds_spice - y_vgt_spice
        Vin_spice = np.array(analysis["input"])
        t = np.array(analysis.time)

        jfet_float64 = jfet(
            Float(jfet_spice_params["BETA"]),
            Float(jfet_spice_params["LAMBDA"]),
            Float(jfet_spice_params["VTO"]),
            Float(jfet_spice_params["RD"]) + Rd,
            Float(jfet_spice_params["RS"]) + r,
            Vdd,
            type=np.float64
        )
        jfet_float32 = jfet_float64.clone(np.float32)
        y_f64 = jfet_float64.calc(Vin_spice * Rg2 / (Rg2 + Rg1), temp_c)
        y_f32 = jfet_float32.calc(Vin_spice * Rg2 / (Rg2 + Rg1), temp_c)

        if write_wav:
            amplitude = np.iinfo(np.int16).max
            data = amplitude * y_f32["y_vout"]
            wavfile.write(
                f"AtomJFET_Sine_20Hz_100ms_{20.0 * np.log10(max_diff_model_id):.1f}dB_Morph0ms_1ch_R{r}.wav", 48000, data.astype(np.int16))

        max_diff = max(abs(y_f64["y_id"] - y_id_spice))
        max_diff_model_id = max(max_diff_model_id, max_diff)
        max_diff_model_id_norm = max(
            max_diff_model_id, max_diff / (max(y_id_spice) - min(y_id_spice)))
        
        max_diff = max(abs(y_f64["y_id"] - y_f32["y_id"]))
        max_diff_dtype = max(max_diff_dtype, max_diff)

        max_diff = max(abs(y_f64["y_vout"] - y_vout_spice))
        max_diff_model_vout = max(max_diff_model_vout, max_diff)

        # Current subplot
        plt.subplot(211)
        plt.plot(t, y_f64["y_id"], label=f"Id_model@R={str(r)}")
        plt.plot(t, y_id_spice, label=f"Id_spice@R={str(r)}")
        plt.plot(t, abs(y_f64["y_id"] - y_id_spice), label=f"Diff@R={str(r)}")
        # Voltage subplot
        plt.subplot(212)
        #plt.plot(t, y_vgt_spice, label=f"Vgt_spice@R={str(r)}")
        #plt.plot(t, y_limit_spice, label=f"Vds-Vgt@R={str(r)}")
        plt.plot(t, y_vout_spice, label=f"Vout_spice@R={str(r)}")
        plt.plot(t, y_f64["y_vout"], label=f"Vout_model@R={str(r)}")
        plt.plot(t, abs(y_f64["y_vout"] - y_vout_spice), label=f"Diff@R={str(r)}")


    # Plot Vin after because it is the same for all Rs in the loop above
    # plt.plot(t, Vin, label="Vin")

    if write_wav:
        amplitude = np.iinfo(np.int16).max
        data = amplitude * Vin_spice
        wavfile.write(
            "Triangle_1Hz_1s_0dB.wav", 48000, data.astype(np.int16))

    plt.subplot(211)
    plt.title('Id x T')
    plt.xlabel('Id [A]')
    plt.ylabel('Time [s]')
    plt.margins(0, 0.1)
    plt.grid(which='both', axis='both')
    plt.legend()

    plt.subplot(212)
    plt.title('V x T')
    plt.xlabel('Time [s]')
    plt.ylabel('Vout [V]')
    plt.margins(0, 0.1)
    plt.grid(which='both', axis='both')
    plt.legend()

    print(f"Max diff dtype: {20.0 * np.log10(max_diff_dtype):.1f}dB")
    print(f"Max diff model Id: {20.0 * np.log10(max_diff_model_id):.1f}dB")
    print(f"Max diff model Id norm: {20.0 * np.log10(max_diff_model_id_norm):.1f}dB")
    print(f"Max diff model Vout: {20.0 * np.log10(max_diff_model_vout):.1f}dB")
    # print(f"Max diff dtype: {20.0 * np.log10(max_diff_dtype):.1f}dB")

    plt.show()


if __name__ == "__main__":
    main()
