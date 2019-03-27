'''
Created on 27 Mar 2019

@author: andraspalfi
'''
import numpy as np
from crybaby_params import crybaby_params as param


class CryBaby:
    """ class CryBaby

    This class implements a Crybaby DAFX object
    The analog circuitry of a Dunlop Crybaby pedal is modelled and the time-varying
    filter transfer function inferred.

    The only adjustable parameters are the sampling frequency, pedal position and the mix balance
    between bandpassed and direct signals
    """

    def __init__(self, fs):
        """ Constructor
        The method acts as a constructor for the CryBaby class.

        Parameters
        ----------
        fs
            Sampling rate in Hz
        """
        self.fs = fs

        self.gf = param['gf']
        self.Cf = param['Cf']
        self.Ci = param['Ci']
        self.re = param['re']
        self.Rp = param['Rp']
        self.Lp = param['Lp']
        self.Ri = param['Ri']
        self.Rs = param['Rs']
        self.RpRi = param['RpRi']
        self.f0 = param['f0']
        self.Q = param['Q']
        self.Gi = param['Gi']
        self.gbpf = param['gbpf']
        self.pedal_max = param['pedal_max']
        self.pedal_min = param['pedal_min']

        # useful params
        self.w0 = 2.0 * np.pi * self.f0 / self.fs
        self.c = np.cos(self.w0)
        self.s = np.sin(self.w0)
        self.alpha = self.s/(2.0 * self.Q)

        # init knob value
        self.gp = 0

        # High Pass Filter coeffs
        self.b0h = (1.0 + self.c) / 2.0
        self.b1h = -(1.0 + self.c)
        self.b2h = (1.0 + self.c) / 2.0
        self.a0h = 1.0 + self.alpha
        self.a1h = -2.0 * self.c
        self.a2h = 1.0 - self.alpha

        # Band Pass Filter coeffs
        self.b0b = self.Q * self.alpha
        # self.b1b =   0.0
        self.b2b = -self.Q * self.alpha
        self.a0b = 1.0 + self.alpha
        self.a1b = -2.0 * self.c
        self.a2b = 1.0 - self.alpha

        # Numerator coefficients
        self.b0 = self.gbpf * self.b0b + self.Gi * self.a0b
        self.b1 =                        self.Gi * self.a1b
        self.b2 = self.gbpf * self.b2b + self.Gi * self.a2b

        # Constants to make denominator coefficients computation more efficient
        # in real-time
        self.a0c = -self.gf * self.b0h
        self.a1c = -self.gf * self.b1h
        self.a2c = -self.gf * self.b2h

        # denominator coefficients (initialization)
        self.a0 = self.a0b + self.gp * self.a0c
        ax = 1.0 / self.a0
        self.a1 = (self.a1b + self.gp * self.a1c) * ax
        self.a2 = (self.a2b + self.gp * self.a2c) * ax
        self.a0 = 1.0

#          % TODO: 1-Pole HPF coeffs
#          % make this part of the signal chain either as a different module or
#          % by including in the [b, a] calculations
#
#              a1p = exp(-1/(Ri*Ci*fs));
#              g = gf*(1.0 + a1p)*0.5;  %put the forward gain into the HPF to avoid repeated
#              multiplications in real-time
#
#              %Transfer functions
#              %1-Pole HPF
#              Hin = g * (1 - z1)./(1 - a1p.*z1);

    def generate_coeffs(self, pedal_pos):
        """ generate_coeffs(self, pedal_pos):

        This method generates the CryBaby biquad coefficients based on the pedal position parameter

        Parameters
        ----------
        pedal_pos : float
            Current position of the pedal.
            Must be between 0 and 1

        Returns:
        --------
        coeffs: list
            List containing biquad coefficients
            b0 - 1st feedforward coeff
            b1 - 2nd feedforward coeff
            b2 - 3rd feedforward coeff
            a0 - 1st feedback coeff - always 1.0
            a1 - 2nd feedback coeff
            a2 - 3rd feedback coeff
        """
        # bound the pedal pos. between min and max values
        gx = max(min(pedal_pos, self.pedal_max), self.pedal_min)

        # Ever so slight gain adjustment to match analog curves.
        # This is not necessary in a real implementation since it represents
        # a fraction of a degree tilt on the treadle (6 degrees pot rotation)
        aa = -0.15
        self.gp = gx * (1.0 + aa)
        # denominator coefficients need to be computed for every change in the pot position
        # This costs 3 multiply and add plus a single 1/x operation for computation of coefficients
        # Then add 2 more multiplications for denominator and one for numerator.
        # Sum is 6 multiply and one 1/x operation per sample.  It's approximately the same
        # as running 2 fixed biquad filters, so not too bad when all is refactored
        self.a0 = self.a0b + self.gp * self.a0c
        ax = 1.0 / self.a0

        self.a0 = 1.0
        self.a1 = (self.a1b + self.gp * self.a1c) * ax
        self.a2 = (self.a2b + self.gp * self.a2c) * ax

        return [self.b0, self.b1, self.b2, self.a0, self.a1, self.a2]


