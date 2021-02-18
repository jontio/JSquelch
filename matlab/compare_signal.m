clc;
clear all;
close all;

%load test signal that was created by the create function
fileID = fopen('test_signal_time.raw','r');
sig_test=fread(fileID,'float32');
fclose(fileID);
Fs=8000;

spectrogram(sig_test,512,250,[],Fs,'yaxis');
title('test signal');

%FFTs
block_size=256;%-->32ms blocks
XBs=zeros(256+1,floor(numel(sig_test)/(block_size/2))+1);
m=0;
for k=1:128:numel(sig_test)-256%ffts are 50% overlapped
    xb=sig_test(k:k+256-1).*hann(256);
    xb=[xb;zeros(numel(xb),1)];
    m=m+1;
    XB=fft(xb);
    XBs(:,m)=XB(1:numel(XB)/2+1);
end
XBs_org=XBs;

%double moving windows
short_moving_window_size=16;
long_moving_window_size=16;
Zstd=movstd(abs(XBs'),short_moving_window_size);
Zmean=movmean(abs(XBs'),short_moving_window_size);
Zmin_std=Zstd;
Zmin_mean=Zmean;
for k=1:size(Zmean,1)
    for n=1:size(Zmean,2)
        [~,idx]=min(Zmean(k:min(k+long_moving_window_size-1,size(Zmean,1)),n));
        Zmin_mean(k,n)=Zmean(idx+k-1,n);
        Zmin_std(k,n)=Zstd(idx+k-1,n);
    end
end

%calculate normalizing factor over a window
%the bigger this window is the less agile the noise estimateion will be
moving_sigma_estimate=1.000*(sqrt((Zmin_std).^2+(Zmin_mean).^2));
moving_sigma_estimate=movmean(moving_sigma_estimate,62);

%normalize std assuming no constant signals. this
%produces a flat noise floor of about 1
x=[];
xb_last=zeros(256,1);
for k=1:size(XBs,2)
    XB=XBs(:,k);
    XB=XB./moving_sigma_estimate(k,:)';
    XBs(:,k)=XB;
    XB=[XB;conj(flip(XB(2:end-1)))];
    xb=real(ifft(XB));
    xb=xb(1:numel(xb)/2);
    xbc=0.5*(xb(1:128)+xb_last(129:256));
    x=[x;xbc];
    xb_last=xb;
end

figure
spectrogram(x,512,250,[],Fs,'yaxis');
title('normalized signal');

%produce lin_snrs a moving snr estimate of the speech
%snr_estimate is just for plotting and signal decision
ZstdFast=movstd(abs(XBs'),8);%needs to be small
ZmeanFast=movmean(abs(XBs'),8);%needs to be small
mu_est=((ZmeanFast.^2+ZstdFast.^2-1))/1;
snr_estimate=[];
lin_snrs=XBs;
for k=1:size(ZmeanFast,1)
    mu=mu_est(k,:);%max((ZmeanFast(k,:).^2+ZstdFast(k,:).^2-1),0)/1;%mu estimate
    lin_snrs(:,k)=mu';
    snr_estimate(end+1)=max(mu(4:96));%only bother about frequencies where most voice energy is
end
lin_snrs=movmean(lin_snrs',8);%reduces white noise but makes it sound echoy and strange
snr_estimate=movmean(snr_estimate,62);%1 sec smoothing
snr_estimate_db=10*log10(snr_estimate);

%plotting expected max snr estimate
figure;
plot(linspace(0,numel(snr_estimate)*128/Fs,numel(snr_estimate)),snr_estimate_db);
ylabel('SNR (dB)');
xlabel('time (s)');
title('Voice signal to noise estimate');

%plotting actual max snr estimate
actual_snr_estimate_db_signal_include();
hold on;
actual_snr_estimate_db_signal=actual_snr_estimate_db_signal(86:end);%there is an offset
plot(linspace(0,numel(actual_snr_estimate_db_signal)*128/Fs,numel(actual_snr_estimate_db_signal)),actual_snr_estimate_db_signal);
hold off;

legend('expected','actual');

%reconstitute signal. scale bins by their snr estimate
min_snr_required_in_dB=5;
x=[];
xb_last=zeros(256,1);
for k=1:size(XBs,2)
    if(snr_estimate_db(k)<min_snr_required_in_dB)
% % % % % % %         continue;
    end
    XB=XBs(:,k).*(lin_snrs(k,:)');
    XB=[XB;conj(flip(XB(2:end-1)))];
    xb=real(ifft(XB));
    xb=xb(1:numel(xb)/2);
    xbc=1.0*(xb(1:128)+xb_last(129:256));
    x=[x;xbc];
    xb_last=xb;
end
x(1:Fs)=[];

%show what the final spectrum looks like
figure
spectrogram(x,512,250,[],Fs,'yaxis');
title('output signal');

%plotting expected and actual output signal
actual_output_signal_include();
figure;
plot(linspace(0,numel(x)/Fs,numel(x)),x(1:end));
hold on;
actual_output_signal=actual_output_signal(61*257+0-252:end);%there is an offset
plot(linspace(0,numel(actual_output_signal)/Fs,numel(actual_output_signal)),actual_output_signal);
hold off;
xlabel('time (s)');
legend('expected','actual');
title('output signal in time domain');

