#include "jsquelch.h"
#include "ui_jsquelch.h"
#include <iostream>
#include <QtDebug>
#include <limits>
#include <random>
#include "util/stdio_utils.h"
#include "util/file_utils.h"
#include <QFile>
#include <QDir>
#include "voicedetectionalgo.h"


using namespace std;

void JSquelch::processAudio(const QVector<double> &input, QVector<double> &output)
{

    algo<<input;
    output.clear();

    //process the audio wile we have some
    while(!algo.process().empty())
    {
        //skip nan blocks
        if(isnan(algo.snr_db))algo.snr_db=-100;


        if(algo.snr_db<5.0)
        {
            algo.ifft.fill(0);
        }
        output+=algo.ifft;
    }

//    if(algo.snr_db>5)output=input;
//    else
//    {
//        output.fill(0,input.size());
//    }

//    if(output.size()==0)

//    qDebug()<<input.size()<<output.size();
//    qDebug()<<algo.snr_db;



    if(algo.snr_db>5)ui->led_snr->On(true);
    else ui->led_snr->On(false);
    ui->label_snr->setText(QString::asprintf("%0.1f",algo.snr_db)+"dB");


//    output=input;

//    output.resize(input.size());
//    for(int k=0;k<input.size();k++)
//    {
//        output[k]=0;//input[k];
//    }
}

JSquelch::JSquelch(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::JSquelch)
{
    ui->setupUi(this);
    connect(&audioLoopback,&AudioLoopback::processAudio,this,&JSquelch::processAudio,Qt::DirectConnection);

    audioLoopback.start();

    return;

    VoiceDetectionAlgo algo;

    //load test file
    QFile file(QString(MATLAB_PATH)+"/test_signal_time.raw");
    if(!file.open(QIODevice::ReadOnly))
    {
        RUNTIME_ERROR("failed to open file",1);
    }
    QDataStream datastream(&file);
    datastream.setByteOrder(QDataStream::LittleEndian);
    datastream.setFloatingPointPrecision (QDataStream::SinglePrecision);

    QVector<double> x;
    x.fill(0,128);

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

        //put the audio into the algo
        algo<<x;

        //process the audio wile we have some
        while(!algo.process().empty())
        {
            //skip nan blocks
            if(isnan(algo.snr_db))continue;

            //keep snr_db
            actual_snr_estimate_db_signal+=algo.snr_db;

            //keep output signal when snr_db is good
            if(algo.snr_db<5.0) continue;
            actual_output_signal+=algo.ifft;
        }

    }
    file.close();

    auto finish = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> elapsed = finish - start;
    double fps=((double)(algo.ifft.getOutSize()*actual_snr_estimate_db_signal.size()))/(elapsed.count()/1000.0);
    const double Fs=8000;
    double cpu_load=Fs/fps;
    std::cout <<"application: cpu_load="<<cpu_load*100.0<<"% at "<<Fs<<"Hz sample rate"<< std::endl;

    QDir dir;
    dir.mkpath(QString(TEST_OUTPUT_PATH));
    file.setFileName(QString(TEST_OUTPUT_PATH)+"/delme.raw");
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

