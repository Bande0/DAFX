k=1000;
n=1e-9;
u=1e-6;

f=100:1:10000;

%Crybaby circuit components
gf = -36  %forward gain of transistor stage
gp =  0   %gain of potentiometer

Cf = 10*n
Ci = 10*n
re = 155*k %Resistance looking into BJT emitter
Rp = 33*k *re/(re + 33*k)
Lp = 0.5
Ri = 68*k
Rs = 1.5*k

% Analog boiler-plate parameters
RpRi = Rp*Ri/(Rp + Ri);
f0 = 1/(2*pi*sqrt(Lp*Cf))
Q = RpRi*sqrt(Cf/Lp)

% DSP parameters    
    fs = 48000
    w=2*pi.*f./fs;
    z1=exp(-j.*w);
    z2=exp(-2*j.*w);

    %Useful variables
    w0 = 2.0*pi*f0/fs;
    c = cos(w0);
    s = sin(w0);
    alpha = s/(2.*Q);
    
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
    
    %combined biquad for the feedback loop
    Gi = Rs/(Ri + Rs);
    gbpf = (1/(2*pi*f0*Ri*Cf))
    
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
        a0 = a0b + gp*a0c
        a1 = a1b + gp*a1c
        a2 = a2b + gp*a2c
        ax = 1/a0;
    
    %1-Pole HPF coeffs
    a1p = exp(-1/(Ri*Ci*fs));
    g = gf*(1.0 + a1p)*0.5;  %put the forward gain into the HPF to avoid repeated multiplications in real-time

    %Transfer functions
    %1-Pole HPF
    Hin = g * (1 - z1)./(1 - a1p.*z1);
 
for gx = 0:0.1:1
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
    a1 = a1b + gp*a1c;
    a2 = a2b + gp*a2c;
    ax = 1/a0;
    
    %final transfer function
    % Hz = Hin.*gf.*( b2*z2 + b1*z1 + b0 )./( a2*z2 + a1*z1 + a0 );
    % semilogx(f, 20*log10(abs(Hz)), "r" )  

    %Refactored to Direct I form digital biquad filter
    Hz = Hin.*ax.*( b2*z2 + b1*z1 + b0 )./( a2*ax*z2 + a1*ax*z1 + 1 );
    semilogx(f, 20*log10(abs(Hz)), 'r' )       
    %plot(f,abs(Hz))
    hold on
end

hold off

