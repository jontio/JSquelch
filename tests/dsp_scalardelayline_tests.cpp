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

TEST_GROUP(Test_DSP_ScalarDelayLine)
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

TEST(Test_DSP_ScalarDelayLine, DoubleDelay2Test)
{
    JDsp::ScalarDelayLine<double> delayline(62);
    QVector<double> input(62*2);
    for(int k=0;k<62;k++)
    {
        input[k]=k;
        input[k+62]=k;
        delayline<<input[k];
    }
    for(int k=0;k<62;k++)
    {
        DOUBLES_EQUAL(input[k+62],delayline<<input[k],double_tolerance);
    }
}

TEST(Test_DSP_ScalarDelayLine, DoubleZeroTest)
{
    JDsp::ScalarDelayLine<double> delayline(0);
    QVector<double> input(62*2);
    for(int k=0;k<62;k++)
    {
        input[k]=k;
        input[k+62]=k;
        delayline.update(input[k]);
    }
    for(int k=0;k<62;k++)
    {
        delayline.update(input[k]);
        DOUBLES_EQUAL(input[k],delayline,double_tolerance);
    }
}

TEST(Test_DSP_ScalarDelayLine, DoubleDelayTest)
{
    JDsp::ScalarDelayLine<double> delayline(62);
    QVector<double> input(62*2);
    for(int k=0;k<62;k++)
    {
        input[k]=k;
        input[k+62]=k;
        delayline.update(input[k]);
    }
    for(int k=0;k<62;k++)
    {
        delayline.update(input[k]);
        DOUBLES_EQUAL(input[k+62],delayline,double_tolerance);
    }
}


