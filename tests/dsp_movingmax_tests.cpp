#include "../src/dsp/dsp.h"
#include "../src/util/RuntimeError.h"

//important for Qt include cpputest last as it mucks up new and causes compiling to fail
#include "CppUTest/TestHarness.h"

TEST_GROUP(Test_DSP_MovingMax)
{

    void setup()
    {
        MemoryLeakWarningPlugin::turnOffNewDeleteOverloads();
    }

    void teardown()
    {
        MemoryLeakWarningPlugin::turnOnNewDeleteOverloads();
    }
};

TEST(Test_DSP_MovingMax, ResetOnChangeSizeDouble)
{
    JDsp::MovingMax<double> mv=JDsp::MovingMax<double>(10);
    QVector<double> input={1,2,3,4};
    for(int k=0;k<10;k++)
    {
        mv<<input;
        for(int i=0;i<4;i++)input[i]+=1;
    }
    mv.setSize(20);
    DOUBLES_EQUAL(0,mv,0.0000000000001);
}

TEST(Test_DSP_MovingMax, SmallMaxWithNanDouble)
{
    JDsp::MovingMax<double> mv=JDsp::MovingMax<double>(3);
    QVector<double> input={1,2,3,4};
    for(int k=0;k<3;k++)mv<<input;
    input[0]=std::numeric_limits<double>::quiet_NaN();
    for(int k=0;k<4;k++)
    {
        mv<<input;
        for(int i=1;i<4;i++)input[i]+=1;
        if(k==2)input[0]=2;
    }

    DOUBLES_EQUAL(7,mv,0.0001);
}

TEST(Test_DSP_MovingMax, SmallMaxDouble)
{
    JDsp::MovingMax<double> mv=JDsp::MovingMax<double>(3);
    QVector<double> input={1,2,3,4};
    for(int k=0;k<4;k++)
    {
        for(int i=0;i<3;i++)mv<<input[i];
        if(k<3)for(int i=0;i<4;i++)input[i]+=1;
    }
    DOUBLES_EQUAL(6,mv,0.0001);
}

TEST(Test_DSP_MovingMax, OutOfBoundsDouble)
{
    try
    {
        JDsp::MovingMax<double> mv=JDsp::MovingMax<double>(-1);
    }
    catch (const RuntimeError&)
    {
        return;
    }
    FAIL("MovingMax did not throw error");
}

TEST(Test_DSP_MovingMax, ZeroMaxDouble)
{
    JDsp::MovingMax<double> mv=JDsp::MovingMax<double>(0);
    QVector<double> input={1,4,3,2};
    for(int k=0;k<9;k++)mv.update(input);

    for(int k=0;k<9;k++)
    {
        for(int i=0;i<4;i++)mv<<input[i];
    }
    DOUBLES_EQUAL(2,mv,0.0000000000001);
}

TEST(Test_DSP_MovingMax, ZeroMaxVectorDouble)
{
    JDsp::MovingMax<double> mv=JDsp::MovingMax<double>(0);
    QVector<double> input={1,4,3,2};
    for(int k=0;k<9;k++)mv.update(input);
    DOUBLES_EQUAL(4,mv,0.0000000000001);
}


