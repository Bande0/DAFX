% wah-wah effect
clear all;
close all;

infile_name = 'clean_funky_lick_90bpm';
outfile_name = [infile_name, '_crybaby'];

% read in audio file
infile_path = ['../../input_audio/',infile_name,'.wav'];
[ x, fs ] = audioread(infile_path);

%time axis
t = 0 : 1/fs : (length(x)-1)/fs; 
%prealloc signal buffers
lfo_out = zeros(1,length(t));
pedal_pos = zeros(1,length(t));
bp_out = zeros(1,length(t));
crybaby_out = zeros(1,length(t));

%% user defined params
% LFO
lfo_freq_bpm = 90;        % LFO frequency (BPM)
rise_fall_balance = 0.8;  % rise / fall balance of sawtooth (between 0 and 1)
amp = 1.3;                % waveform amplitude 
offset = 0;               % DC offset
clip_h = 1.4;             % clip amplitude - upper
clip_l = -0.5;            % clip amplitude - lower
waveform = 'sawtooth';    % 'sine' or 'sawtooth'

% at this point in time, the LFO phase is reinitialized
lfo_reinit_phase_t = 3;

% Crybaby
wah_balance = 0.75;        % balance of direct and wah-ed signal

%% Setting up module parameters
%Crybaby circuit components
%multiplier constants
k=1000;
n=1e-9;
u=1e-6;
% electric components
gf = -36;  %forward gain of transistor stage
Cf = 10*n;
Ci = 10*n;
re = 155*k; %Resistance looking into BJT emitter
Rp = 33*k *re/(re + 33*k);
Lp = 0.5;
Ri = 68*k;
Rs = 1.5*k;
% Analog boiler-plate parameters
RpRi = Rp*Ri/(Rp + Ri);
f0 = 1/(2*pi*sqrt(Lp*Cf));
Q = RpRi*sqrt(Cf/Lp);
%combined biquad for the feedback loop
Gi = Rs/(Ri + Rs);
gbpf = (1/(2*pi*f0*Ri*Cf));
%useful params
w0 = 2.0*pi*f0/fs;
c = cos(w0);
s = sin(w0);
alpha = s/(2.*Q);
% hard clipping of pedal position - MUST be between 0 and 1
pedal_max = 0.99;
pedal_min = 0.01;

% setting crybaby params
crybaby_param.fs = fs;
crybaby_param.gf = gf;
crybaby_param.Cf = Cf;
crybaby_param.Ci = Ci;
crybaby_param.re = re; 
crybaby_param.Rp = Rp;
crybaby_param.Lp = Lp;
crybaby_param.Ri = Ri;
crybaby_param.Rs = Rs;
crybaby_param.RpRi = RpRi;
crybaby_param.f0 = f0;
crybaby_param.Q = Q;
crybaby_param.Gi = Gi;
crybaby_param.gbpf = gbpf;
crybaby_param.w0 = w0;
crybaby_param.c = c;
crybaby_param.s = s;
crybaby_param.alpha = alpha;

% lfo params
lfo_param.fs = fs;
lfo_param.f = lfo_freq_bpm / 60;   % LFO frequency (Hz);
lfo_param.rise_fall_balance = rise_fall_balance;
lfo_param.amp = amp;
lfo_param.offset = offset;
lfo_param.clip_h = clip_h;
lfo_param.clip_l = clip_l;

lfo_reinit_phase_idx = round(lfo_reinit_phase_t * fs);
%% ALGO

% main loop
for n = 1:length(x) 
    
    % trigger a phase reinit in the LFO
    if (n == lfo_reinit_phase_idx)
        lfo_param.reinit_phase = 1;
    else
        lfo_param.reinit_phase = 0;
    end;
    
    % LFO
    if (strcmp(waveform,'sine') == 1)
        lfo_out(n) = sine_lfo(lfo_param);
    elseif (strcmp(waveform,'sawtooth') == 1)
        lfo_out(n) = sawtooth_lfo(lfo_param);
    end
    
    % clipping the LFO output between 0 and 1 - otherwise crybaby algo blows up!
    pedal_pos(n) = max(min(lfo_out(n), pedal_max), pedal_min);
    
    % invert the LFO output for pedal pos function:
    % zero corresponds to highest pedal position and 1 corresponds to lowest
    pedal_pos(n) = 1.0 - pedal_pos(n);
    
    % CALCULATE CRYBABY COEFFS
    [b0, b1, b2, a0, a1, a2] = crybaby_biquad_coeff_gen(pedal_pos(n), crybaby_param);
    
    % 2ND ORDER IIR
    %only for the first 2 samples, so we don't index into negative x
    if (n == 1)
        bp_out(n) = b0 * x(n);
    elseif (n == 2)
        bp_out(n) = b0 * x(n) + b1 * x(n-1) - a1 * bp_out(n-1); 
    else
        % direct form
        bp_out(n) = b0 * x(n) + b1 * x(n-1) + b2 * x(n-2) - a1 * bp_out(n-1) - a2 * bp_out(n-2);
    end
    
    % TODO: 1ST ORDER HP FILTER HERE!!!
    
    % mixing the filtered signal with the direct signal for final output
    crybaby_out(n) = wah_balance * bp_out(n) + (1-wah_balance) * x(n);
   
end

%plot(pedal_pos);

%% write output
outfile_path = ['../../output_audio/crybaby/',outfile_name,'.wav'];

% write output wav file
audiowrite(outfile_path, crybaby_out, fs);
