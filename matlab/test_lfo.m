clear all;
close all;

% LFO parameters
fs = 48000;     % sampling freq
bpm = 60;       % LFO frequency (BPM)
rise_fall_balance = 0.8;  % rise / fal balance (between 0 and 1)
amp = 1.3;        % waveform amplitude 
offset = 0.4;   % DC offset
clip_h = 1.4;   % clip amplitude - upper
clip_l = -0.5;   % clip amplitude - lower
waveform = 'sawtooth';  % 'sine' of 'sawtooth'

% for plotting waveform
block_size = 128;    % size of one block in samples
sec_per_block = 3;   % no. of seconds one sample block corresponds to

%prealloc output signal buffer
len = 3 * fs;
t_ax = 0 : 1/fs : (len-1)/fs;
y = zeros(1,length(t_ax));

lfo_param.fs = fs;
lfo_param.f = bpm / 60;   % LFO frequency (Hz);
lfo_param.rise_fall_balance = rise_fall_balance;
lfo_param.amp = amp;
lfo_param.offset = offset;
lfo_param.clip_h = clip_h;
lfo_param.clip_l = clip_l;

% algo
for n = 1:length(t_ax) 
    if (strcmp(waveform,'sine') == 1)
        y(n) = sine_lfo(lfo_param);
    elseif (strcmp(waveform,'sawtooth') == 1)
        y(n) = sawtooth_lfo(lfo_param);
    end
end

figure();
subplot(211)
plot(y);
ylim([-3, 3]);
xlim([0, length(t_ax)]);

%% plotting
%same for plotting the waveform shape
block_fs = (block_size / sec_per_block);
lfo_plot_param.fs = block_fs; % here is the difference
lfo_plot_param.f = bpm / 60;   % LFO frequency (Hz);
lfo_plot_param.rise_fall_balance = rise_fall_balance;
lfo_plot_param.amp = amp;
lfo_plot_param.offset = offset;
lfo_plot_param.clip_h = clip_h;
lfo_plot_param.clip_l = clip_l;

%prealloc plot buffer
t_plot = 0 : 1/block_fs : (block_size-1)/block_fs;
y_plot = zeros(1,length(t_plot));

% algo
for n = 1:length(t_plot) 
    % this is just another instance of the sawtooth_lfo object
    % reason why this has to be in a different function is 
    % because this would screw up the inner state of the other
    if (strcmp(waveform,'sine') == 1)
        y_plot(n) = sine_lfo_plotter(lfo_plot_param);
    elseif (strcmp(waveform,'sawtooth') == 1)
        y_plot(n) = sawtooth_lfo_plotter(lfo_plot_param);
    end
end

subplot(212)
plot(y_plot);
ylim([-3, 3]);
xlim([0, length(t_plot)]);


