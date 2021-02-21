#include "../src/dsp/dsp.h"
#include "../src/util/RuntimeError.h"
#include "../src/util/stdio_utils.h"
#include "../src/util/file_utils.h"
#include <QFile>
#include <QDataStream>
#include <iostream>

//important for Qt include cpputest last as it mucks up new and causes compiling to fail
#include "CppUTest/TestHarness.h"

//this unit test is the big one and tests the C++ algo implimentation with
//that of matlab. the output signal and snr are compared

TEST_GROUP(Test_DSP_InverseOverlappedRealFFTDelayLine)
{
    const double double_tolerance=0.0000001;

    //cpputest does not seem to work for qt accessing io such as qfile qdebug etc
    //i get momory leak messages so im turning it off for this unit where i really want qfile
    void setup()
    {
//        MemoryLeakWarningPlugin::turnOffNewDeleteOverloads();
    }

    void teardown()
    {
//        MemoryLeakWarningPlugin::turnOnNewDeleteOverloads();
    }
};

TEST(Test_DSP_InverseOverlappedRealFFTDelayLine, DoubleZeroTest)
{
    JDsp::InverseOverlappedRealFFT ifft(128);
    JDsp::InverseOverlappedRealFFTDelayLine delayline(0);
    for(int k=0;k<ifft.size();k++)
    {
        ifft[k]=k;
    }

    delayline.delay(ifft);

    for(int k=0;k<ifft.size();k++)
    {
        DOUBLES_EQUAL(k,ifft[k],double_tolerance);
    }

}

TEST(Test_DSP_InverseOverlappedRealFFTDelayLine, DoubleDelayTest)
{
    JDsp::InverseOverlappedRealFFT ifft(128);
    JDsp::InverseOverlappedRealFFTDelayLine delayline(100);
    double current_val=0;
    QVector<double> result;
    for(int k=0;k<ifft.size();k++)
    {
        ifft[k]=current_val;
        current_val++;
    }

    delayline.delay(ifft);

    for(int k=0;k<ifft.size();k++)
    {
        result+=ifft[k];
    }

    for(int k=0;k<ifft.size();k++)
    {
        ifft[k]=current_val;
        current_val++;
    }

    delayline.delay(ifft);

    for(int k=0;k<ifft.size();k++)
    {
        result+=ifft[k];
    }

    for(int k=0;k<100;k++)
    {
        DOUBLES_EQUAL(0,result[k],double_tolerance);
    }

    for(int k=100;k<result.size();k++)
    {
        DOUBLES_EQUAL(k-100,result[k],double_tolerance);
    }

}
