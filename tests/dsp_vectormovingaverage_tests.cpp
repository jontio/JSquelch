#include "../src/dsp/dsp.h"
#include "../src/util/RuntimeError.h"
#include <complex>

//important for Qt include cpputest last as it mucks up new and causes compiling to fail
#include "CppUTest/TestHarness.h"


TEST_GROUP(Test_DSP_VectorMovingAverage)
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

TEST(Test_DSP_VectorMovingAverage, IndexOperatorDouble)
{
    //must not sum size else resetting is done and test is pointless
    JDsp::VectorMovingAverage<double> ma=JDsp::VectorMovingAverage<double>(QPair<int,int>(4,10));
    QVector<double> input={1,2,3,4};
    for(int k=0;k<9;k++)
    {
        ma<<input;
        for(int i=0;i<4;i++)input[i]+=1;
    }
    ma[0]*=2.0;
    ma[1]=10.1;
    ma[3]-=0.2;
    DOUBLES_EQUAL(9.0,ma[0],0.0000000000001);
    DOUBLES_EQUAL(10.1,ma[1],0.0000000000001);
    DOUBLES_EQUAL(6.3,ma[2],0.0000000000001);
    DOUBLES_EQUAL(7.0,ma[3],0.0000000000001);
}

TEST(Test_DSP_VectorMovingAverage, InsertionOperatorDouble)
{
    //must not sum size else resetting is done and test is pointless
    JDsp::VectorMovingAverage<double> ma=JDsp::VectorMovingAverage<double>(QPair<int,int>(4,10));
    QVector<double> input={1,2,3,4};
    for(int k=0;k<9;k++)
    {
        ma<<input;
        for(int i=0;i<4;i++)input[i]+=1;
    }
    DOUBLES_EQUAL(4.5,ma.val[0],0.0000000000001);
    DOUBLES_EQUAL(5.4,ma.val[1],0.0000000000001);
    DOUBLES_EQUAL(6.3,ma.val[2],0.0000000000001);
    DOUBLES_EQUAL(7.2,ma.val[3],0.0000000000001);
}

TEST(Test_DSP_VectorMovingAverage, MultipleQuantizationResettingDouble)
{
    //make quantization bad 7 rounds
    int ma_sz=100;
    JDsp::VectorMovingAverage<double> ma=JDsp::VectorMovingAverage<double>(QPair<int,int>(2,ma_sz));
    for(int k=0;k<7*ma_sz;k++)
    {
        double f1 = 2.0*((double)rand() / RAND_MAX)-1.0;
        f1*=100000000000000000.0;
        double f2 = 2.0*((double)rand() / RAND_MAX)-1.0;
        f2*=100000000000000000.0;
        QVector<double> input={f1,f2};
        ma.update(input);
    }

    //quantization should be removed after ma_sz updates
    double ave1=0;
    double ave2=0;
    for(int k=0;k<ma_sz;k++)
    {
        double f1 = 2.0*((double)rand() / RAND_MAX)-1.0;
        ave1+=f1;
        double f2 = 2.0*((double)rand() / RAND_MAX)-1.0;
        ave2+=f2;
        QVector<double> input={f1,f2};
        ma.update(input);
    }
    ave1/=(double)ma_sz;
    ave2/=(double)ma_sz;
    DOUBLES_EQUAL(ave1,ma.val[0],0.0000000000001);
    DOUBLES_EQUAL(ave2,ma.val[1],0.0000000000001);
}

TEST(Test_DSP_VectorMovingAverage, OutOfBoundsDouble)
{
    try
    {
        JDsp::VectorMovingAverage<double> ma=JDsp::VectorMovingAverage<double>(QPair<int,int>(-10,10));
    }
    catch (const RuntimeError&)
    {
        return;
    }
    FAIL("VectorMovingAverage did not throw error");
}

TEST(Test_DSP_VectorMovingAverage, ResetOnChangeSizeDouble)
{
    JDsp::VectorMovingAverage<double> ma=JDsp::VectorMovingAverage<double>(QPair<int,int>(4,10));
    QVector<double> input={1,2,3,4};
    for(int k=0;k<10;k++)
    {
        ma.update(input);
        for(int i=0;i<4;i++)input[i]+=1;
    }
    ma.setSize(QPair<int,int>(4,20));
    DOUBLES_EQUAL(0,ma.val[0],0.0000000000001);
    DOUBLES_EQUAL(0,ma.val[1],0.0000000000001);
    DOUBLES_EQUAL(0,ma.val[2],0.0000000000001);
    DOUBLES_EQUAL(0,ma.val[3],0.0000000000001);
}

