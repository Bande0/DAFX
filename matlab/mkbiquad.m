function [SOS] = mkbiquad(param, fs)

% extract parameters
w0 = 2*pi*param.f0/fs;
Q=10^(param.Q_db/20);
gainLinear =10^(param.gain_db/20);
dBBoost = param.boost_db;
A  = 10^(dBBoost/40);

switch param.type
    case 'high_shelf'
        alpha = sin(w0)/2 * sqrt( (A + 1/A)*(1/Q - 1) + 2 );
        a0 =       (A+1) - (A-1)*cos(w0) + 2*sqrt(A)*alpha;
        a1 =   2*( (A-1) - (A+1)*cos(w0) );
        a2 =       (A+1) - (A-1)*cos(w0) - 2*sqrt(A)*alpha;
        b0 =   A*( (A+1) + (A-1)*cos(w0) + 2*sqrt(A)*alpha ) * gainLinear;
        b1 = -2*A*( (A-1) + (A+1)*cos(w0) ) * gainLinear;
        b2 =   A*( (A+1) + (A-1)*cos(w0) - 2*sqrt(A)*alpha ) * gainLinear;
    case 'low_shelf'
        alpha = sin(w0)/2 * sqrt( (A + 1/A)*(1/Q - 1) + 2 ); 
        a0 =       (A+1) + (A-1)*cos(w0) + 2*sqrt(A)*alpha;
        a1 =  -2*( (A-1) + (A+1)*cos(w0) );
        a2 =       (A+1) + (A-1)*cos(w0) - 2*sqrt(A)*alpha;
        b0 =   A*( (A+1) - (A-1)*cos(w0) + 2*sqrt(A)*alpha ) * gainLinear;
        b1 = 2*A*( (A-1) - (A+1)*cos(w0) ) * gainLinear;
        b2 =   A*( (A+1) - (A-1)*cos(w0) - 2*sqrt(A)*alpha ) * gainLinear;
    case 'band_pass'
        alpha = sin(w0) * sinh( log(2)/2) * Q * w0/sin(w0);
        a0 =   1 + alpha;
        a1 =  -2*cos(w0);
        a2 =   1 - alpha;
        b0 =   alpha * gainLinear;
        b1 =   0;
        b2 =  -alpha * gainLinear;
    case 'band_stop'
        alpha = sin(w0) * sinh( log(2)/2) * Q * w0/sin(w0);
        a0 =   1 + alpha;
        a1 =  -2*cos(w0);
        a2 =   1 - alpha;
        b0 =   1 * gainLinear;
        b1 =   -2*cos(w0) * gainLinear;
        b2 =   1 * gainLinear;
    case 'notch'
        fny=fs/2;
        fc=param.f0/(fny);
        amp=gainLinear;
        b=Q;
        c=(1-amp*(1-b));
        b0=(1-c+b);
        b1=(1-c+b)*(-2*c*cos(pi*fc));
        b2=(1-c+b)* c^2;
        a0=1;
        a1=-2*b*cos(pi*fc);
        a2= b^2;
    case 'peaking'
        alpha = sin(w0)/(2*Q);
        a0 =  1 + alpha/A;
        a1 = -2 * cos(w0);
        a2 =  1 - alpha/A;
        b0 = (1 + alpha*A) * gainLinear;
        b1 = -(2 * cos(w0)) * gainLinear;
        b2 = (1 - alpha*A) * gainLinear;
    case 'butter_lp'
        % butterworth LP
        alpha = sin(w0) / ( 2.0 * 1/sqrt(2));
        a0 =   1 + alpha;
        a1 =  -2*cos(w0);
        a2 =   1 - alpha;
        b0 =  (1 - cos(w0)) * gainLinear / 2;
        b1 =   1 - cos(w0)  * gainLinear;
        b2 =  (1 - cos(w0)) * gainLinear / 2;
    case 'butter_hp'
        % butterworth HP
        alpha = sin(w0) / ( 2.0 * 1/sqrt(2) );
        a0 =   1 + alpha;
        a1 =  -2*cos(w0);
        a2 =   1 - alpha;
        b0 = -(1 + cos(w0)) * gainLinear / 2;
        b1 =   1 + cos(w0)  * gainLinear;
        b2 = -(1 + cos(w0)) * gainLinear / 2;
   case 'pass_through'
       a0=1;b0=1;
       a1=0;a2=0;b1=0;b2=0;
    otherwise
        error 'Not a valid filter type'
end

% export biquad over a SOS form
SOS = [b0 b1 b2 a0 a1 a2];
end