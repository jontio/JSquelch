#include "CppUTest/TestHarness.h"
#include "../src/dsp/dsp.h"
#include "../src/util/RuntimeError.h"
#include <complex>

TEST_GROUP(Test_DSP_MovingAverage)
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

TEST(Test_DSP_MovingAverage, QuantizationZeroInt)
{
    //dont do quantization resetting
    int ma_sz=10000;
    JDsp::MovingAverage<int> ma=JDsp::MovingAverage<int>(ma_sz);
    //quantization should be zero for int
    int ave=0;
    for(int k=0;k<ma_sz-1;k++)
    {
        double f = 2.0*((double)rand() / RAND_MAX)-1.0;
        int i=f*10000;
        ave+=i;
        ma.Update(i);
    }
    ave/=ma_sz;
    LONGS_EQUAL(ave,ma.val);
}

TEST(Test_DSP_MovingAverage, MultipleQuantizationResettingComplex)
{
    //make quantization bad 3x rounds
    int ma_sz=1000;
    JDsp::MovingAverage<std::complex<double>> ma=JDsp::MovingAverage<std::complex<double>>(ma_sz);
    for(int k=0;k<3*ma_sz;k++)
    {
        double f1 = 2.0*((double)rand() / RAND_MAX)-1.0;
        f1*=100000000000000000.0;
        double f2 = 2.0*((double)rand() / RAND_MAX)-1.0;
        f2*=100000000000000000.0;
        std::complex<double> z=std::complex<double>(f1,f2);
        ma.Update(z);
    }

    //quantization should be removed after n*ma_sz updates for n being an integer
    std::complex<double> ave=0;
    for(int k=0;k<ma_sz;k++)
    {
        double f1 = 2.0*((double)rand() / RAND_MAX)-1.0;
        double f2 = 2.0*((double)rand() / RAND_MAX)-1.0;
        std::complex<double> z=std::complex<double>(f1,f2);
        ave+=z;
        ma.Update(z);
    }
    ave/=(double)ma_sz;
    CHECK_EQUAL(ave.real(),ma.val.real());
    CHECK_EQUAL(ave.imag(),ma.val.imag());
}

TEST(Test_DSP_MovingAverage, MultipleQuantizationResettingDouble)
{
    //make quantization bad 3x rounds
    int ma_sz=1000;
    JDsp::MovingAverage<double> ma=JDsp::MovingAverage<double>(ma_sz);
    for(int k=0;k<3*ma_sz;k++)
    {
        double f = 2.0*((double)rand() / RAND_MAX)-1.0;
        f*=100000000000000000.0;
        ma.Update(f);
    }

    //quantization should be removed after n*ma_sz updates for n being an integer
    double ave=0;
    for(int k=0;k<ma_sz;k++)
    {
        double f = 2.0*((double)rand() / RAND_MAX)-1.0;
        ave+=f;
        ma.Update(f);
    }
    ave/=(double)ma_sz;
    CHECK_EQUAL(ave,ma.val);


}

TEST(Test_DSP_MovingAverage, QuantizationResettingComplex)
{
    //make quantization bad 1 round
    int ma_sz=100;
    JDsp::MovingAverage<std::complex<double>> ma=JDsp::MovingAverage<std::complex<double>>(ma_sz);
    for(int k=0;k<ma_sz;k++)
    {
        double f1 = 2.0*((double)rand() / RAND_MAX)-1.0;
        f1*=100000000000000000.0;
        double f2 = 2.0*((double)rand() / RAND_MAX)-1.0;
        f2*=100000000000000000.0;
        std::complex<double> z=std::complex<double>(f1,f2);
        ma.Update(z);
    }

    //quantization should be removed after ma_sz updates
    std::complex<double> ave=0;
    for(int k=0;k<ma_sz;k++)
    {
        double f1 = 2.0*((double)rand() / RAND_MAX)-1.0;
        double f2 = 2.0*((double)rand() / RAND_MAX)-1.0;
        std::complex<double> z=std::complex<double>(f1,f2);
        ave+=z;
        ma.Update(z);
    }
    ave/=(double)ma_sz;
    CHECK_EQUAL(ave.real(),ma.val.real());
    CHECK_EQUAL(ave.imag(),ma.val.imag());
}

TEST(Test_DSP_MovingAverage, QuantizationResettingDouble)
{
    //make quantization bad 1 round
    int ma_sz=100;
    JDsp::MovingAverage<double> ma=JDsp::MovingAverage<double>(ma_sz);
    for(int k=0;k<ma_sz;k++)
    {
        double f = 2.0*((double)rand() / RAND_MAX)-1.0;
        f*=100000000000000000.0;
        ma.Update(f);
    }

    //quantization should be removed after ma_sz updates
    double ave=0;
    for(int k=0;k<ma_sz;k++)
    {
        double f = 2.0*((double)rand() / RAND_MAX)-1.0;
        ave+=f;
        ma.Update(f);
    }
    ave/=(double)ma_sz;
    CHECK_EQUAL(ave,ma.val);
}

