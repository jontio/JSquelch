#include "../src/dsp/dsp.h"
#include "../src/util/RuntimeError.h"

//important for Qt include cpputest last as it mucks up new and causes compiling to fail
#include "CppUTest/TestHarness.h"


TEST_GROUP(Test_DSP_Hann)
{

    void setup()
    {
        srand(1);
    }

    void teardown()
    {
        // This gets run after every test
    }
};


TEST(Test_DSP_Hann, ShapeTest)
{

    QVector<double> window=JDsp::Hann(4);
    DOUBLES_EQUAL(0,window[0],0.0000000000001);
    DOUBLES_EQUAL(0.75,window[1],0.0000000000001);
    DOUBLES_EQUAL(0.75,window[2],0.0000000000001);
    DOUBLES_EQUAL(0,window[3],0.0000000000001);

    window=JDsp::Hann(5);
    DOUBLES_EQUAL(0,window[0],0.0000000000001);
    DOUBLES_EQUAL(0.5,window[1],0.0000000000001);
    DOUBLES_EQUAL(1,window[2],0.0000000000001);
    DOUBLES_EQUAL(0.5,window[3],0.0000000000001);
    DOUBLES_EQUAL(0,window[4],0.0000000000001);

    window=JDsp::Hann(1);
    DOUBLES_EQUAL(1,window[0],0.0000000000001);

    window=JDsp::Hann(0);

}
