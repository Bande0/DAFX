'''
Created on 27 Mar 2019

@author: andraspalfi
'''
import numpy as np
from biquad import BiquadFilter


class Crossover:
    """ class Crossover

    This class implements a 4th-order Linkwitz-Riley IIR crossover filter object
    """

    def __init__(self, fs, fc):
        """ Constructor
        The method acts as a constructor for the Crossover class.

        Parameters
        ----------
        fs
            Sampling rate in Hz
        fc
            Crossover frequency in Hz
        """
        self.fs = fs

        # instantiate biquad filters - with no coeffs yet
        # LP filters
        self.lp1 = BiquadFilter()
        self.lp2 = BiquadFilter()
        # HP filters
        self.hp1 = BiquadFilter()
        self.hp2 = BiquadFilter()

        # set crossover frequency and set filter coeffs
        self.set_fc(fc)

    def set_fc(self, fc):
        """ set_fc(self, fc):
        Sets the crossover fruequency

        Parameters
        ----------
        fc
            Crossover frequency in Hz
        """
        self.fc = fc

        w0 = 2.0 * np.pi * self.fc / self.fs
        wc = np.cos(w0)
        ws = np.sin(w0)
        # specifically for butterworth: wc/(2*Q) where Q == 1/sqrt(2)
        alpha = ws / np.sqrt(2.0)

        # butter LP coeffs
        a0_lp = 1.0 + alpha
        a1_lp = -2.0 * wc
        a2_lp = 1.0 - alpha
        b0_lp = 0.5 * (1.0 - wc)
        b1_lp = 1.0 - wc
        b2_lp = 0.5 * (1.0 - wc)

        self.a1_lp = a1_lp / a0_lp
        self.a2_lp = a2_lp / a0_lp
        self.b0_lp = b0_lp / a0_lp
        self.b1_lp = b1_lp / a0_lp
        self.b2_lp = b2_lp / a0_lp
        self.a0_lp = 1.0

        # butter HP coeffs
        a0_hp = 1.0 + alpha
        a1_hp = -2.0 * wc
        a2_hp = 1.0 - alpha
        b0_hp = -0.5 * (1.0 + wc)
        b1_hp = 1.0 + wc
        b2_hp = b0_hp

        self.a1_hp = a1_hp / a0_hp
        self.a2_hp = a2_hp / a0_hp
        self.b0_hp = b0_hp / a0_hp
        self.b1_hp = b1_hp / a0_hp
        self.b2_hp = b2_hp / a0_hp
        self.a0_hp = 1.0

        # set filter coefficients
        # LP filters
        self.lp1.set_coeffs(self.b0_lp, self.b1_lp, self.b2_lp, self.a0_lp, self.a1_lp, self.a2_lp)
        self.lp2.set_coeffs(self.b0_lp, self.b1_lp, self.b2_lp, self.a0_lp, self.a1_lp, self.a2_lp)
        # HP filters
        self.hp1.set_coeffs(self.b0_hp, self.b1_hp, self.b2_hp, self.a0_hp, self.a1_hp, self.a2_hp)
        self.hp2.set_coeffs(self.b0_hp, self.b1_hp, self.b2_hp, self.a0_hp, self.a1_hp, self.a2_hp)

    def process(self, x):
        """ process(self, x):

        Applies crossover processing on input signal on a sample by sample basis

        Parameters
        ----------
        x : float
            input sample

        Returns:
        --------
        (y_lp, y_hp): tuple of LP and HP samples after crossover

        """
        # LP
        y_lp1 = self.lp1.process_td2(x)
        y_lp2 = self.lp2.process_td2(y_lp1)
        # HP
        y_hp1 = self.hp1.process_td2(x)
        y_hp2 = self.hp2.process_td2(y_hp1)

        return (y_lp2, y_hp2)


if __name__ == '__main__':
    # import matplotlib.pyplot as plt
    from dafx_audio_io import read_audio, write_audio

    # %% -- Demonstrating Linkwitz-Riley crossover filter

    # file I/O
    infile_name = 'whitenoise_5s'  # 'whitenoise'  'clean_funky_lick_90bpm' 'clean_pop_strum_100bpm'
    outfile_lp_name = infile_name + '_crossover_lp'
    outfile_hp_name = infile_name + '_crossover_hp'
    outfile_sum_name = infile_name + '_crossover_sum'

    # read in wav file
    infile_path = '../../input_audio/' + infile_name + '.wav'
    (fs, x) = read_audio(infile_path)
    leng = x.__len__()

    # time axis
    t_ax = np.r_[0: leng / fs: 1 / fs]

    # prealloc signal buffers
    y_lp_out = np.zeros(t_ax.__len__())
    y_hp_out = np.zeros(t_ax.__len__())

    # set up crossover
    fc = 2000  # crossover frequency
    linkwitz = Crossover(fs, fc)

    # %% MAIN LOOP
    for n in range(leng):
        y_lp_out[n], y_hp_out[n] = linkwitz.process(x[n])

    outfile_path = '../../output_audio/crossover/'
    write_audio(y_lp_out, fs, outfile_path + outfile_lp_name + '.wav')
    write_audio(y_hp_out, fs, outfile_path + outfile_hp_name + '.wav')
    write_audio(y_lp_out + y_hp_out, fs, outfile_path + outfile_sum_name + '.wav')
