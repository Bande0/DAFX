clear all
close all

% diameter of the horn installment
% distance between "closest to listener" and "furthest to listener"
% independent of listener distance to speaker 
diam = 5; 

% distance of listener to speaker
% the closer we are to the speaker, the higher the AM effect due to the 
% inverse square law
dist = 2; 

%minimum distance to speaker 
%(so that the inverse square law doesnt 'blow up)
% has to be larger than 1
% if this is 1, the ampltide modulation becomes maximal, with troughs at
% zero
mindist = 1.1;

% Amplitude modulation, based on distance
A = 1/((dist+mindist)^2);

w = 4 * 2*pi;   % rotational speed of the horn (in radians)
t = linspace(0,1,1000); % time axis

delay_L = 0.5*diam * sin(w*t) + 0.5*diam;
delay_R = -0.5*diam * sin(w*t) + 0.5*diam;
amp_L = -A*sin(w*t) + 1;
amp_R = A*sin(w*t) + 1;

% pitch shift caused by the delay
% negative sign: distance (delay) increases -> pitch decreases
pitch_L = -diff(delay_L);
pitch_R = -diff(delay_R);
% add an extra element to keep the length the same
pitch_L = [pitch_L(1), pitch_L];
pitch_R = [pitch_R(1), pitch_R];
% normalize them for plotting
pitch_L_norm = 0.5*max(delay_L)/max(pitch_L) * pitch_L;
pitch_R_norm = 0.5*max(delay_R)/max(pitch_R) * pitch_R;

figure();
subplot(321);
plot(t,delay_L);
title('delay L');
subplot(323);
plot(t, pitch_L);
title('pitch L');
subplot(325);
plot(t,amp_L);
title('amp L');

subplot(322);
plot(t,delay_R);
title('delay R');
subplot(324);
plot(t, pitch_R);
title('pitch R');
subplot(326);
plot(t,amp_R);
title('amp R');

