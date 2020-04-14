fs = 44100;

param.f0 = 1000;
param.Q_db = 0;
param.gain_db = 0;
param.boost_db = 0;
param.type = 'butter_lp';

format long
coeffs = mkbiquad(param, fs)