TEST(Test_DSP_MovingAverage, OutOfBoundsComplex)
{
    try
    {
        JDsp::MovingAverage<std::complex<double>> ma=JDsp::MovingAverage<std::complex<double>>(-1);
    }
    catch (const RuntimeError&)
    {
        return;
    }
    FAIL("MovingAverage did not throw error");
}

TEST(Test_DSP_MovingAverage, OutOfBoundsInt)
{
    try
    {
        JDsp::MovingAverage<int> ma=JDsp::MovingAverage<int>(-1);
    }
    catch (const RuntimeError&)
    {
        return;
    }
    FAIL("MovingAverage did not throw error");
}

TEST(Test_DSP_MovingAverage, OutOfBoundsDouble)
{
    try
    {
        JDsp::MovingAverage<double> ma=JDsp::MovingAverage<double>(-1);
    }
    catch (const RuntimeError&)
    {
        return;
    }
    FAIL("MovingAverage did not throw error");
}

TEST(Test_DSP_MovingAverage, ResetOnChangeSizeComplex)
{
    JDsp::MovingAverage<std::complex<double>> ma=JDsp::MovingAverage<std::complex<double>>(10);
    for(int k=0;k<10;k++)
    {
        std::complex<double> z=std::complex<double>(k,-k);
        ma.Update(z);
    }
    ma.setSize(20);
    DOUBLES_EQUAL(0,ma.val.real(),0.0000000000001);
    DOUBLES_EQUAL(0,ma.val.imag(),0.0000000000001);
}

TEST(Test_DSP_MovingAverage, ResetOnChangeSizeInt)
{
    JDsp::MovingAverage<int> ma=JDsp::MovingAverage<int>(10);
    for(int k=0;k<10;k++)ma.Update(k);
    ma.setSize(20);
    LONGS_EQUAL(0,ma.val);
}

TEST(Test_DSP_MovingAverage, ResetOnChangeSizeDouble)
{
    JDsp::MovingAverage<double> ma=JDsp::MovingAverage<double>(10);
    for(int k=0;k<10;k++)ma.Update(k);
    ma.setSize(20);
    DOUBLES_EQUAL(0,ma.val,0.0000000000001);
}

TEST(Test_DSP_MovingAverage, SmallMeanComplex)
{
    //must not sum size else resetting is done and test is pointless
    JDsp::MovingAverage<std::complex<double>> ma=JDsp::MovingAverage<std::complex<double>>(10);
    for(int k=0;k<9;k++)
    {
        std::complex<double> z=std::complex<double>(k,-k);
        ma.Update(z);
    }
    DOUBLES_EQUAL(3.6,ma.val.real(),0.0000000000001);
    DOUBLES_EQUAL(-3.6,ma.val.imag(),0.0000000000001);
}

TEST(Test_DSP_MovingAverage, SmallMeanInt)
{
    //must not sum size else resetting is done and test is pointless
    JDsp::MovingAverage<int> ma=JDsp::MovingAverage<int>(10);
    for(int k=0;k<9;k++)ma.Update(k);
    LONGS_EQUAL(3,ma.val);
}

TEST(Test_DSP_MovingAverage, SmallMeanDouble)
{
    //must not sum size else resetting is done and test is pointless
    JDsp::MovingAverage<double> ma=JDsp::MovingAverage<double>(10);
    for(int k=0;k<9;k++)ma.Update(k);
    DOUBLES_EQUAL(3.6,ma.val,0.0000000000001);
}


TEST(Test_DSP_MovingAverage, ZeroMeanComplex)
{
    JDsp::MovingAverage<std::complex<double>> ma=JDsp::MovingAverage<std::complex<double>>(0);
    for(int k=0;k<9;k++)
    {
        std::complex<double> z=std::complex<double>(k,-k);
        ma.Update(z);
    }
    DOUBLES_EQUAL(8,ma.val.real(),0.0000000000001);
    DOUBLES_EQUAL(-8,ma.val.imag(),0.0000000000001);
}

TEST(Test_DSP_MovingAverage, ZeroMeanInt)
{
    JDsp::MovingAverage<int> ma=JDsp::MovingAverage<int>(0);
    for(int k=0;k<9;k++)ma.Update(k);
    LONGS_EQUAL(8,ma.val);
}

