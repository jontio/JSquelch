#include "../src/dsp/dsp.h"
#include "../src/util/RuntimeError.h"
#include "../src/util/stdio_utils.h"
#include "../src/util/file_utils.h"
#include <QFile>
#include <QDataStream>
#include <iostream>
#include <QDir>

//important for Qt include cpputest last as it mucks up new and causes compiling to fail
#include "CppUTest/TestHarness.h"

//uses matlab input and output and compare the AGC with these

TEST_GROUP(Test_DSP_AGC)
{
    const double double_tolerance=0.02;
    const int min_number_of_tests=100;

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

TEST(Test_DSP_AGC, MatlabCompareTest)
{
    JDsp::AGC agc;
    JDsp::AGC::Settings settings=agc.getSettings();
    settings.K=0.01;
    settings.agc_level=1.0;
    settings.max_gain=100;
    settings.moving_max_window_size=128;
    settings.delayline_size=128;
    agc.setSettings(settings);

    //load input test file
    QFile file(QString(MATLAB_PATH)+"/agc_input_signal.raw");
    CHECK(file.open(QIODevice::ReadOnly));
    QDataStream datastream(&file);
    datastream.setByteOrder(QDataStream::LittleEndian);
    datastream.setFloatingPointPrecision (QDataStream::SinglePrecision);
    QVector<double> input;
    while(!file.atEnd())
    {
        double val;
        datastream>>val;
        input.push_back(val);
    }
    file.close();
    file.setFileName(QString(MATLAB_PATH)+"/agc_output_signal.raw");
    CHECK(file.open(QIODevice::ReadOnly));
    QVector<double> expected_output;
    while(!file.atEnd())
    {
        double val;
        datastream>>val;
        expected_output.push_back(val);
    }
    file.close();

    QVector<double> actual_output=input;
    agc.update(actual_output);

#ifdef GENERATE_TEST_OUTPUT_FILES
    QDir dir;
    dir.mkpath(QString(TEST_OUTPUT_PATH));
    File_Utils::Matlab::print((QString(TEST_OUTPUT_PATH)+"/agc_output_signal_from_cpp_include.m").toLocal8Bit().data(),"agc_output_signal_from_cpp",actual_output);
#endif

    int ntests=0;
    int max_buffer_size=qMin(expected_output.size(),actual_output.size());
    for(int k=1000;k<max_buffer_size;k++)
    {
        DOUBLES_EQUAL(expected_output[k-128],actual_output[k],double_tolerance);
        ntests++;
    }
    CHECK(ntests>=min_number_of_tests);

}


