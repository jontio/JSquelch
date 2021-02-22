#include "../src/dsp/dsp.h"
#include "../src/util/RuntimeError.h"
#include "../src/util/stdio_utils.h"
#include "../src/util/file_utils.h"
#include "../src/voicedetectionalgo.h"
#include <QFile>
#include <QDir>
#include <QDataStream>
#include <iostream>

//important for Qt include cpputest last as it mucks up new and causes compiling to fail
#include "CppUTest/TestHarness.h"

//this unit test creates a raw audio file in LE 32bit float 8000Hz mono doing the algo

TEST_GROUP(Test_VoiceDetectionAlgoTests)
{
    const double double_tolerance_for_db_snr=0.13;
    const double double_tolerance_for_output_signal=0.05;

    //cpputest does not seem to work for qt accessing io such as qfile qdebug etc
    //i get momory leak messages so im turning it off for this unit where i really want qfile
    void setup()
    {
        MemoryLeakWarningPlugin::turnOffNewDeleteOverloads();
    }

    void teardown()
    {
        MemoryLeakWarningPlugin::turnOnNewDeleteOverloads();
    }
};

TEST(Test_VoiceDetectionAlgoTests, AlgoProcessAudioTest)
{
    //reads a raw file processes it and write the result back to disk

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
    x.fill(0,300);//size doesn't matter with this algo class

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

        //add the audio to the algo
        algo+=x;

        //process the audio wile we have some
        while(!algo.process().empty())
        {
            //skip nan blocks
            if(std::isnan(algo.snr_db))continue;

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
    std::cout <<"VoiceDetectionAlgoTests: cpu_load="<<cpu_load*100.0<<"% at "<<Fs<<"Hz sample rate"<< std::endl;

#ifdef GENERATE_TEST_OUTPUT_FILES
    QDir dir;
    dir.mkpath(QString(TEST_OUTPUT_PATH));
    file.setFileName(QString(TEST_OUTPUT_PATH)+"/AlgoProcessAudioTest.raw");
    if(!file.open(QIODevice::WriteOnly|QIODevice::Truncate))
    {
        RUNTIME_ERROR("failed to open file for writing",1);
    }
    for(int k=0;k<actual_output_signal.size();k++)
    {
        datastream<<actual_output_signal[k];
    }
    file.close();
#endif

}

