'''
Created on 28 Mar 2019

@author: andraspalfi
'''
import numpy as np


class DelayLine:
    """ class DelayLine
    TODO documentation
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
            Initial delay value in milliseconds.
            By default it takes the value of the max_delay_ms argument.

        Raises:
        _______
        ValueError: if the delay_ms parameter exceeds the max_delay_ms argument or negative
        """
        # default value of initial delay is the maximal delay
        if delay_ms is None:
            delay_ms = max_delay_ms

        # if provided delay is out of bounds
        if delay_ms > max_delay_ms:
            raise ValueError('delay parameter cannot exceed the maximal delay')
        if delay_ms < 0.0:
            raise ValueError('delay parameter cannot be negative')

        self.fs = fs
        self.max_delay_samples = int((max_delay_ms * 0.001) * self.fs) + 1
        self.delay_samples = int((delay_ms * 0.001) * self.fs)
        self.delay_line = np.zeros(self.max_delay_samples)

        # initialize read and write pointers
        self.wp = 0
        self.rp = self.max_delay_samples - self.delay_samples

    def set_delay_ms(self, delay_ms):
        """set_delay_ms(self, delay_ms):

        This method sets the instantaneous delay of the delay line.

        Parameters:
        ___________
        delay_ms : float
            Delay value in milliseconds.

        """
        # bound delay value between 0 and max
        d_ms = max(min(delay_ms, self.max_delay_samples), 0.0)
        # update the internal state
        self.delay_samples = int((d_ms * 0.001) * self.fs)
        # update the read pointer with the new delay
        self.rp = (self.wp - self.delay_samples) % self.max_delay_samples

    def process(self, x):
        """process(self, x):

        Performs delaying of the input by circular buffering

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

    # test signal
    # read in wav file
    infile_name = 'clean_guitar_melody' # 'whitenoise'  'clean_funky_lick_90bpm' 'clean_pop_strum_100bpm'
    outfile_name = infile_name + '_flanger'
    
    
    infile_path = '../../input_audio/' + infile_name + '.wav'
    (fs, x) = read_audio(infile_path)
    leng_samp = x.__len__()

#     fs = 48000
#     len_s = 5
#     leng_samp = len_s * fs
#     x = np.sin(2 * np.pi * 500 * t_ax)

    t_ax = np.r_[0: (leng_samp) / fs: 1 / fs]
    y = np.zeros(x.__len__())

    # LFO
    rate_bpm = 180 / 3
    rate_lfo_hz = rate_bpm / 60
    min_lfo = 0
    max_lfo = 5.0 / rate_lfo_hz  # the 'depth' control
    amp_lfo = max_lfo * 0.5
    offset_lfo = (max_lfo + min_lfo) * 0.5
    lfo = amp_lfo * np.sin(2 * np.pi * rate_lfo_hz * t_ax) + offset_lfo

    dl = DelayLine(fs, max_lfo+1.0)

    for (i, xn) in enumerate(x):
        y[i] = dl.process(x[i])
        dl.set_delay_ms(lfo[i])

#     f, (ax, ax2) = plt.subplots(2, 1)
#     ax.plot(x)
#     ax.plot(y)
#     ax2.plot(lfo)
# 
#     plt.show()

    out = (y + x) * 0.5
    #out = y
    outfile_path = '../../output_audio/vibrato/' + outfile_name + '.wav'
    write_audio(out, fs, outfile_path)
