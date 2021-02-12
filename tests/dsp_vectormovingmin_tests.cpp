#include "../src/dsp/dsp.h"
#include "../src/util/RuntimeError.h"
#include <complex>

//important for Qt include cpputest last as it mucks up new and causes compiling to fail
#include "CppUTest/TestHarness.h"


TEST_GROUP(Test_DSP_VectorMovingMin)
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

TEST(Test_DSP_VectorMovingMin, BigTestDouble)
{
    class TestVectorMovingMin : public JDsp::VectorMovingMin<double>
    {
    public:
        TestVectorMovingMin(const QPair<int,int> mn):
            JDsp::VectorMovingMin<double>(mn){}
        inline const QVector<QVector<double>> &getMv() {return JDsp::VectorMovingMin<double>::getMv();}
    };
    int mv_sz=50;
    TestVectorMovingMin mv=TestVectorMovingMin(QPair<int,int>(2,mv_sz));
    for(int k=0;k<7*mv_sz-2;k++)
    {
        double f1 = 2.0*((double)rand() / RAND_MAX)-1.0;
        f1*=100000000000000000.0;
        double f2 = 2.0*((double)rand() / RAND_MAX)-1.0;
        f2*=100000000000000000.0;
        QVector<double> input={f1,f2};
        mv<<input;
    }

    for(int k=0;k<mv_sz;k++)
    {
        double f1 = 2.0*((double)rand() / RAND_MAX)-1.0;
        double f2 = 4.0*(2.0*((double)rand() / RAND_MAX)-1.0);
        QVector<double> input={f1,f2};
        mv<<input;
    }

    DOUBLES_EQUAL(*std::min_element(mv.getMv()[0].begin(),mv.getMv()[0].end()),mv[0],0.0000000000001);
    DOUBLES_EQUAL(*std::min_element(mv.getMv()[1].begin(),mv.getMv()[1].end()),mv[1],0.0000000000001);
}

TEST(Test_DSP_VectorMovingMin, OutOfBoundsDouble)
{
    try
    {
        JDsp::VectorMovingMin<double> mv=JDsp::VectorMovingMin<double>(QPair<int,int>(-10,10));
    }
    catch (const RuntimeError&)
    {
        return;
    }
    FAIL("VectorMovingMin did not throw error");
}

TEST(Test_DSP_VectorMovingMin, ResetOnChangeSizeDouble)
{
    JDsp::VectorMovingMin<double> mv=JDsp::VectorMovingMin<double>(QPair<int,int>(4,10));
    QVector<double> input={1,2,3,4};
    for(int k=0;k<10;k++)
    {
        mv.update(input);
        for(int i=0;i<4;i++)input[i]+=1;
    }
    mv.setSize(QPair<int,int>(4,20));
    DOUBLES_EQUAL(0,mv[0],0.0000000000001);
    DOUBLES_EQUAL(0,mv[1],0.0000000000001);
    DOUBLES_EQUAL(0,mv[2],0.0000000000001);
    DOUBLES_EQUAL(0,mv[3],0.0000000000001);
}

TEST(Test_DSP_VectorMovingMin, SmallMinDouble)
{
    JDsp::VectorMovingMin<double> mv=JDsp::VectorMovingMin<double>(QPair<int,int>(4,3));
    QVector<double> input={1,2,3,4};
    for(int k=0;k<4;k++)
    {
        mv<<input;
        for(int i=0;i<4;i++)input[i]+=1;
    }

    DOUBLES_EQUAL(2,mv[0],0.0001);
    DOUBLES_EQUAL(3,mv[1],0.0001);
    DOUBLES_EQUAL(4,mv[2],0.0001);
    DOUBLES_EQUAL(5,mv.getMin()[3],0.0001);
}

//this should fail to compile if uncommented
//TEST(Test_DSP_VectorMovingMin, NoModifyDouble)
//{
//    JDsp::VectorMovingMin<double> mv=JDsp::VectorMovingMin<double>(QPair<int,int>(4,3));
//    mv[1]=55;
//}

TEST(Test_DSP_VectorMovingMin, ZeroZeroMinDouble)
{
    try
    {
        JDsp::VectorMovingMin<double> mv=JDsp::VectorMovingMin<double>(QPair<int,int>(0,0));
    }
    catch (const RuntimeError&)
    {
        return;
    }
    FAIL("VectorMovingMin did not throw error");
}

TEST(Test_DSP_VectorMovingMin, ZeroMinDouble)
{
    JDsp::VectorMovingMin<double> mv=JDsp::VectorMovingMin<double>(QPair<int,int>(4,0));
    QVector<double> input={1,2,3,4};
    for(int k=0;k<9;k++)mv.update(input);
    DOUBLES_EQUAL(1,mv[0],0.0000000000001);
    DOUBLES_EQUAL(2,mv[1],0.0000000000001);
    DOUBLES_EQUAL(3,mv[2],0.0000000000001);
    DOUBLES_EQUAL(4,mv[3],0.0000000000001);
}

TEST(Test_DSP_VectorMovingMin, SelfAssignmentDouble)
{
    //create
    JDsp::VectorMovingMin<double> mv=JDsp::VectorMovingMin<double>(QPair<int,int>(4,5));
    //self assignment
    mv=mv;
    DOUBLES_EQUAL(0,mv[2],0.0000000000001);
    LONGS_EQUAL(4,mv.getSize().first);
    LONGS_EQUAL(5,mv.getSize().second);
}

TEST(Test_DSP_VectorMovingMin, AssignmentDouble)
{
    //create
    JDsp::VectorMovingMin<double> mv=JDsp::VectorMovingMin<double>(QPair<int,int>(4,5));
    //assignment
    JDsp::VectorMovingMin<double> mv2=JDsp::VectorMovingMin<double>(mv);
    LONGS_EQUAL(4,mv.getSize().first);
    LONGS_EQUAL(5,mv.getSize().second);
}

TEST(Test_DSP_VectorMovingMin, CopyAssignmentDouble)
{
    //create
    JDsp::VectorMovingMin<double> mv=JDsp::VectorMovingMin<double>(QPair<int,int>(4,5));
    //copy assignment
    mv=JDsp::VectorMovingMin<double>(QPair<int,int>(7,3));;
    LONGS_EQUAL(7,mv.getSize().first);
    LONGS_EQUAL(3,mv.getSize().second);
}

TEST(Test_DSP_VectorMovingMin, CreateDouble)
{
    //create 3 ways
    JDsp::VectorMovingMin<double> mv=JDsp::VectorMovingMin<double>(QPair<int,int>(4,5));
    LONGS_EQUAL(4,mv.getSize().first);
    LONGS_EQUAL(5,mv.getSize().second);
    JDsp::VectorMovingMin<double> mb;
    LONGS_EQUAL(0,mb.getSize().first);
    LONGS_EQUAL(0,mb.getSize().second);
    JDsp::VectorMovingMin<double> mc(mv);
    LONGS_EQUAL(4,mc.getSize().first);
    LONGS_EQUAL(5,mc.getSize().second);
}




