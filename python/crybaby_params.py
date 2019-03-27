'''
Created on 27 Mar 2019

@author: andraspalfi
'''
import numpy as np

k = 1000
n = 1e-9
u = 1e-6

# electric components
gf = -36   # forward gain of transistor stage
Cf = 10*n
Ci = 10*n
re = 155*k  # Resistance looking into BJT emitter
Rp = 33*k * re/(re + 33*k)
Lp = 0.5
Ri = 68*k
Rs = 1.5*k

# Analog boiler-plate parameters
RpRi = Rp*Ri/(Rp + Ri)
f0 = 1/(2*np.pi*np.sqrt(Lp*Cf))
Q = RpRi*np.sqrt(Cf/Lp)

# combined biquad for the feedback loop
Gi = Rs/(Ri + Rs)
gbpf = (1/(2*np.pi*f0*Ri*Cf))

crybaby_params = {
    'gf': gf,
    'Cf': Cf,
    'Ci': Ci,
    're': re,
    'Rp': Rp,
    'Lp': Lp,
    'Ri': Ri,
    'Rs': Rs,
    'RpRi': RpRi,
    'f0': f0,
    'Q': Q,
    'Gi': Gi,
    'gbpf': gbpf,
    'pedal_max': 0.99,
    'pedal_min': 0.01,
}