TEST(Test_DSP_VectorMovingAverage, SmallMeanDouble)
{
    //must not sum size else resetting is done and test is pointless
    JDsp::VectorMovingAverage<double> ma=JDsp::VectorMovingAverage<double>(QPair<int,int>(4,10));
    QVector<double> input={1,2,3,4};
    for(int k=0;k<9;k++)
    {
        ma.update(input);
        for(int i=0;i<4;i++)input[i]+=1;
    }
    DOUBLES_EQUAL(4.5,ma.val[0],0.0000000000001);
    DOUBLES_EQUAL(5.4,ma.val[1],0.0000000000001);
    DOUBLES_EQUAL(6.3,ma.val[2],0.0000000000001);
    DOUBLES_EQUAL(7.2,ma.val[3],0.0000000000001);
}

TEST(Test_DSP_VectorMovingAverage, ZeroZeroMeanDouble)
{
    try
    {
        JDsp::VectorMovingAverage<double> ma=JDsp::VectorMovingAverage<double>(QPair<int,int>(0,0));
    }
    catch (const RuntimeError&)
    {
        return;
    }
    FAIL("VectorMovingAverage did not throw error");
}

TEST(Test_DSP_VectorMovingAverage, ZeroMeanDouble)
{
    JDsp::VectorMovingAverage<double> ma=JDsp::VectorMovingAverage<double>(QPair<int,int>(4,0));
    QVector<double> input={1,2,3,4};
    for(int k=0;k<9;k++)ma.update(input);
    DOUBLES_EQUAL(1,ma.val[0],0.0000000000001);
    DOUBLES_EQUAL(2,ma.val[1],0.0000000000001);
    DOUBLES_EQUAL(3,ma.val[2],0.0000000000001);
    DOUBLES_EQUAL(4,ma.val[3],0.0000000000001);
}

TEST(Test_DSP_VectorMovingAverage, SelfAssignmentDouble)
{
    //create
    JDsp::VectorMovingAverage<double> ma=JDsp::VectorMovingAverage<double>(QPair<int,int>(4,5));
    //self assignment
    ma=ma;
    LONGS_EQUAL(4,ma.getSize().first);
    LONGS_EQUAL(5,ma.getSize().second);
}


TEST(Test_DSP_VectorMovingAverage, AssignmentDouble)
{
    //create
    JDsp::VectorMovingAverage<double> ma=JDsp::VectorMovingAverage<double>(QPair<int,int>(4,5));
    //assignment
    JDsp::VectorMovingAverage<double> ma2=JDsp::VectorMovingAverage<double>(ma);
    LONGS_EQUAL(4,ma.getSize().first);
    LONGS_EQUAL(5,ma.getSize().second);
}

TEST(Test_DSP_VectorMovingAverage, CopyAssignmentDouble)
{
    //create
    JDsp::VectorMovingAverage<double> ma=JDsp::VectorMovingAverage<double>(QPair<int,int>(4,5));
    //copy assignment
    ma=JDsp::VectorMovingAverage<double>(QPair<int,int>(7,3));;
    LONGS_EQUAL(7,ma.getSize().first);
    LONGS_EQUAL(3,ma.getSize().second);
}

TEST(Test_DSP_VectorMovingAverage, CreateComplex)
{
    //create 3 ways
    JDsp::VectorMovingAverage<std::complex<double>> ma=JDsp::VectorMovingAverage<std::complex<double>>(QPair<int,int>(4,5));
    LONGS_EQUAL(4,ma.getSize().first);
    LONGS_EQUAL(5,ma.getSize().second);
    JDsp::VectorMovingAverage<std::complex<double>> mb;
    LONGS_EQUAL(0,mb.getSize().first);
    LONGS_EQUAL(0,mb.getSize().second);
    JDsp::VectorMovingAverage<std::complex<double>> mc(ma);
    LONGS_EQUAL(4,mc.getSize().first);
    LONGS_EQUAL(5,mc.getSize().second);
}

TEST(Test_DSP_VectorMovingAverage, CreateInt)
{
    //create 3 ways
    JDsp::VectorMovingAverage<int> ma=JDsp::VectorMovingAverage<int>(QPair<int,int>(4,5));
    LONGS_EQUAL(4,ma.getSize().first);
    LONGS_EQUAL(5,ma.getSize().second);
    JDsp::VectorMovingAverage<int> mb;
    LONGS_EQUAL(0,mb.getSize().first);
    LONGS_EQUAL(0,mb.getSize().second);
    JDsp::VectorMovingAverage<int> mc(ma);
    LONGS_EQUAL(4,mc.getSize().first);
    LONGS_EQUAL(5,mc.getSize().second);
}

TEST(Test_DSP_VectorMovingAverage, CreateDouble)
{
    //create 3 ways
    JDsp::VectorMovingAverage<double> ma=JDsp::VectorMovingAverage<double>(QPair<int,int>(4,5));
    LONGS_EQUAL(4,ma.getSize().first);
    LONGS_EQUAL(5,ma.getSize().second);
    JDsp::VectorMovingAverage<double> mb;
    LONGS_EQUAL(0,mb.getSize().first);
    LONGS_EQUAL(0,mb.getSize().second);
    JDsp::VectorMovingAverage<double> mc(ma);
    LONGS_EQUAL(4,mc.getSize().first);
    LONGS_EQUAL(5,mc.getSize().second);
}




