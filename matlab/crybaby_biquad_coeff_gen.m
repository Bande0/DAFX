function [b0, b1, b2, a0, a1, a2] = crybaby_biquad_coeff_gen(knob_pos, crybaby_param)

    %% init part    
    fs = crybaby_param.fs;        
    gf = crybaby_param.gf;
    Cf = crybaby_param.Cf;
    Ci = crybaby_param.Ci;
    re = crybaby_param.re; 
    Rp = crybaby_param.Rp;
    Lp = crybaby_param.Lp;
    Ri = crybaby_param.Ri;
    Rs = crybaby_param.Rs;
    RpRi = crybaby_param.RpRi;
    f0 = crybaby_param.f0;
    Q = crybaby_param.Q;
    Gi = crybaby_param.Gi;
    gbpf = crybaby_param.gbpf;    
    
    w0 = crybaby_param.w0;
    c = crybaby_param.c;
    s = crybaby_param.s;
    alpha = crybaby_param.alpha;
    
    %init knob value
    gp = 0; 
    
    %High Pass Filter coeffs
    b0h =  (1.0 + c)/2.0;
    b1h =   -(1.0 + c);
    b2h =  (1.0 + c)/2.0;
    a0h =   1.0 + alpha;
    a1h =  -2.0*c;
    a2h =   1.0 - alpha;
    
    %Band Pass Filter coeffs
    b0b =   Q*alpha;
    %b1b =   0.0;
    b2b =  -Q*alpha;
    a0b =   1.0 + alpha;
    a1b =  -2.0*c;
    a2b =   1.0 - alpha;
    
    %Numerator coefficients
    b0 = gbpf*b0b + Gi*a0b;
    b1 =            Gi*a1b;
    b2 = gbpf*b2b + Gi*a2b;

    %Constants to make denominator coefficients computation more efficient
    %in real-time
    a0c = -gf*b0h;
    a1c = -gf*b1h;
    a2c = -gf*b2h;
    
    %denominator coefficients (initialization)
    a0 = a0b + gp*a0c;
    ax = 1/a0;
    a1 = (a1b + gp*a1c) * ax;
    a2 = (a2b + gp*a2c) * ax;
    a0 = 1.0;    
    
%     % TODO: 1-Pole HPF coeffs
%     % make this part of the signal chain either as a different module or 
%     % by including in the [b, a] calculations
%     
%         a1p = exp(-1/(Ri*Ci*fs));
%         g = gf*(1.0 + a1p)*0.5;  %put the forward gain into the HPF to avoid repeated multiplications in real-time
%       
%         %Transfer functions
%         %1-Pole HPF
%         Hin = g * (1 - z1)./(1 - a1p.*z1);
    
    %% Algo
    
    %potentiometer position
    gx = knob_pos;
    
    %Ever so slight gain adjustment to match analog curves.  
    %This is not necessary in a real implementation since it represents
    %a fraction of a degree tilt on the treadle (6 degrees pot rotation)
    aa= -0.15;  
    gp = gx*(1+aa);
    %denominator coefficients need to be computed for every change in the pot position
    %This costs 3 multiply and add plus a single 1/x operation for computation of coefficients
    %Then add 2 more multiplications for denominator and one for numerator.
    %Sum is 6 multiply and one 1/x operation per sample.  It's approximately the same
    %as running 2 fixed biquad filters, so not too bad when all is refactored
    a0 = a0b + gp*a0c;
    ax = 1/a0;
    
    a0 = 1.0;   
    a1 = (a1b + gp*a1c) * ax;
    a2 = (a2b + gp*a2c) * ax;       
     
end