'''
Created on 25 Mar 2019

@author: andraspalfi
'''
import numpy as np

# TODO:
# Common ancestor class for these two with inheritance?


class SinusoidalLowFrequencyOscillator:
    """class SinusoidalLowFrequencyOscillator

    This class implements a sinusoidal oscillator by a recursive algorithm.
    """

    def __init__(self, f, fs, amp=1.0, offset=0.0, clip_h=1.0, clip_l=-1.0):
        """ Constructor
        The method acts as a constructor for the SinusoidalLowFrequencyOscillator class.

        Parameters
        ----------
        f : float
            Frequency of the desired sinusoid in Hz
        fs
            Sampling rate in Hz
        amp
            amplitude of the sinusoid (default: 1.0)
        offset
            dc offset of the sinusioud (default: 0.0)
        clip_h
            Upper clipping level (default: 1.0)
        clip_l
            Lower clipping level (default: -1.0)
        """
        self.fs = fs
        self.f = f
        self.amp = amp
        self.offset = offset
        self.clip_h = clip_h
        self.clip_l = clip_l

        # init - freq. dependent constants
        self._recalculate_freq_dependent_variables()

        # init - inner states
        self.u = 1.0  # cos(w)
        self.v = 0.0  # sin(w)

    def _recalculate_freq_dependent_variables(self):
        """
         _recalculate_freq_dependent_variables(self):

        This private method recalculates internal parameters when settings are changed.
        """
        # reinit freq. dependent constants
        self.w = 2.0 * np.pi * self.f / self.fs
        self.k1 = np.tan(0.5 * self.w)
        self.k2 = 2.0 * self.k1 / (1.0 + self.k1**2)

    def set_freq(self, f):
        """
        set_freq(self, f):

        This method sets the frequency of the oscillator.

        Parameters
        ----------
        f : float
            Frequency of the desired sinusoid in Hz
        """
        # set new frequency
        self.f = f
        self._recalculate_freq_dependent_variables

    def set_amp(self, amp):
        """
        set_amp(self, amp):

        This method sets the amplitude of the oscillator.

        Parameters
        ----------
        amp : float
            Amplitude of the desired sinusoid
        """
        # set new amplitude
        self.amp = amp

    def set_offset(self, offset):
        """
        set_offset(self, offset):

        This method sets the offset of the oscillator.

        Parameters
        ----------
        offset : float
            offset of the desired sinusoid
        """
        # set new offset
        self.offset = offset

    def set_clip_values(self, clip_h, clip_l):
        """
        set_clip_values(self, clip_h, clip_l):

        This method sets the clip values of the oscillator.

        Parameters
        ----------
        clip_h : float
            Upper clip value of the desired sinusoid
        clip_l : float
            Lower clip value of the desired sinusoid
        """
        # set new clip_values
        self.clip_h = clip_h
        self.clip_l = clip_l

    def reinit_phase(self):
        """
        reinit_phase(self):

        This method triggers re-initialization of the phase of the output sinusoid.
        """
        # reset inner states
        self.u = 1.0  # cos(w)
        self.v = 0.0  # sin(w)

    def generate(self):
        """
        generate(self):

        This method generates the next sample of the sinusoidal output.
        """
        # The recursive algorithm
        vv = self.u - self.k1 * self.v
        self.v = self.v + self.k2 * vv  # sin(w)
        self.u = vv - self.k1 * self.v  # cos(w)

        # offset and clip output
        return (max(min((self.amp * self.v + self.offset), self.clip_h), self.clip_l))


