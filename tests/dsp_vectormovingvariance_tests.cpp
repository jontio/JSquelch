#include "../src/dsp/dsp.h"
#include "../src/util/RuntimeError.h"
#include <complex>

//important for Qt include cpputest last as it mucks up new and causes compiling to fail
#include "CppUTest/TestHarness.h"


TEST_GROUP(Test_DSP_VectorMovingVariance)
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

TEST(Test_DSP_VectorMovingVariance, MultipleQuantizationResettingDouble)
{
    //make quantization bad 7 rounds
    int mv_sz=500;
    JDsp::VectorMovingVariance<double> mv=JDsp::VectorMovingVariance<double>(QPair<int,int>(2,mv_sz));
    for(int k=0;k<7*mv_sz;k++)
    {
        double f1 = 2.0*((double)rand() / RAND_MAX)-1.0;
        f1*=100000000000000000.0;
        double f2 = 2.0*((double)rand() / RAND_MAX)-1.0;
        f2*=100000000000000000.0;
        QVector<double> input={f1,f2};
        mv<<input;
    }

    //quantization should be removed after mv_sz updates
    for(int k=0;k<mv_sz;k++)
    {
        double f1 = 2.0*((double)rand() / RAND_MAX)-1.0;
        double f2 = 4.0*(2.0*((double)rand() / RAND_MAX)-1.0);
        QVector<double> input={f1,f2};
        mv<<input;
    }
    DOUBLES_EQUAL(4.0/12.0,mv.val[0],0.02);
    DOUBLES_EQUAL(64.0/12.0,mv.val[1],0.2);
}

TEST(Test_DSP_VectorMovingVariance, OutOfBoundsDouble)
{
    try
    {
        JDsp::VectorMovingVariance<double> mv=JDsp::VectorMovingVariance<double>(QPair<int,int>(-10,10));
    }
    catch (const RuntimeError&)
    {
        return;
    }
    FAIL("VectorMovingVariance did not throw error");
}

TEST(Test_DSP_VectorMovingVariance, ResetOnChangeSizeDouble)
{
    JDsp::VectorMovingVariance<double> mv=JDsp::VectorMovingVariance<double>(QPair<int,int>(4,10));
    QVector<double> input={1,2,3,4};
    for(int k=0;k<10;k++)
    {
        mv.update(input);
        for(int i=0;i<4;i++)input[i]+=1;
    }
    mv.setSize(QPair<int,int>(4,20));
    DOUBLES_EQUAL(0,mv.val[0],0.0000000000001);
    DOUBLES_EQUAL(0,mv.val[1],0.0000000000001);
    DOUBLES_EQUAL(0,mv.val[2],0.0000000000001);
    DOUBLES_EQUAL(0,mv.val[3],0.0000000000001);
}

TEST(Test_DSP_VectorMovingVariance, SmallVarianceDouble)
{
    //must not sum size else resetting is done and test is pointless
    JDsp::VectorMovingVariance<double> mv=JDsp::VectorMovingVariance<double>(QPair<int,int>(4,10));
    QVector<double> input={1,2,3,4};
    for(int k=0;k<9;k++)
    {
        mv<<input;
        for(int i=0;i<4;i++)input[i]+=1;
    }
    DOUBLES_EQUAL(9.1667,mv[0],0.0001);
    DOUBLES_EQUAL(10.2667,mv[1],0.0001);
    DOUBLES_EQUAL(11.5667,mv[2],0.0001);
    DOUBLES_EQUAL(13.0667,mv.val[3],0.0001);

}

TEST(Test_DSP_VectorMovingVariance, ZeroZeroVarianceDouble)
{
    try
    {
        JDsp::VectorMovingVariance<double> mv=JDsp::VectorMovingVariance<double>(QPair<int,int>(0,0));
    }
    catch (const RuntimeError&)
    {
        return;
    }
    FAIL("VectorMovingAverage did not throw error");
}

TEST(Test_DSP_VectorMovingVariance, ZeroVarianceDouble)
{
    JDsp::VectorMovingVariance<double> mv=JDsp::VectorMovingVariance<double>(QPair<int,int>(4,0));
    QVector<double> input={1,2,3,4};
    for(int k=0;k<9;k++)mv<<input;
    DOUBLES_EQUAL(0,mv[0],0.0000000000001);
    DOUBLES_EQUAL(0,mv[1],0.0000000000001);
    DOUBLES_EQUAL(0,mv[2],0.0000000000001);
    DOUBLES_EQUAL(0,mv[3],0.0000000000001);
}

TEST(Test_DSP_VectorMovingVariance, SelfAssignmentDouble)
{
    //create
    JDsp::VectorMovingVariance<double> mv=JDsp::VectorMovingVariance<double>(QPair<int,int>(4,5));
    //self assignment
    mv[2]=3.21;
    mv=mv;
    DOUBLES_EQUAL(3.21,mv[2],0.0000000000001);
    LONGS_EQUAL(4,mv.getSize().first);
    LONGS_EQUAL(5,mv.getSize().second);
}

TEST(Test_DSP_VectorMovingVariance, AssignmentDouble)
{
    //create
    JDsp::VectorMovingVariance<double> mv=JDsp::VectorMovingVariance<double>(QPair<int,int>(4,5));
    //assignment
    JDsp::VectorMovingVariance<double> mv2=JDsp::VectorMovingVariance<double>(mv);
    LONGS_EQUAL(4,mv.getSize().first);
    LONGS_EQUAL(5,mv.getSize().second);
}

TEST(Test_DSP_VectorMovingVariance, CopyAssignmentDouble)
{
    //create
    JDsp::VectorMovingVariance<double> mv=JDsp::VectorMovingVariance<double>(QPair<int,int>(4,5));
    //copy assignment
    mv=JDsp::VectorMovingVariance<double>(QPair<int,int>(7,3));;
    LONGS_EQUAL(7,mv.getSize().first);
    LONGS_EQUAL(3,mv.getSize().second);
}

TEST(Test_DSP_VectorMovingVariance, CreateDouble)
{
    //create 3 ways
    JDsp::VectorMovingVariance<double> mv=JDsp::VectorMovingVariance<double>(QPair<int,int>(4,5));
    LONGS_EQUAL(4,mv.getSize().first);
    LONGS_EQUAL(5,mv.getSize().second);
    JDsp::VectorMovingVariance<double> mb;
    LONGS_EQUAL(0,mb.getSize().first);
    LONGS_EQUAL(0,mb.getSize().second);
    JDsp::VectorMovingVariance<double> mc(mv);
    LONGS_EQUAL(4,mc.getSize().first);
    LONGS_EQUAL(5,mc.getSize().second);
}




