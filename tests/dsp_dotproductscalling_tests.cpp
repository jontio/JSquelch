#include "../src/dsp/dsp.h"
#include "../src/util/RuntimeError.h"
#include "../src/util/stdio_utils.h"
#include <random>

//important for Qt include cpputest last as it mucks up new and causes compiling to fail
#include "CppUTest/TestHarness.h"


TEST_GROUP(Test_DSP_DotProductScalling)
{
    double double_tolerance=0.00001;


    void setup()
    {

    }

    void teardown()
    {
        // This gets run after every test
    }
};

TEST(Test_DSP_DotProductScalling, ScaleTest)
{

    JDsp::OverlappedRealFFT fft(128);
    JDsp::MovingSignalEstimator mse(fft.getOutSize(),8,3,96,62,8);

    //normally distributed numbers for the signal
    std::default_random_engine generator(1);
    std::normal_distribution<double> distribution(0.0,1.0);

    QVector<JFFT::cpx_type> expected_complex;
    QVector<double> expected;
    QVector<JFFT::cpx_type> actual_complex;
    QVector<double> actual;

    QVector<double> x(fft.getInSize());
    for(int k=0;k<20;k++)
    {
        for(int i=0;i<x.size();i++)x[i]=distribution(generator);
        CHECK(x.size()==fft.getInSize());
        fft<<x;
        CHECK(mse.val.size()==fft.getOutSize());
        mse<<fft;

        QVector<double> a=fft.Xabs;
        for(int m=0;m<a.size();m++)a[m]*=mse[m];
        expected+=a;

        QVector<JFFT::cpx_type> b=fft.Xfull;
        for(int m=0;m<mse.val.size();m++)
        {
            CHECK(m<b.size());
            b[m]*=mse[m];
        }
        for(int m=1;m<mse.val.size()-1;m++)
        {
            CHECK((b.size()-m)>=0);
            CHECK(m<mse.val.size());
            b[b.size()-m]*=mse[m];
        }
        expected_complex+=b;

        fft*=mse;
        actual+=fft.Xabs;
        actual_complex+=fft.Xfull;
    }


    CHECK(actual.size()==expected.size());
    CHECK(actual_complex.size()==expected_complex.size());

    for(int k=0;k<actual.size();k++)
    {
        DOUBLES_EQUAL(expected[k],actual[k],double_tolerance);
    }

    for(int k=0;k<actual_complex.size();k++)
    {
        DOUBLES_EQUAL(expected_complex[k].real(),actual_complex[k].real(),double_tolerance);
        DOUBLES_EQUAL(expected_complex[k].imag(),actual_complex[k].imag(),double_tolerance);
    }

}
