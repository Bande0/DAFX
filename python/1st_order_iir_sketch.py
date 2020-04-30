# -*- coding: utf-8 -*-
"""
Created on Fri May  1 00:01:39 2020

@author: anlf
"""
import numpy as np
import matplotlib.pyplot as plt

tc_ms = 130
fs = 48000
ms_at_step = 200   # the time instant where the step function should kick in
val1 = 1.0   # value before the step onset
val2 = 2.0   # value after the step onset
marker_ms = 1*tc_ms + ms_at_step # the time instant where the marker should be drawn

# filter parameters
fc = 1/(tc_ms*0.001)
tau = tc_ms*0.001
alpha = 1.0 - np.exp(-fc/fs)

t = np.r_[0: 2: 1/fs] # time axis
x = np.zeros_like(t) # clean step signal
y = np.zeros_like(t) # 1st order IIR output

step_idx = int(ms_at_step*0.001*fs) # time axis index where the step kicks in
tc_idx = int(marker_ms*0.001*fs) # time axis index where the response should have reached 63%

# step response setup
x[0 :step_idx] = val1
x[step_idx :] = val2
y[0] = x[0]  # initial value of response

#1st order IIR
for n in range(1, len(y)):
    y[n] = alpha * x[n] + (1.0 - alpha) * y[n-1]

# value of the response when the time constant has elapsed
z = y[tc_idx] * np.ones_like(t)
print('value at marker: {} ({:.2f} %)'.format(y[tc_idx], (y[tc_idx] - y[0]) / (y[-1] - y[0])))


plt.plot(t,x)
plt.plot(t,y)
plt.axvline(tc_idx/fs, linewidth=1, color='r', linestyle='--')
plt.plot(t,z, 'r--', linewidth=1,)
plt.show()


