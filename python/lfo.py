'''
Created on 25 Mar 2019

@author: andraspalfi
'''
import numpy as np


class LowFrequencyOscillator:
    """class LowFrequencyOscillator

    This is a base class for low frequency oscillators.
    """

    def __init__(self, f, fs, amp=1.0, offset=0.0, clip_h=1.0, clip_l=-1.0):
        """ Constructor
        The method acts as a constructor for the LowFrequencyOscillator class.

        Parameters
        ----------
        f : float
            Frequency of the desired signal in Hz
        fs
            Sampling rate in Hz
        amp
            amplitude of the signal (default: 1.0)
        offset
            dc offset of the signal (default: 0.0)
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

    def set_freq(self, f):
        """
        set_freq(self, f):

        This method sets the frequency of the oscillator.

        Parameters
        ----------
        f : float
            Frequency of the desired signal in Hz
        """
        # set new frequency
        self.f = f

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
            offset of the desired signal
        """
        # set new offset
        self.offset = offset

    def set_clip_h(self, clip_h):
        """
        set_clip_h(self, clip_h):

        This method sets the upper clip value of the oscillator.

        Parameters
        ----------
        clip_h : float
            Upper clip value of the desired signal
        """
        # set new clip_h value
        self.clip_h = clip_h

    def set_clip_l(self, clip_l):
        """
        set_clip_h(self, clip_l):

        This method sets the lower clip value of the oscillator.

        Parameters
        ----------
        clip_l : float
            Lower clip value of the desired signal
        """
        # set new clip_h value
        self.clip_l = clip_l

    def _recalculate_private_variables(self):
        """
         _recalculate_private_variables(self):

        This private method recalculates internal parameters
        when settings are changed.
        Must be overridden by subclass!

        Raises:
        -------
        NotImplementedError(): raised if subclass does not override this function
        """
        raise NotImplementedError()

    def reinit_phase(self):
        """
        reinit_phase(self):

        This method triggers re-initialization of the phase of the output signal.
        Must be overridden by subclass!

        Raises:
        -------
        NotImplementedError(): raised if subclass does not override this function
        """
        raise NotImplementedError()

    def generate(self):
        """
        generate(self):

        This method generates the next sample of the output.
        Must be overridden by subclass!

        Raises:
        -------
        NotImplementedError(): raised if subclass does not override this function
        """
        raise NotImplementedError()


class SinusoidalLowFrequencyOscillator(LowFrequencyOscillator):
    """class SinusoidalLowFrequencyOscillator

    This class implements a sinusoidal oscillator by a recursive algorithm.
    """

    def __init__(self, f, fs, amp=1.0, offset=0.0, clip_h=1.0, clip_l=-1.0):
        super().__init__(f, fs, amp, offset, clip_h, clip_l)

        # init freq. dependent constants
        self._recalculate_private_variables()

        # init - inner states
        self.u = 1.0  # cos(w)
        self.v = 0.0  # sin(w)

    def set_freq(self, f):
        # set new frequency
        super().set_freq(f)
        self._recalculate_private_variables()

    def _recalculate_private_variables(self):
        """
         _recalculate_private_variables(self):

        This private method recalculates internal parameters
        when settings are changed.
        """
        # reinit freq. dependent constants
        self.w = 2.0 * np.pi * self.f / self.fs
        self.k1 = np.tan(0.5 * self.w)
        self.k2 = 2.0 * self.k1 / (1.0 + self.k1**2)

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


class SawtoothLowFrequencyOscillator(LowFrequencyOscillator):
    """class SawtoothLowFrequencyOscillator

    This class implements a sawtooth waveform generator.
    """

    def __init__(self, f, fs, balance=0.5, amp=1.0, offset=0.0, clip_h=1.0, clip_l=-1.0):
        """ Constructor
        The method acts as a constructor for
        the SawtoothLowFrequencyOscillator class.

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
        super().__init__(f, fs, amp, offset, clip_h, clip_l)
        self.balance = balance

        self.d_state = 'rising'  # init rise/fall state flag

        self._recalculate_private_variables()

        # init - inner states
        self.y = 0.0    # output sample
        self.d = self.d_rise  # init differential

    def _recalculate_private_variables(self):
        """
        _recalculate_private_variables(self):

        This private method recalculates internal parameters
        when settings are changed.
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

        # update with new differential
        if (self.d_state == 'rising'):
            self.d = self.d_rise  # init differential
        else:
            self.d = self.d_fall

    def set_freq(self, f):
        # set new frequency
        super().set_freq(f)
        self._recalculate_private_variables()

    def set_amp(self, amp):
        # set new amplitude
        super().set_amp(amp)
        self._recalculate_private_variables()

    def set_balance(self, balance):
        """
        set_balance(self, balance):

        This method sets the balance between rise and fall periods
        of the sawtooth wave generator.

        Parameters
        ----------
        balance : float
            ratio between rise time and fall time (or upwards and downwards slope)
            of the sawtooth waveform. 0.5 corresponds to equal rise and fall
        """
        # set new balance
        self.balance = max(min(balance, 0.99), 0.01)
        self._recalculate_private_variables()

    def reinit_phase(self):
        """
        reinit_phase(self):

        This method triggers re-initialization of the phase of the output sinusoid.
        """
        # reinit inner states
        self.y = 0.0    # output sample
        self.d = self.d_rise  # init differential
        self.d_state = 'rising'

    def generate(self):
        """
        generate(self):

        This method generates the next sample of the sawtooth waveform output.
        """
        # recursive modified sawtooth waveform generation
        if (self.y >= self.amp):
            self.d = self.d_fall
            self.d_state = 'falling'
        elif (self.y <= -self.amp):
            self.d = self.d_rise
            self.d_state = 'rising'

        # update new sample with differential
        self.y = self.y + self.d

        # offset and clip output
        return (max(min((self.y + self.offset), self.clip_h), self.clip_l))


