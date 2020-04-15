close all
clear all
clc

x = linspace(-1.0,1.0,100);
out_lim = 0.75;

%% Hyperbolic tangent 
figure();
title('Hyperbolic Tangent')
legendcellarray = {} 
hold on;
A = 1;

for i = 1:20
    y_tan = out_lim * tanh(A*x);  
    plot(x,y_tan);
    legendcellarray{i} = strcat('A = ',num2str(A)); 
    A = A+1;
end
legend(legendcellarray);

%% Sinusoidal
figure();
title('Sinusoidal')
legendcellarray = {} 
hold on;

thresh = 0.9;
for i = 1:9    
    satur_point = 0.5 * (1/thresh); % this is so that the argument in the sin gives 0.5pi, i.e. sin(90deg) == 1 
    y_sin(abs(x) > thresh) = out_lim * sign(x(abs(x) > thresh));  % this time X only takes positive values, but you get it 
    y_sin(abs(x) <= thresh) = out_lim * sin(satur_point*pi*x(abs(x) <= thresh));
    plot(x,y_sin);
    legendcellarray{i} = strcat('thresh = ',num2str(thresh)); 
    thresh = thresh - 0.1;
end
legend(legendcellarray);

%% Exponential
figure();
title('Exponential')
legendcellarray = {} 
cnt = 1;
hold on;

thresholds = [0.2, 0.5, 0.9];

for k = 1:3 % loop over threshold
    thresh = thresholds(k);
    inv_thresh = 1/thresh;
    for E = 3:5  %loop over Exponent  

        for i = 1:length(x)
           if (abs(x(i)) > thresh)
               y_exp(i) = out_lim * sign(x(i));
           else
               y_exp(i) = out_lim * sign(x(i)) * (1 - abs(inv_thresh*x(i) - sign(x(i)))^E);
           end
        end
        
        plot(x,y_exp);
        legendcellarray{cnt} = strcat('thresh = ',num2str(thresh), ', E = ', num2str(E)); 
        cnt = cnt+1;
    end    
end

legend(legendcellarray);


