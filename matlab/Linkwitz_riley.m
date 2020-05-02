clear all;
close all;
clc;

fs = 48000;
fc = 2500;
w0 = 2*pi*fc/fs;
wc = cos(w0);
ws = sin(w0);
% specifically for butterworth: wc/(2*Q) where Q == 1/sqrt(2)
alpha = ws / sqrt(2);

% butter LP coeffs
a0_lp =   1 + alpha;
a1_lp =  -2*wc;
a2_lp =   1 - alpha;
b0_lp = 0.5 * (1 - wc);
b1_lp =   1 - wc;
b2_lp = b0_lp;

% butter HP coeffs
a0_hp =   1 + alpha;
a1_hp =  -2*wc;
a2_hp =   1 - alpha;
b0_hp = -0.5 * (1 + wc);
b1_hp =   1 + wc;
b2_hp = b0_hp;

a1_lp = a1_lp / a0_lp;
a2_lp = a2_lp / a0_lp;
b0_lp = b0_lp / a0_lp;
b1_lp = b1_lp / a0_lp;
b2_lp = b2_lp / a0_lp;
a0_lp = 1;

a1_hp = a1_hp / a0_hp;
a2_hp = a2_hp / a0_hp;
b0_hp = b0_hp / a0_hp;
b1_hp = b1_hp / a0_hp;
b2_hp = b2_hp / a0_hp;
a0_hp = 1;

% creating signals
len = fs*20;
%input
x = 2*rand(1,len)-1;
% output of each biquad stage
y_lp1 = 2*zeros(1,len);
y_lp2 = 2*zeros(1,len);
y_hp1 = 2*zeros(1,len);
y_hp2 = 2*zeros(1,len);

% running white noise through the cascade
y_lp1 = filter([b0_lp, b1_lp, b2_lp],[a0_lp, a1_lp, a2_lp],x);
y_lp2 = filter([b0_lp, b1_lp, b2_lp],[a0_lp, a1_lp, a2_lp],y_lp1);
y_hp1 = filter([b0_hp, b1_hp, b2_hp],[a0_hp, a1_hp, a2_hp],x);
y_hp2 = filter([b0_hp, b1_hp, b2_hp],[a0_hp, a1_hp, a2_hp],y_hp1);

% plotting spectrum of input, L-R output, summed outputs
NFFT = 512;
f = linspace(0, fs/2 - (fs/2*(1/NFFT)), NFFT/2+1);
win = hanning(NFFT);

X = spectrogram(x,win,NFFT/8,NFFT);
Y_LP2 = spectrogram(y_lp2,win,NFFT/8,NFFT);
Y_HP2 = spectrogram(y_hp2,win,NFFT/8,NFFT);
SUMMED = spectrogram(y_lp2 + y_hp2,win,NFFT/8,NFFT);
semilogx(f, 20*log10(mean(abs(X),2)));
hold on;
semilogx(f, 20*log10(mean(abs(Y_LP2),2)));
semilogx(f, 20*log10(mean(abs(Y_HP2),2)));
semilogx(f, 20*log10(mean(abs(SUMMED),2)));
xlim([100 fs/4]);
ylim([0 max(20*log10(mean(abs(X),2))) * 1.2]);





