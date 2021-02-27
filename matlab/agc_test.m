clc;
clear all;
close all;

%an AGC algo test
%not used

%create a test signal
mu0=4.5;
mu1=0.7;
mu2=2.2;
sigma=0;
Fs=8000;
x=zeros(Fs*1,1);
sin_wave_org = sin(2*pi*100.1*[1:numel(x)]/Fs)';
sin_wave3 = mu1*sin_wave_org;
sin_wave3(1:end/4)=mu0*sin_wave_org(1:end/4);
sin_wave3(end/2:end)=mu2*sin_wave_org(end/2:end);
x=1*sin_wave3+1*sigma*randn(numel(x),1);

%test the agc algo
Dx=movmax(x.^2,128);
g=0;
K=0.01;
agc_level=1;
max_gain=100;
max_g=log(max_gain);
A=2*log(agc_level);
y=zeros(size(x));
x=delayseq(x,64);
for n=1:numel(x)
    y(n)=x(n)*exp(g);
    z=Dx(n)*exp(2*g);
    e=A-log(z);
    if(e>100)
        e=100;
    end
    if(e<-100)
        e=-100;
    end
    if(isnan(e))
        e=0;
    end
    g=g+K*e;
    if(g>max_g)
        g=max_g;
    end
end

%plot results
subplot(2,1,1);
plot(x);
title('No AGC');
subplot(2,1,2);
plot(y);
ylim([-2*agc_level 2*agc_level]);
title(['With AGC level=',num2str(agc_level)]);

%save input and output for c++
fileID = fopen('agc_input_signal.raw','w');
fwrite(fileID,x,'float32');
fclose(fileID);
fileID = fopen('agc_output_signal.raw','w');
fwrite(fileID,y,'float32');
fclose(fileID);

file_exists=false;
cd('../test_output');
if exist('agc_output_signal_from_cpp_include.m', 'file') == 2
     file_exists=true;
     agc_output_signal_from_cpp_include();
end
cd('../matlab');
if(file_exists)
    figure;
    plot(agc_output_signal_from_cpp);
    hold on;
    plot(delayseq(y,128));
    hold off;
    title('C++ vs Matlab output');
    legend('C++','Matlab delayed');
    figure;plot(agc_output_signal_from_cpp'-delayseq(y,128));title('Residule');
end