class SawtoothLowFrequencyOscillator:
    """class SawtoothLowFrequencyOscillator

    This class implements a sawtooth waveform generator.
    """

    def __init__(self, f, fs, balance=0.5, amp=1.0, offset=0.0, clip_h=1.0, clip_l=-1.0):
        """ Constructor
        The method acts as a constructor for the SawtoothLowFrequencyOscillator class.

        Parameters
        ----------
        f : float
            Frequency of the desired sawtooth waveform in Hz
        fs
            Sampling rate in Hz
        balance:
            ratio between rise time and fall time (or upwards and downwards slope)
            of the sawtooth waveform. Default: 0.5 (equal rise and fall)
        amp
            amplitude of the sawtooth waveform (default: 1.0)
        offset
            dc offset of the sawtooth waveform (default: 0.0)
        clip_h
            Upper clipping level (default: 1.0)
        clip_l
            Lower clipping level (default: -1.0)
        """
        self.fs = fs
        self.f = f
        self.balance = balance
        self.amp = amp
        self.offset = offset
        self.clip_h = clip_h
        self.clip_l = clip_l

        self._recalculate_private_variables()

        # init - inner states
        self.y = 0.0    # output sample
        self.d = self.d_rise  # init differential

    def _recalculate_private_variables(self):
        """
         _recalculate_private_variables(self):

        This private method recalculates internal parameters when settings are changed.
        """
        # calculate internal constants
        # duration of a full period in samples (rounded to the nearest sample)
        self.T = round(self.fs / self.f)
        # duration of rise and fall periods in samples
        self.t1 = self.T * self.balance
        self.t2 = self.T - self.t1
        # rise and fall differentials during one sample
        self.d_rise = 2.0 * self.amp / self.t1
        self.d_fall = -2.0 * self.amp / self.t2

    def set_freq(self, f):
        """
        set_freq(self, f):

        This method sets the frequency of the sawtooth wave generator.

        Parameters
        ----------
        f : float
            Frequency of the desired sawtooth waveform in Hz
        """
        # set new frequency
        self.f = f
        self._recalculate_private_variables

        # TODO: IS IT NECESSARY TO RESET THE PHASE OR CAN THIS BE CONTINUOUS?

    def set_balance(self, balance):
        """
        set_balance(self, balance):

        This method sets the balance between rise and fall periods of the sawtooth wave generator.

        Parameters
        ----------
        balance : float
            ratio between rise time and fall time (or upwards and downwards slope)
            of the sawtooth waveform. 0.5 corresponds to equal rise and fall
        """
        # set new frequency
        self.balance = balance
        self._recalculate_private_variables

    def set_amp(self, amp):
        """
        set_amp(self, amp):

        This method sets the amplitude of the sawtooth wave generator.

        Parameters
        ----------
        amp : float
            Amplitude of the sawtooth wave generator.
        """
        # set new frequency
        self.amp = amp
        self._recalculate_private_variables

    def set_offset(self, offset):
        """
        set_offset(self, offset):

        This method sets the offset of the oscillator.

        Parameters
        ----------
        offset : float
            offset of the desired sinusoid
        """
        # set new offset
        self.offset = offset

    def set_clip_values(self, clip_h, clip_l):
        """
        set_clip_values(self, clip_h, clip_l):

        This method sets the clip values of the oscillator.

        Parameters
        ----------
        clip_h : float
            Upper clip value of the desired sinusoid
        clip_l : float
            Lower clip value of the desired sinusoid
        """
        # set new clip_values
        self.clip_h = clip_h
        self.clip_l = clip_l

    def reinit_phase(self):
        """
        reinit_phase(self):

        This method triggers re-initialization of the phase of the output sinusoid.
        """
        # reinit inner states
        self.y = 0.0    # output sample
        self.d = self.d_rise  # init differential

    def generate(self):
        """
        generate(self):

        This method generates the next sample of the sawtooth waveform output.
        """
        # recursive modified sawtooth waveform generation
        if (self.y >= self.amp):
            self.d = self.d_fall
        elif (self.y <= -self.amp):
            self.d = self.d_rise

        # update new sample with differential
        self.y = self.y + self.d

        # offset and clip output
        return (max(min((self.y + self.offset), self.clip_h), self.clip_l))


if __name__ == '__main__':
    import matplotlib as mpl
    import matplotlib.pyplot as plt

    # LFO parameters
    fs = 48000     # sampling freq
    bpm = 60       # LFO frequency (BPM)
    rise_fall_balance = 0.8  # rise / fal balance (between 0 and 1)
    amp = 1.3        # waveform amplitude
    offset = 0.4   # DC offset
    clip_h = 1.4   # clip amplitude - upper
    clip_l = -0.5   # clip amplitude - lower
    waveform = 'sawtooth'  # 'sine' of 'sawtooth'

    # for plotting waveform
    block_size = 128    # size of one block in samples
    sec_per_block = 3   # no. of seconds one sample block corresponds to

    # prealloc output signal buffer
    leng = 3 * fs
    t_ax = np.r_[0: (leng-1)/fs: 1/fs]
    y = np.zeros(leng)

    sinegen = SinusoidalLowFrequencyOscillator(bpm/60,
                                               fs,
                                               amp,
                                               offset,
                                               clip_h,
                                               clip_l)

    sawtoothgen = SawtoothLowFrequencyOscillator(bpm/60,
                                                 fs,
                                                 rise_fall_balance,
                                                 amp,
                                                 offset,
                                                 clip_h,
                                                 clip_l)

    for i, t in enumerate(t_ax):
        y[i] = sawtoothgen.generate()
        # y[i] = sinegen.generate()

    plt.plot(y)
    plt.show()
