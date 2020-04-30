'''
Created on 01 May 2020

@author: andraspalfi
'''


class FirstOrderIIR:
    """ class FirstOrderIIR

    This class implements a simple first order IIR filter object
    """

    def __init__(self, fs, tc_ms=None, fc=None, tau=None, alpha=None):
        """__init__(self, fs, tc_ms=None, fc=None, tau=None, alpha=None):

        This method acts as the constructor for the FirstOrderIIR class

        Response can either be specified by directly giving alpha, or by
        a time constant, tau, or fc. But they are mutually exclusive

        Parameters:
        -----------
        fs :
            sampling rate (mandatory)
        fc:
            cutoff frequency
        tc_ms:
            time constant in milliseconds
        tau:
            tau time constant
        alpha:
            alpha coefficient
        """
        self.fs = fs
        self.yn_1 = 0.0  # y[n-1]

        arglist = list([tc_ms, fc, tau, alpha])

        if sum(x is not None for x in arglist) != 1:
            raise ValueError('Parameters are mutually exclusive, only one is allowed')
        elif fc:
            self.alpha = 1.0 - np.exp(-fc/self.fs)
        elif tc_ms:
            fc = 1.0/(tc_ms*0.001)
            self.alpha = 1.0 - np.exp(-fc/self.fs)
        elif tau:
            fc = 1.0/tau
            self.alpha = 1.0 - np.exp(-fc/self.fs)
        elif alpha:
            self.alpha = alpha

    def set_fc(self, fc):
        self.alpha = 1.0 - np.exp(-fc/self.fs)

    def set_tc_ms(self, tc_ms):
        fc = 1.0/(tc_ms*0.001)
        self.alpha = 1.0 - np.exp(-fc/self.fs)

    def set_tau(self, tau):
        fc = 1.0/tau
        self.alpha = 1.0 - np.exp(-fc/self.fs)

    def set_alpha(self, alpha):
        self.alpha = alpha

    def process(self, x):
        """process(self, x):

        Performs 1st order IIR filtering on a singe sample of input

        Parameters:
        -----------
        x: float
            input sample

        Returns:
        ________
        y: float
            output sample
        """
        y = self.alpha * x + (1.0 - self.alpha) * self.yn_1
        self.yn_1 = y

        return y


if __name__ == '__main__':
    import numpy as np
    # from dafx_audio_io import read_audio, write_audio
    import matplotlib.pyplot as plt

    # %% -- Demonstrating 1st order IIR object by step response

    tc_ms = 100  # filter time constant in millisec
    fs = 48000  # sampling rate

    # input signal 1 - step response
    ms_at_step = 330   # the time instant where the step function should kick in
    val1 = 0.0   # value before the step onset
    val2 = 1.0   # value after the step onset

    # the time instant where the marker should be drawn
    marker_ms = 1*tc_ms + ms_at_step

    # prealloc arrays for time axis and input-output
    t = np.r_[0: 2: 1/fs]  # time axis
    x = np.zeros_like(t)  # clean step signal
    y = np.zeros_like(t)  # 1st order IIR output
    x2 = np.zeros_like(t)  # input signal 2
    y2 = np.zeros_like(t)  # 1st order IIR output

    step_idx = int(ms_at_step*0.001*fs)  # time axis index where the step kicks in
    tc_idx = int(marker_ms*0.001*fs)  # time axis index where the response should have reached 63%

    # step response setup
    x[0: step_idx] = val1
    x[step_idx:] = val2
    y[0] = x[0]  # initial value of response

    # input signal 2
    x2[0: int(200*0.001*fs)] = 1.0
    x2[int(200*0.001*fs): int(600*0.001*fs)] = 2.0
    x2[int(600*0.001*fs): int(1200*0.001*fs)] = 4.0
    x2[int(1200*0.001*fs): int(1600*0.001*fs)] = -2.0
    x2[int(1600*0.001*fs): int(2000*0.001*fs)] = -1.0

    # 1st order IIR
    iir = FirstOrderIIR(fs, tc_ms=tc_ms)
    iir2 = FirstOrderIIR(fs, tau=0.06)

    for n in range(0, len(y)):
        y[n] = iir.process(x[n])
        y2[n] = iir2.process(x2[n])

    # value of the response when the time constant has elapsed
    z = y[tc_idx] * np.ones_like(t)
    print('value at marker: {} ({:.2f} %)'.format(y[tc_idx], (y[tc_idx] - y[0]) / (y[-1] - y[0])))

    plt.figure()
    plt.plot(t, x)
    plt.plot(t, y)
    plt.axvline(tc_idx/fs, linewidth=1, color='r', linestyle='--')
    plt.plot(t, z, 'r--', linewidth=1)

    plt.figure()
    plt.plot(t, x2)
    plt.plot(t, y2)
    plt.show()

    plt.show()
