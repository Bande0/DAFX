function out = sawtooth_lfo_plotter(lfo_param) %todo: also determine the init phase

fs = lfo_param.fs;
f = lfo_param.f;
rise_fall_balance = lfo_param.rise_fall_balance;
amp = lfo_param.amp;
offset = lfo_param.offset;
clip_h = lfo_param.clip_h;
clip_l = lfo_param.clip_l;

% init output sample
persistent y;
if isempty(y)
    y = 0; %TODO: init to a certain phase offset!
end

% duration of a full period in samples (rounded)
T = round(fs / f); 
% duration of rise and fall periods in samples
t1 = T * rise_fall_balance;
t2 = T - t1;
% rise and fall differentials during one sample
d_rise = (2*amp) / t1;
d_fall = -(2*amp) / t2;

% init differential step
persistent d;
if isempty(d)
    % init to "rising" 
    d = d_rise;
end

%recursive modified sawtooth waveform generation    
if (y >= amp)
   d = d_fall; 
elseif (y <= -amp)
   d = d_rise; 
end

%update new sample with differential
y = y + d;

out = max(min((y + offset), clip_h), clip_l);

end