TEST(Test_DSP_MovingAverage, ZeroMeanDouble)
{
    JDsp::MovingAverage<double> ma=JDsp::MovingAverage<double>(0);
    for(int k=0;k<9;k++)ma.Update(k);
    DOUBLES_EQUAL(8,ma.val,0.0000000000001);
}


TEST(Test_DSP_MovingAverage, SelfAssignmentComplex)
{
    //create
    JDsp::MovingAverage<std::complex<double>> ma=JDsp::MovingAverage<std::complex<double>>(1);
    //self assignment
    ma=ma;
    LONGS_EQUAL(1,ma.getSize());
}

TEST(Test_DSP_MovingAverage, SelfAssignmentInt)
{
    //create
    JDsp::MovingAverage<int> ma=JDsp::MovingAverage<int>(1);
    //self assignment
    ma=ma;
    LONGS_EQUAL(1,ma.getSize());
}

TEST(Test_DSP_MovingAverage, SelfAssignmentDouble)
{
    //create
    JDsp::MovingAverage<double> ma=JDsp::MovingAverage<double>(1);
    //self assignment
    ma=ma;
    LONGS_EQUAL(1,ma.getSize());
}

TEST(Test_DSP_MovingAverage, AssignmentComplex)
{
    //create
    JDsp::MovingAverage<std::complex<double>> ma=JDsp::MovingAverage<std::complex<double>>(1);
    //assignment
    JDsp::MovingAverage<std::complex<double>> ma2=JDsp::MovingAverage<std::complex<double>>(ma);
    LONGS_EQUAL(1,ma.getSize());
}

TEST(Test_DSP_MovingAverage, AssignmentInt)
{
    //create
    JDsp::MovingAverage<double> ma=JDsp::MovingAverage<double>(1);
    //assignment
    JDsp::MovingAverage<double> ma2=JDsp::MovingAverage<double>(ma);
    LONGS_EQUAL(1,ma.getSize());
}

TEST(Test_DSP_MovingAverage, AssignmentDouble)
{
    //create
    JDsp::MovingAverage<int> ma=JDsp::MovingAverage<int>(1);
    //assignment
    JDsp::MovingAverage<int> ma2=JDsp::MovingAverage<int>(ma);
    LONGS_EQUAL(1,ma.getSize());
}

TEST(Test_DSP_MovingAverage, CopyAssignmentComplex)
{
    //create
    JDsp::MovingAverage<std::complex<double>> ma=JDsp::MovingAverage<std::complex<double>>(1);
    //copy assignment
    ma=JDsp::MovingAverage<std::complex<double>>(10);
    LONGS_EQUAL(10,ma.getSize());
}

TEST(Test_DSP_MovingAverage, CopyAssignmentInt)
{
    //create
    JDsp::MovingAverage<int> ma=JDsp::MovingAverage<int>(1);
    //copy assignment
    ma=JDsp::MovingAverage<int>(10);
    LONGS_EQUAL(10,ma.getSize());
}

TEST(Test_DSP_MovingAverage, CopyAssignmentDouble)
{
    //create
    JDsp::MovingAverage<double> ma=JDsp::MovingAverage<double>(1);
    //copy assignment
    ma=JDsp::MovingAverage<double>(10);
    LONGS_EQUAL(10,ma.getSize());
}

TEST(Test_DSP_MovingAverage, CreateComplex)
{
    //create 3 ways
    JDsp::MovingAverage<std::complex<double>> ma=JDsp::MovingAverage<std::complex<double>>(1);
    LONGS_EQUAL(1,ma.getSize());
    JDsp::MovingAverage<std::complex<double>> mb;
    LONGS_EQUAL(0,mb.getSize());
    JDsp::MovingAverage<std::complex<double>> mc(ma);
    LONGS_EQUAL(1,mc.getSize());
}

TEST(Test_DSP_MovingAverage, CreateInt)
{
    //create 3 ways
    JDsp::MovingAverage<int> ma=JDsp::MovingAverage<int>(1);
    LONGS_EQUAL(1,ma.getSize());
    JDsp::MovingAverage<int> mb;
    LONGS_EQUAL(0,mb.getSize());
    JDsp::MovingAverage<int> mc(ma);
    LONGS_EQUAL(1,mc.getSize());
}

TEST(Test_DSP_MovingAverage, CreateDouble)
{
    //create 3 ways
    JDsp::MovingAverage<double> ma=JDsp::MovingAverage<double>(1);
    LONGS_EQUAL(1,ma.getSize());
    JDsp::MovingAverage<double> mb;
    LONGS_EQUAL(0,mb.getSize());
    JDsp::MovingAverage<double> mc(ma);
    LONGS_EQUAL(1,mc.getSize());
}




