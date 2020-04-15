'''
Created on 15 Apr 2020

@author: andraspalfi
'''
import numpy as np


class Overdrive:
    """ class Overdrive

    This class implements an overdrive (soft clipping) sound effect.

    The soft clipping is implemented by three different algorithms:
        Hyperbolic tangent, sinusoidal and exponential

    The processing is sample-based
    """

    def __init__(self, algo='tanh', out_lim=0.75, **kwargs):
        """ __init__(self, algo='tanh', **kwargs):

        This method is the Constructor of the Overdrive class

        Parameters:
        ___________
        algo : string
            Specifies soft clipping algorithm.
            Available options are 'tanh', 'sin' or 'exp'
        out_lim : float
            Gain limit (upper bound) of the output signal.
            Default: 0.75

        Other Parameters:
        _________________
        param : float
            Adjustable numerical parameter, with varying interpretation based
            on the chosen soft clipping method.
            - For the 'tanh' algorithm, it is a pre-gain on the input sample.
            Default: 5
            - For the 'exp' algorithm, it is the exponent in the output  expression
            Default: 2
            - For the 'sin' algorithm it is unused and can be omitted
        thresh : float
            Adjustable clipping threshold. Only used for 'exp' and 'sin' types.
            Default: 0.66


        Raises:
        _______

        TypeError:
            If the algo parameter has an invalid value
        ValueError:
            If a numerical parameter is out of the allowed bounds

        """
        # default values
        thresh_def = 0.66
        tan_def = 5.0
        exp_def = 2.0

        # bounds
        self.thresh_min = 0.01
        self.thresh_max = 1.0
        self.tan_min = 3.0
        self.tan_max = 100.0
        self.exp_min = 2.0
        self.exp_max = 10.0
        self.out_lim_min = 0.5
        self.out_lim_max = 0.99

        self.algo = algo

        if out_lim <= self.out_lim_max and out_lim >= self.out_lim_min:
            self.out_lim = out_lim
        else:
            raise ValueError('Specified limit out of bounds!')

        if algo == 'tanh':
            self.tan_param = kwargs.get('param', tan_def)
            # these two are not used in this algorithm, but are set in case the
            # algo is suddenly changed on the fly
            self.thresh = thresh_def
            self.inv_thresh = 1.0/self.thresh
            self.exp_param = exp_def
        elif algo == 'sin':
            self.thresh = kwargs.get('thresh', thresh_def)
            self.inv_thresh = 1.0/self.thresh
            # these two are not used in this algorithm, but are set in case the
            # algo is suddenly changed on the fly
            self.tan_param = tan_def
            self.exp_param = exp_def
        elif algo == 'exp':
            self.thresh = kwargs.get('thresh', thresh_def)
            self.inv_thresh = 1.0/self.thresh
            self.exp_param = kwargs.get('param', exp_def)
            # this one is not used in this algorithm, but is set in case the
            # algo is suddenly changed on the fly
            self.tan_param = tan_def
        else:
            raise TypeError('Invalid overdrive type chosen!')

    def set_algo(self, algo):
        """set_algo(self, algo):

        This method changes the used soft-clipping algorithm on the fly.


        Parameters:
        ___________
        algo : string
            Specifies soft clipping algorithm.
            Available options are 'tanh', 'sin' or 'exp'

        Raises:
        _______

        TypeError:
            If the algo parameter has an invalid value

        """
        if algo == 'tanh' or algo == 'sin' or algo == 'exp':
            self.algo = algo
        else:
            raise TypeError('Invalid overdrive type chosen!')

    def set_param(self, param):
        """set_param(self, param):

        This method sets the numerical parameter of the soft-clipping algorithm.
        The interpretation of the parameter is dependent on the algorithm used.


        Parameters:
        ___________
        param : float
            Adjustable numerical parameter, with varying interpretation based
            on the chosen soft clipping method.
            - For the 'tanh' algorithm, it is a pre-gain on the input sample.
            Default: 5
            - For the 'exp' algorithm, it is the exponent in the output  expression
            Default: 2
            - For the 'sin' algorithm it is unused and can be omitted

        Raises:
        _______

        ValueError:
            If the parameter is out of the allowed bounds

        """
        if self.algo == 'tanh':
            if param > self.tan_max or param < self.tan_min:
                raise ValueError('Parameter out of bounds!')
            else:
                self.tan_param = param
        elif self.algo == 'exp':
            if param > self.exp_max or param < self.exp_min:
                raise ValueError('Parameter out of bounds!')
            else:
                self.exp_param = param
        else:
            pass

    def set_thresh(self, thresh):
        """set_param(self, thresh):

        This method sets the clipping threshold of the soft-clipping algorithm


        Parameters:
        ___________
        thresh : float
            Adjustable clipping threshold. Only used for 'exp' and 'sin' types.
            Default: 0.66

        Raises:
        _______

        ValueError:
            If the threshold is out of the allowed bounds

        """
        if self.algo == 'sin' or self.algo == 'exp':
            if thresh > self.thresh_max or thresh < self.thresh_min:
                raise ValueError('Threshold out of bounds!')
            else:
                self.thresh = thresh
                self.inv_thresh = 1.0/self.thresh
        else:
            pass

    def set_limit(self, out_lim):
        """set_limit(self, out_lim):

        This method sets the gain limit (upper bound) of the output signal


        Parameters:
        ___________
        out_lim : float
            Gain limit (upper bound) of the output signal.

        Raises:
        _______

        ValueError:
            If the limit is out of the allowed bounds

        """
        if out_lim <= self.out_lim_max and out_lim >= self.out_lim_min:
            self.out_lim = out_lim
        else:
            raise ValueError('Specified limit out of bounds!')

    def process(self, x):
        """process(self, x):

        Performs overdrive by applying a soft-clipping algorithm

        Parameters:
        -----------
        x: float
            input sample

        Returns:
        ________
        y: float
            processed output sample
        """
        if self.algo == 'tanh':
            y = self.out_lim * np.tanh(self.tan_param * x)
        elif self.algo == 'sin':
            # This is the saturation point - together with the set threshold, the
            # argument of the sin will be 0.5*pi, where the signal is saturated
            saturation = 0.5 * self.inv_thresh

            if (np.abs(x) > self.thresh):
                y = self.out_lim * np.sign(x)
            else:
                y = self.out_lim * np.sin(saturation * np.pi * x)
        elif self.algo == 'exp':
            if (np.abs(x) > self.thresh):
                y = self.out_lim * np.sign(x)
            else:
                y = self.out_lim * np.sign(x) * (1.0 - np.abs(self.inv_thresh * x - np.sign(x))**self.exp_param)
        else:
            y = x
        return y