if __name__ == '__main__':
    import matplotlib.pyplot as plt

    # %% ---  Step 1. - Setting up and running LFO objects ---
    # We are setting up both a Sinusoidal and a Sawtooth-type
    # LFO object with the same parameters. We are running them for 2 seconds.

    # sampling rate
    fs = 48000
    # duration of testrun in seconds
    sec_per_plot = 2

    # LFO parameters
    bpm = 110       # LFO frequency (BPM)
    rise_fall_balance = 0.9  # rise / fal balance (between 0 and 1) for sawtooth LFO
    amp = 1.3        # waveform amplitude
    offset = 0.4   # DC offset
    clip_h = 1.4   # clip amplitude - upper
    clip_l = -0.5   # clip amplitude - lower

    # prealloc output signal buffer
    leng = sec_per_plot * fs
    t_ax = np.r_[0: (leng-1)/fs: 1/fs]
    y_sin = np.zeros(leng)
    y_saw = np.zeros(leng)

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
        y_sin[i] = sinegen.generate()
        y_saw[i] = sawtoothgen.generate()

    # %% ---  Step 2. - Setting up and running waveform plotters ---
    # these are extra instances of the LFO's just with lower sampling rates
    # to match the desired plot length. In the max implementation, the wrapper
    # object will contain 2 instances of the ADSP object, and the set/get functions
    # should handle that together with the "actual" dsp object

    # OBS:
    # The sawtooth plotter often plots bullshit due to numeric errors in
    # calculating the slope of "undersampled" signals - this especially shows
    # up when then chosen plot length is short and the sawtooth balance is
    # well away from 50%. I have decided not to care, as this is "just a plotter"

    # for plotting waveform
    plot_size_in_samples = 256    # size of one block in samples
    # reduced sampling rate for plotters:
    fs_plotter = plot_size_in_samples / sec_per_plot

    # prealloc plot buffer
    pl_sin = np.zeros(plot_size_in_samples)
    pl_saw = np.zeros(plot_size_in_samples)

    sine_plotter = SinusoidalLowFrequencyOscillator(bpm/60,
                                                    fs_plotter,
                                                    amp,
                                                    offset,
                                                    clip_h,
                                                    clip_l)

    sawtooth_plotter = SawtoothLowFrequencyOscillator(bpm/60,
                                                      fs_plotter,
                                                      rise_fall_balance,
                                                      amp,
                                                      offset,
                                                      clip_h,
                                                      clip_l)

    for i in range(plot_size_in_samples):
        pl_sin[i] = sine_plotter.generate()
        pl_saw[i] = sawtooth_plotter.generate()

    # %% ---  Step 3. - Plotting LFO and Waveform plotter outputs ---

    f1, (ax1, ax2) = plt.subplots(2, 1)
    ax1.plot(y_sin)
    ax2.plot(pl_sin)

    f2, (ax1, ax2) = plt.subplots(2, 1)
    ax1.plot(y_saw)
    ax2.plot(pl_saw)

    # %% ---  Step 4. - Testing an LFO with changing parameters on the go ---

    # Don't run this block of code on its own several times, you'll see bullshit
    # values - this block depends on the LFO object's setup previously

    # prealloc output signal buffer
    leng = 10 * fs  # let's run it for 10 seconds
    t_ax = np.r_[0: (leng-1)/fs: 1/fs]
    y_long = np.zeros(leng)

    i = 0
    sawtoothgen.set_clip_h(4.5)
    sawtoothgen.set_clip_l(-4.5)

    # increase frequency and tilt balance backwards
    while(i < (0.3*leng)):
        y_long[i] = sawtoothgen.generate()
        sawtoothgen.set_freq(bpm/60)
        bpm += 0.002
        sawtoothgen.set_balance(rise_fall_balance)
        rise_fall_balance -= 1.0 / (0.3*leng)
        i += 1

    # decrease frequency
    while(i < (0.5*leng)):
        y_long[i] = sawtoothgen.generate()
        sawtoothgen.set_freq(bpm/60)
        bpm -= 0.0035
        i += 1

    # decrease DC
    while(i < (0.7*leng)):
        y_long[i] = sawtoothgen.generate()
        sawtoothgen.set_offset(offset)
        offset -= 0.000025
        i += 1

    # increase amplitude
    while(i < leng):
        y_long[i] = sawtoothgen.generate()
        sawtoothgen.set_amp(amp)
        amp += 0.000075
        i += 1

    f3, ax1 = plt.subplots(1, 1)
    ax1.plot(y_long)

    plt.show()
