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

    JDsp::OverlappedRealFFT fft(128);
    JDsp::MovingNoiseEstimator mne(fft.getOutSize(),16,16,62);
    JDsp::MovingSignalEstimator mse(fft.getOutSize(),8,3,96,62,8);
    JDsp::InverseOverlappedRealFFT ifft(fft.getInSize());

    //normally distributed numbers for the signal
    std::default_random_engine generator(1);
    std::normal_distribution<double> distribution(0.0,1.0);

//    qDebug()<<"a=[];b=[];";


    QFile file(QString(MATLAB_PATH)+"/test_signal_time.raw");
    if(!file.open(QIODevice::ReadOnly))return;
    QDataStream datastream(&file);
    datastream.setByteOrder(QDataStream::LittleEndian);
    datastream.setFloatingPointPrecision (QDataStream::SinglePrecision);
    QVector<double> x(fft.getInSize());
    QVector<double> y;
    QVector<double> y_sound;
    QVector<double> yabs;
    QVector<double> ymne;
    QVector<double> ymse;

    JDsp::VectorDelayLine<double> vdl;
    JDsp::VectorDelayLine<JFFT::cpx_type> vcomplexdl;
//    vdl.setSize(QPair<int,int>(257,51));
//    vcomplexdl.setSize(QPair<int,int>(512,51));

    JDsp::OverlappedRealFFTDelayLine fft_delay1(fft,51);
    JDsp::OverlappedRealFFTDelayLine fft_delay2(fft,6);

//    x={1,-6};
//    qDebug()<<(vdl.update(x));
//    x={2,8};
//    qDebug()<<(vdl.update(x));
//    x={3,2};
//    qDebug()<<(vdl.update(x));
//    x={4,0.5};
//    qDebug()<<(vdl.update(x));
//    x={0,0};
//    qDebug()<<vdl.update(x);
//    x={0,0};
//    qDebug()<<vdl.update(x);
//    x={0,0};
//    qDebug()<<vdl.update(x);

//    return;


    auto start = std::chrono::high_resolution_clock::now();

    for(int k=0;k<40000;k++)//400;k++)
    {
        for(int i=0;i<x.size();i++)
        {
//            x[i]=distribution(generator);
            if(!file.atEnd())
            {
                datastream>>x[i];
            }
            else
            {
                break;
                x[i]=distribution(generator);
            }



        }
        if(file.atEnd())break;


//        qDebug()<<x;
//        return;

//        Stdio_Utils::Matlab::print("a(end+1:end+128)",x);

        fft<<x;//signal in
//        ifft<<fft;

//        ifft




//        qDebug()<<ifft.update(fft.Xfull);

//        ifft.update(fft.Xfull);

//        qDebug()<<fft.Xfull.size()<<ifft.update(fft.Xfull).size();

//        Stdio_Utils::Matlab::print("b(end+1:end+128)",ifft);


//        fft.Xabs[20]=100;//artificial tone
        mne<<fft;//output of fft to input of mne


//fft.Xabs=vdl.update(fft.Xabs);
//fft.Xfull=vcomplexdl.update(fft.Xfull);

fft_delay1.delay(fft);

ymne+=mne.val;
//yabs+=fft.Xabs;


        fft/=mne;//normalize output of fft

        yabs+=fft.Xabs;

//        fft.Xabs[0]=0;//no need for dc
//        fft.Xabs[256]=0;//no need for nyquest
        mse<<fft;//output of fft into mse


        ymse+=mse;

        fft_delay2.delay(fft);

        fft*=mse;//scale fft output by signal estimate
        ifft<<fft;//output of fft to inverse fft to produce reconstuted signal

//        if((k+1)%64)continue;

        y+=10.0*log10(mse.voice_snr_estimate);

//        if(10.0*log10(mse.voice_snr_estimate)>5.0)y_sound+=ifft;

y_sound+=ifft;



//        qDebug()<<10.0*log10(mse.voice_snr_estimate);//mse.voice_snr_estimate;
    }

    auto finish = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> elapsed = finish - start;
    std::cout << "Elapsed Time: " << elapsed.count() << " ms" << std::endl;

    file.close();



    File_Utils::Matlab::print((QString(MATLAB_PATH)+"/actual_snr_estimate_db_signal_include.m").toLocal8Bit().data(),"actual_snr_estimate_db_signal",y);
    File_Utils::Matlab::print((QString(MATLAB_PATH)+"/actual_output_signal_include.m").toLocal8Bit().data(),"actual_output_signal",y_sound);

    return;

    File_Utils::Matlab::print("/home/jontio/ds/3TBusb/linux_downloads/ai_hiss/yq_include.m","yq",y);
    File_Utils::Matlab::print("/home/jontio/ds/3TBusb/linux_downloads/ai_hiss/y_sound_include.m","y_sound",y_sound);


    File_Utils::Matlab::print("/home/jontio/ds/3TBusb/linux_downloads/ai_hiss/yabs_include.m","yabs",yabs);
    File_Utils::Matlab::print("/home/jontio/ds/3TBusb/linux_downloads/ai_hiss/ymne_include.m","ymne",ymne);
    File_Utils::Matlab::print("/home/jontio/ds/3TBusb/linux_downloads/ai_hiss/ymse_include.m","ymse",ymse);
//    printf("plot(a);hold on;plot(b(129:end));hold off;\n");
//    printf("plot(a(1:end-128)./b(129:end));\n");




}

JSquelch::~JSquelch()
{
    delete ui;
}