if __name__ == '__main__':
    import matplotlib.pyplot as plt
    from dafx_audio_io import read_audio, write_audio

    # %% -- Demonstrating soft clipping algorithms

    # input signal -  read in wav file
    infile_name = 'clean_pop_strum_100bpm'
    outfile_name = infile_name + '_overdrive'

    infile_path = '../../input_audio/' + infile_name + '.wav'
    (fs, x) = read_audio(infile_path)
    leng_samp = x.__len__()
    t_ax = np.r_[0: (leng_samp) / fs: 1 / fs]
    y = np.zeros(x.__len__())

    # create overdrive object
    od = Overdrive(algo='exp', thresh=0.3, param=3)

    # processing the overdrive output
    for (i, _) in enumerate(x):
        y[i] = od.process(x[i])

    f1, (ax, ax2) = plt.subplots(2, 1)
    ax.plot(x)
    ax2.plot(y)
    plt.show()

    outfile_path = '../../output_audio/overdrive/' + outfile_name + '.wav'
    write_audio(y, fs, outfile_path)

    # %% Plotting the gain curve
    gain_plot_size = 256
    plot_x = np.r_[0: 1: 1/gain_plot_size]
    plot_y = np.zeros(gain_plot_size)

    # creating the plot
    for (i, _) in enumerate(plot_x):
        plot_y[i] = od.process(plot_x[i])

    f2, ax1 = plt.subplots(1, 1)
    ax1.plot(plot_y)
    plt.show()
