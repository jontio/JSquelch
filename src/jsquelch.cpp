#include "jsquelch.h"
#include "ui_jsquelch.h"
#include <iostream>
#include <QtDebug>
#include <limits>
#include <random>

using namespace std;


JSquelch::JSquelch(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::JSquelch)
{
    ui->setupUi(this);

    //this is what I'm planning on using for the settings
    //JDsp::OverlappedRealFFT fft(128);//for an fft that takes 128 as in will output 2*128+1=257
    //JDsp::MovingNoiseEstimator mne(fft.getOutSize(),16,16,32);

    //normally distributed numbers for the signal
    std::default_random_engine generator(1);
    std::normal_distribution<double> distribution(0.0,123.0);

    JDsp::OverlappedRealFFT fft(3);//for an fft that takes 3 as in will output 2*3+1=7
    QVector<double> x;//this would be the input signal split up into blocks
    JDsp::MovingNoiseEstimator mne(fft.getOutSize(),16,16,32);
    for(int k=0;k<640;k++)
    {
        x={distribution(generator),distribution(generator),distribution(generator)};
        mne<<(fft<<x);
        if((k+1)%64)continue;
        qDebug()<<mne;
    }

}

JSquelch::~JSquelch()
{
    delete ui;
}

