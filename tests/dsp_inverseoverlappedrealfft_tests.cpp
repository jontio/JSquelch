#include "../src/dsp/dsp.h"
#include "../src/util/RuntimeError.h"
#include <random>

//important for Qt include cpputest last as it mucks up new and causes compiling to fail
#include "CppUTest/TestHarness.h"


TEST_GROUP(Test_DSP_InverseOverlappedRealFFT)
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

TEST(Test_DSP_InverseOverlappedRealFFT, IdenpotentTest)
{

    JDsp::OverlappedRealFFT fft(128);
    JDsp::InverseOverlappedRealFFT ifft(fft.getInSize());

    //normally distributed numbers for the signal
    std::default_random_engine generator(1);
    std::normal_distribution<double> distribution(0.0,1.0);

    QVector<double> x(fft.getInSize());
    QVector<double> expected;
    QVector<double> actual;

    for(int k=0;k<20;k++)
    {
        for(int i=0;i<x.size();i++)x[i]=distribution(generator);
        expected+=x;
        //forward
        fft<<x;
        //inverse
        ifft<<fft;
        actual+=ifft;
    }

    //nb windowing may be already done in the InverseOverlappedRealFFT class

    //windowing testing
//    for(int k=0;k<actual.size();k++)
//    {
//        double dn=fft.getInSize();
//        actual[k]*=std::abs(std::sin(M_PI*(((double)k)+0.5)/dn)/(1.2729*dn-1.1621))+1.0;
//    }

    //find max difference
//    double mv=0;
//    for(int k=0;k<actual.size();k++)
//    {
//        if(k>=(expected.size()-fft.getInSize()))continue;
//        if(k<fft.getInSize())continue;
//        double v=std::abs(1.0-expected[k]/actual[k+fft.getInSize()]);
//        if(isnanf(v))continue;
//        if(isinff(v))continue;
//        if(v>mv)mv=v;
//    }
//    printf("mv=%f\n",mv);

    //this inverse isn't exact and has a bit of a bump every 128 samples.
    //it's about 0.6% at 128 samples. it gets worse for small n and better for large n
    //the bump is about abs(sin(pi*(x-0.5)/128)/161.8)+1.0 in matlab indexing for 128
    //generally it's about abs(sin(pi*(x-0.5)/n)/(1.2729*n-1.1621))+1.0
    for(int k=fft.getInSize();k<expected.size()-fft.getInSize();k++)
    {
        //if no output windowing is done
//        DOUBLES_EQUAL(expected[k],actual[k+fft.getInSize()],0.02);
//        DOUBLES_EQUAL(1.0,expected[k]/actual[k+fft.getInSize()],0.0062);//this is due to the inverse not quite being exact but it's good enough I think
        //if output windowing is done
        DOUBLES_EQUAL(expected[k],actual[k+fft.getInSize()],0.0002);
        DOUBLES_EQUAL(0.0,std::abs(1.0-expected[k]/actual[k+fft.getInSize()]),0.00005);//this is due to the inverse not quite being exact but it's good enough I think
    }

}
