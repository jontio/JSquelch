#include "jsquelch.h"
#include "ui_jsquelch.h"
#include <iostream>
#include <QtDebug>
#include <limits>
#include <random>
#include "util/stdio_utils.h"
#include "util/file_utils.h"
#include <QFile>

using namespace std;


JSquelch::JSquelch(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::JSquelch)
{
    ui->setupUi(this);

    //algo things
    //these settings have to match exactly with the create.m Matlab script
    JDsp::OverlappedRealFFT fft(128);
    JDsp::MovingNoiseEstimator mne(fft.getOutSize(),16,16,62);
    JDsp::MovingSignalEstimator mse(fft.getOutSize(),8,3,96,62,8);
    JDsp::InverseOverlappedRealFFT ifft(fft.getInSize());
    JDsp::OverlappedRealFFTDelayLine fft_delay1(fft,51);
    JDsp::OverlappedRealFFTDelayLine fft_delay2(fft,6);
    QVector<double> x(fft.getInSize());

    //for alignment of output signal
    double movmax_delay_in_frames=8000*1;
    JDsp::MovingMax<double> movmax(movmax_delay_in_frames/ifft.getOutSize());
    JDsp::InverseOverlappedRealFFTDelayLine delayline(movmax_delay_in_frames/2.0);//this should be half of movave
    JDsp::ScalarDelayLine<double> delayline2(movmax_delay_in_frames/(128*2)-31);//not sure how this changes for mne and mse

    double last_snr=0;

    //load test file
    QFile file(QString(MATLAB_PATH)+"/test_signal_time.raw");
    if(!file.open(QIODevice::ReadOnly))
    {
        RUNTIME_ERROR("failed to open file",1);
    }
    QDataStream datastream(&file);
    datastream.setByteOrder(QDataStream::LittleEndian);
    datastream.setFloatingPointPrecision (QDataStream::SinglePrecision);

    //what we want from the algo output
    QVector<double> actual_snr_estimate_db_signal;
    QVector<double> actual_output_signal;

    //see how fast this is
    auto start = std::chrono::high_resolution_clock::now();

    //run algo over file
    while(!file.atEnd())
    {
        //read some audio
        for(int i=0;i<x.size();i++)
        {
            if(file.atEnd())break;
            datastream>>x[i];
        }
        if(file.atEnd())break;

        //do main algo
        fft<<x;//signal in
        mne<<fft;//output of fft to input of mne
        fft_delay1.delay(fft);//1st delayline to match matlab
        fft/=mne;//normalize output of fft
        mse<<fft;//output of fft into mse
        fft_delay2.delay(fft);//2nd delayline to match matlab
        fft*=mse;//scale fft output by signal estimate

        //now reconstruct a signal and get volume and adjust timing etc
        ifft<<fft;//output of fft to inverse fft to produce reconstuted signal
        movmax<<ifft;//update moving max with time domain output signal
        delayline.delay(ifft);//delay the output by half the moving window length so we are looking forward and back in time.
        ifft/=(1.25*movmax);//normalize the time domain output signal

        mse.voice_snr_estimate=delayline2.update(mse.voice_snr_estimate);//delay snr as audio is delayed
        if(isnan(mse.voice_snr_estimate))
        {
//            continue;//some snr estimates may be nan. if so then needs to be ignored
            mse.voice_snr_estimate=last_snr;
        }
        double snr_db=10.0*log10(mse.voice_snr_estimate);

        last_snr=mse.voice_snr_estimate;

        //keep snr db
        actual_snr_estimate_db_signal+=snr_db;

        //keep output signal when snr_db is good
        if(snr_db<5.0) continue;
        actual_output_signal+=ifft;

    }
    file.close();

    auto finish = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> elapsed = finish - start;
    double fps=((double)(fft.getInSize()*actual_snr_estimate_db_signal.size()))/(elapsed.count()/1000.0);
    const double Fs=8000;
    double cpu_load=Fs/fps;
    std::cout <<"cpu_load="<<cpu_load*100.0<<"% at "<<Fs<<"Hz sample rate"<< std::endl;

    file.setFileName(QString(MATLAB_PATH)+"/delme.raw");
    if(!file.open(QIODevice::WriteOnly|QIODevice::Truncate))
    {
        RUNTIME_ERROR("failed to open file for writing",1);
    }
    for(int k=0;k<actual_output_signal.size();k++)
    {
        datastream<<actual_output_signal[k];
    }
    file.close();


}

JSquelch::~JSquelch()
{
    delete ui;
}