if __name__ == '__main__':

    import matplotlib.pyplot as plt
    from lfo import SinusoidalLowFrequencyOscillator, SawtoothLowFrequencyOscillator
    from dafx_audio_io import read_audio, write_audio

    # file I/O
    infile_name = 'clean_funky_lick_90bpm'   # 'clean_pop_strum_100bpm' 'clean_funky_lick_90bpm'
    outfile_name = infile_name + '_crybaby'

    # user defined params
    wah_balance = 0.75      # balance between wah'ed and direct signal
    lfo_reinit_phase_s = 3  # at this point in time, the LFO phase is reinitialized

    # LFO parameters
    bpm = 90                  # LFO frequency (BPM)
    rise_fall_balance = 0.8   # rise / fal balance (between 0 and 1)
    amp = 1.3                 # waveform amplitude
    offset = 0.0              # DC offset
    clip_h = 1.4              # clip amplitude - upper
    clip_l = -0.5             # clip amplitude - lower
    waveform = 'sawtooth'     # 'sine' of 'sawtooth'

    # read in wav file
    infile_path = '../../input_audio/' + infile_name + '.wav'
    (fs, x) = read_audio(infile_path)
    leng = x.__len__()

    # time axis
    t_ax = np.r_[0: leng/fs: 1/fs]

    # prealloc signal buffers
    lfo_out = np.zeros(t_ax.__len__())
    pedal_pos = np.zeros(t_ax.__len__())
    bp_out = np.zeros(t_ax.__len__())
    crybaby_out = np.zeros(t_ax.__len__())

    # sample index where the LFO phase is reinitialized
    lfo_reinit_phase_idx = np.round(lfo_reinit_phase_s * fs)

    # objects instantiation
    if (waveform == 'sine'):
        lfo = SinusoidalLowFrequencyOscillator(bpm/60,
                                               fs,
                                               amp,
                                               offset,
                                               clip_h,
                                               clip_l)
    elif (waveform == 'sawtooth'):
        lfo = SawtoothLowFrequencyOscillator(bpm/60,
                                             fs,
                                             rise_fall_balance,
                                             amp,
                                             offset,
                                             clip_h,
                                             clip_l)

    crybaby = CryBaby(fs)

    # MAIN LOOP
    for n in range(leng):

        # trigger a phase reinit in the LFO
        if (n == lfo_reinit_phase_idx):
            lfo.reinit_phase()

        # LFO
        lfo_out[n] = lfo.generate()

        # clipping the LFO output between 0 and 1 - otherwise crybaby algo blows up!
        pedal_pos[n] = max(min(lfo_out[n], 0.99), 0.01)

        # invert the LFO output for pedal pos function:
        # zero corresponds to highest pedal position and 1 corresponds to lowest
        pedal_pos[n] = 1.0 - pedal_pos[n]

        # CALCULATE CRYBABY COEFFS
        [b0, b1, b2, a0, a1, a2] = crybaby.generate_coeffs(pedal_pos[n])

        # 2ND ORDER IIR
        # only for the first 2 samples, so we don't index into negative x
        if (n == 0):
            bp_out[n] = b0 * x[n]
        elif (n == 1):
            bp_out[n] = b0 * x[n] + b1 * x[n-1] - a1 * bp_out[n-1]
        else:
            # biquad direct form
            bp_out[n] = b0 * x[n] + b1 * x[n-1] + b2 * x[n-2] - a1 * bp_out[n-1] - a2 * bp_out[n-2]

        # TODO: 1ST ORDER HP FILTER HERE!!!

        # mixing the filtered signal with the direct signal for final output
        crybaby_out[n] = wah_balance * bp_out[n] + (1.0 - wah_balance) * x[n]

    plt.plot(lfo_out)
    plt.show()

    outfile_path = '../../output_audio/crybaby/' + outfile_name + '.wav'
    write_audio(crybaby_out, fs, outfile_path)
