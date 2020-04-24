#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Created on Sat Apr 25 00:01:49 2020

@author: andraspalfi
"""
import numpy as np
from lfo import SinusoidalLowFrequencyOscillator


class Tremolo:
    """ class Tremolo

    This class implements a DAFX Tremolo object.
    """

    def __init__(self, fs, rate_bpm=300, depth_percent=50):
        """ __init__(self, fs, max_delay_ms, delay_ms=None):

        This method is the Constructor of the Tremolo class

        Parameters:
        ___________
        fs : int
            sampling rate in hz
        rate_bpm : int
            frequency of oscillation in BPM
        depth_percent : int
            tremolo amplitude (depth) in percent

        Raises:
        _______
        ValueError:
            For negative parameters or depth higher than 100%
        """
        if (fs <= 0):
            raise ValueError('Sampling rate must be positive')
        if (rate_bpm < 0):
            raise ValueError('Tremolo oscillation rate cannot be negative')
        if (depth_percent < 0) or (depth_percent > 100):
            raise ValueError('Tremolo depth must be between 0 and 100 percent')

        self.fs = fs
        self.rate_bpm = rate_bpm
        self.f = float(self.rate_bpm) / 60.0
        self.depth_percent = depth_percent
        self.amp = float(depth_percent) * 0.01 * 0.5 # Peak-to peak amplude * 1/2
        self.offset = 1.0 - self.amp  # so that LFO signal peak is at 1

        self.lfo = SinusoidalLowFrequencyOscillator(self.f, fs, self.amp, self.offset)

    def set_rate_bmp(self, rate_bpm):
        """
        set_rate_bmp(self, rate_bpm):

        This method sets the oscillation rate (in BPM) of the Tremolo.

        Parameters
        ----------
        rate_bpm : int
            frequency of oscillation in BPM
        Raises:
        _______
        ValueError:
            For negative rate value
        """
        if (rate_bpm < 0):
            raise ValueError('Tremolo oscillation rate cannot be negative')

        self.rate_bpm = rate_bpm
        self.f = float(self.rate_bpm) / 60.0
        self.lfo.set_freq(self.f)

    def set_depth_percent(self, depth_percent):
        """
        set_depth_percent(self, depth_percent):

        This method sets the depth of the oscillator.

        Parameters
        ----------
        depth_percent : int
            tremolo amplitude (depth) in percent

        Raises:
        _______
        ValueError:
            For depth negative or higher than 100%
        """
        if (depth_percent < 0) or (depth_percent > 100):
            raise ValueError('Tremolo depth must be between 0 and 100 percent')

        self.depth_percent = depth_percent
        self.amp = float(depth_percent) * 0.01 * 0.5 # Peak-to peak amplude * 1/2
        self.offset = 1.0 - self.amp  # so that LFO signal peak is at 1

        self.lfo.set_amp(self.amp)
        self.lfo.set_offset(self.offset)

    def process(self, x):
        """process(self, x):

        Performs tremolo on the input, sample by sample operation

        Parameters:
        -----------
        x: float
            input sample

        Returns:
        ________
        y: float
            output sample
        gain: float
            gain (for debugging)
        """
        gain = self.lfo.generate()
        y = x * gain

        return (y, gain)


if __name__ == '__main__':
    import matplotlib.pyplot as plt
    from dafx_audio_io import read_audio, write_audio

    # %% -- Demonstrating tremolo effect
    # A clean input file is read, onto which the tremolo effect is applied

    rate_bpm = 400
    depth_percent = 50

    # input signal -  read in wav file
    infile_name = 'clean_guitar_melody'  # 'whitenoise'  'clean_funky_lick_90bpm' 'clean_pop_strum_100bpm'
    outfile_name = infile_name + '_tremolo'

    infile_path = '../../input_audio/' + infile_name + '.wav'
    (fs, x) = read_audio(infile_path)
    leng_samp = x.__len__()
    t_ax = np.r_[0: (leng_samp) / fs: 1 / fs]
    y = np.zeros(x.__len__())

    # snoop the gain values too
    gain_snoop = np.zeros(x.__len__())

    # # Uncomment this to use a sinusoid as an input signal instead
    # outfile_name = 'sinusoid_tremolo'
    # fs = 48000
    # freq_hz = 500
    # amp = 0.5
    # len_s = 5
    # leng_samp = len_s * fs
    # x = amp * np.sin(2.0 * np.pi * freq_hz * t_ax)

    # create Tremolo object
    trem = Tremolo(fs, rate_bpm, depth_percent)

    # processing the signal
    for (i, xn) in enumerate(x):
        (y[i], gain_snoop[i]) = trem.process(x[i])

    # f, (ax, ax2, ax3) = plt.subplots(3, 1)
    # ax.plot(x)
    # ax2.plot(y)
    # ax3.plot(gain_snoop)
    # plt.show()

    outfile_path = '../../output_audio/tremolo/' + outfile_name + '.wav'
    write_audio(y, fs, outfile_path)
