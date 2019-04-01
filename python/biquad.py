'''
Created on 28 Mar 2019

@author: andraspalfi
'''


class BiquadFilter:

    def __init__(self, b0=1.0, b1=0.0, b2=0.0, a0=1.0, a1=0.0, a2=0.0):
        """__init__(self, b0, b1, b2, a0, a1, a2):

        This method acts as the constructor for the BiquadFilter class

        Parameters:
        -----------
        b0, b1, b2, a0, a1, a2: float
            biquad coefficients (b0-b2: feedforward, a0-a2: feedback)
            Default values are pass-thru filter coeffs (b0 and a0 = 1.0, all others 0.0)
        """
        # set coefficients
        self.set_coeffs(b0, b1, b2, a0, a1, a2)

        # internal delay registers
        self.w1 = 0.0
        self.w2 = 0.0

    def set_coeffs(self, b0, b1, b2, a0, a1, a2):
        """set_coeffs(self, b0, b1, b2, a0, a1, a2):

        This method sets the coefficients of the biquad filter.

        Parameters:
        -----------
        b0, b1, b2, a0, a1, a2: float
            biquad coefficients (b0-b2: feedforward, a0-a2: feedback)
        """
        # normalizing coeffs:
        ax = 1.0 / a0

        self.b0 = b0 * ax
        self.b1 = b1 * ax
        self.b2 = b2 * ax
        self.a0 = 1.0
        self.a1 = a1 * ax
        self.a2 = a2 * ax

    def process_d2(self, x):
        """process_d2(self, x):

        Performs biquad filtering for 1 sample of input signal
        Implementation: Direct Form II

        Parameters:
        -----------
        x: float
            input sample

        Returns:
        ________
        y: float
            output sample
        """
        # difference equation
        w = x - self.a1 * self.w1 - self.a2 * self.w2
        y = self.b0 * w + self.b1 * self.w1 + self.b2 * self.w2

        # delay registers
        self.w2 = self.w1
        self.w1 = w

        return y

    def process_td2(self, x):
        """process_td2(self, x):

        Performs biquad filtering for 1 sample of input signal
        Implementation: Transposed Direct Form II

        Parameters:
        -----------
        x: float
            input sample

        Returns:
        ________
        y: float
            output sample
        """
        # difference equation
        y = self.b0 * x + self.w1
        self.w1 = self.w2 + self.b1 * x - self.a1 * y
        self.w2 = self.b2 * x - self.a2 * y

        return y
