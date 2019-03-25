function out = sine_lfo(lfo_param) %todo: also determine the init phase

fs = lfo_param.fs;
f = lfo_param.f;
amp = lfo_param.amp;
offset = lfo_param.offset;
clip_h = lfo_param.clip_h;
clip_l = lfo_param.clip_l;

%init - freq. dependent constants
w = 2*pi*f / fs;
k1 = tan(0.5 * w);
k2 = 2 * k1 / (1 + k1^2);

% init persistent inner states
% cos(w)
persistent u;
if (isempty(u) || lfo_param.reinit_phase)
    u = 1;
end

% sin(w)
persistent v;
if (isempty(v) || lfo_param.reinit_phase)
    v = 0;
end

%recursive algorithm    
vv = u - k1 * v;
v = v + k2 * vv; % sin(w)
u = vv - k1 * v; % cos(w)

%offset and clip output
out = max(min((amp * v + offset), clip_h), clip_l);

end