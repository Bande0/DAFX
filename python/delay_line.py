'''
Created on 28 Mar 2019

@author: andraspalfi
'''
import numpy as np


class DelayLine:
    """ class DelayLine

    This class implements a simple delay line object.

    Currently, only integer sample delaying is supported.
    This results in buzzing artifacts in the output when the input is a pure sine
    """

    def __init__(self, fs, max_delay_ms, delay_ms=None):
        """ __init__(self, fs, max_delay_ms, delay_ms=None):

        This method is the Constructor of the DelayLine class

        Parameters:
        ___________
        fs : int
            sampling rate in hz
        max_delay_ms : float
            maximal delay to allocate the delayline for in milliseconds
        delay_ms : float
            Delay value in milliseconds.
            By default it takes the value of the max_delay_ms argument.

        Raises:
        _______
        ValueError: if the delay_ms parameter exceeds
        the max_delay_ms argument or negative
        """
        # default value of initial delay is the maximal delay
        if delay_ms is None:
            delay_ms = max_delay_ms

        # if provided delay is out of bounds
        if delay_ms > max_delay_ms:
            raise ValueError('delay parameter cannot exceed the maximal delay')
        if delay_ms < 0.0:
            raise ValueError('delay parameter cannot be negative')

        # set up sampling frequency and delay times in samples
        self.fs = fs
        self.max_delay_samples = int((max_delay_ms * 0.001) * self.fs) + 1
        self.delay_samples = int((delay_ms * 0.001) * self.fs)

        # allocate the delay line itself
        self.delay_line = np.zeros(self.max_delay_samples)

        # initialize read and write pointers
        self.wp = 0
        self.rp = self.max_delay_samples - self.delay_samples

    def set_delay_ms(self, delay_ms):
        """set_delay_ms(self, delay_ms):

        This method sets the instantaneous delay of the delay line.

        Currently, only integer sample delaying is implemented

        Parameters:
        ___________
        delay_ms : float
            Delay value in milliseconds.

        """
        # bound delay value between 0 and max
        d_ms = max(min(delay_ms, self.max_delay_samples), 0.0)

        # update the delay value in samples
        # OBS: the delay time is rounded to the nearest integer!
        # No fractional delaying is supported
        self.delay_samples = int((d_ms * 0.001) * self.fs)

        # update the read pointer with the new delay
        self.rp = (self.wp - self.delay_samples) % self.max_delay_samples

    def process(self, x):
        """process(self, x):

        Performs delaying of the input by circular buffering

        Currently, only integer sample delaying is implemented

        Parameters:
        -----------
        x: float
            input sample

        Returns:
        ________
        y: float
            delayed sample
        """
        rp = self.rp
        wp = self.wp

        # write new sample
        self.delay_line[wp] = x

        # advance read and write pointers circularly
        self.rp = (rp + 1) % self.max_delay_samples
        self.wp = (wp + 1) % self.max_delay_samples

        # read out sample from delay line and return
        return self.delay_line[rp]


if __name__ == '__main__':
    import matplotlib.pyplot as plt
    from dafx_audio_io import read_audio, write_audio

    # %% -- Demonstrating delay line by flanging and vibrato
    # A clean input file is read, onto which the vibrato and/or the flanging
    # effect is applied. The vibrato is achieved by running the signal through
    # a delay line with periodically varying delay (i.e. Doppler-effect).
    # Here, the delay value is controlled by an LFO.
    # The flanger effect is achieved by summing the vibrato (Doppler) signal
    # with the original clean signal.

    # input signal -  read in wav file
    infile_name = 'clean_guitar_melody'  # 'whitenoise'  'clean_funky_lick_90bpm' 'clean_pop_strum_100bpm'
    outfile_flanger_name = infile_name + '_flanger'
    outfile_vibrato_name = infile_name + '_vibrato'

    infile_path = '../../input_audio/' + infile_name + '.wav'
    (fs, x) = read_audio(infile_path)
    leng_samp = x.__len__()
    t_ax = np.r_[0: (leng_samp) / fs: 1 / fs]
    y = np.zeros(x.__len__())

    # # Uncomment this to use a sinusoid as an input signal instead
    # outfile_flanger_name = 'sinusoid_flanger'
    # outfile_vibrato_name = 'sinusoid_vibrato'
    # fs = 48000
    # freq_hz = 500
    # amp = 0.5
    # len_s = 5
    # leng_samp = len_s * fs
    # x = amp * np.sin(2.0 * np.pi * freq_hz * t_ax)

    # LFO parameters
    # pre-generating an LFO-curve (i.e. not using the DAFX LFO object)
    rate_bpm = 180 / 3
    rate_lfo_hz = rate_bpm / 60
    min_lfo = 0
    max_lfo = 5.0 / rate_lfo_hz  # the 'depth' control
    amp_lfo = max_lfo * 0.5
    offset_lfo = (max_lfo + min_lfo) * 0.5  # bring up to all non-negative values

    # generate LFO curve
    lfo = amp_lfo * np.sin(2 * np.pi * rate_lfo_hz * t_ax) + offset_lfo

    # create delayLine object
    dl = DelayLine(fs, max_lfo + 1.0)

    # processing the delay line output
    # and taking in the input from the LFO
    for (i, xn) in enumerate(x):
        y[i] = dl.process(x[i])
        dl.set_delay_ms(lfo[i])

#     f, (ax, ax2) = plt.subplots(2, 1)
#     ax.plot(x)
#     ax.plot(y)
#     ax2.plot(lfo)
#     plt.show()

    out_flanger = (y + x) * 0.5
    out_vibrato = y
    outfile_flanger_path = '../../output_audio/flanger/' + outfile_flanger_name + '.wav'
    outfile_vibrato_path = '../../output_audio/vibrato/' + outfile_vibrato_name + '.wav'
    write_audio(out_flanger, fs, outfile_flanger_path)
    write_audio(out_vibrato, fs, outfile_vibrato_path)
