#include "../src/dsp/dsp.h"
#include "../src/util/RuntimeError.h"
#include <complex>

//important for Qt include cpputest last as it mucks up new and causes compiling to fail
#include "CppUTest/TestHarness.h"


TEST_GROUP(Test_DSP_VectorMovingMax)
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

TEST(Test_DSP_VectorMovingMax, BigTestDouble)
{
    class TestVectorMovingMax : public JDsp::VectorMovingMax<double>
    {
    public:
        TestVectorMovingMax(const QPair<int,int> mn):
            JDsp::VectorMovingMax<double>(mn){}
        inline const QVector<QVector<double>> &getMv() {return JDsp::VectorMovingMax<double>::getMv();}
    };
    int mv_sz=50;
    TestVectorMovingMax mv=TestVectorMovingMax(QPair<int,int>(2,mv_sz));
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

    DOUBLES_EQUAL(*std::max_element(mv.getMv()[0].begin(),mv.getMv()[0].end()),mv[0],0.0000000000001);
    DOUBLES_EQUAL(*std::max_element(mv.getMv()[1].begin(),mv.getMv()[1].end()),mv[1],0.0000000000001);
}

TEST(Test_DSP_VectorMovingMax, OutOfBoundsDouble)
{
    try
    {
        JDsp::VectorMovingMax<double> mv=JDsp::VectorMovingMax<double>(QPair<int,int>(-10,10));
    }
    catch (const RuntimeError&)
    {
        return;
    }
    FAIL("VectorMovingMax did not throw error");
}

TEST(Test_DSP_VectorMovingMax, ResetOnChangeSizeDouble)
{
    JDsp::VectorMovingMax<double> mv=JDsp::VectorMovingMax<double>(QPair<int,int>(4,10));
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

TEST(Test_DSP_VectorMovingMax, SmallMaxDouble)
{
    JDsp::VectorMovingMax<double> mv=JDsp::VectorMovingMax<double>(QPair<int,int>(4,10));
    QVector<double> input={1,2,3,4};
    for(int k=0;k<9;k++)
    {
        mv<<input;
        for(int i=0;i<4;i++)input[i]+=1;
    }
    DOUBLES_EQUAL(9,mv[0],0.0001);
    DOUBLES_EQUAL(10,mv[1],0.0001);
    DOUBLES_EQUAL(11,mv[2],0.0001);
    DOUBLES_EQUAL(12,mv.getMax()[3],0.0001);
}

//this should fail to compile if uncommented
//TEST(Test_DSP_VectorMovingMax, NoModifyDouble)
//{
//    JDsp::VectorMovingMax<double> mv=JDsp::VectorMovingMax<double>(QPair<int,int>(4,3));
//    mv[1]=55;
//}

TEST(Test_DSP_VectorMovingMax, ZeroZeroMaxDouble)
{
    try
    {
        JDsp::VectorMovingMax<double> mv=JDsp::VectorMovingMax<double>(QPair<int,int>(0,0));
    }
    catch (const RuntimeError&)
    {
        return;
    }
    FAIL("VectorMovingMax did not throw error");
}

TEST(Test_DSP_VectorMovingMax, ZeroMaxDouble)
{
    JDsp::VectorMovingMax<double> mv=JDsp::VectorMovingMax<double>(QPair<int,int>(4,0));
    QVector<double> input={1,2,3,4};
    for(int k=0;k<9;k++)mv.update(input);
    DOUBLES_EQUAL(1,mv[0],0.0000000000001);
    DOUBLES_EQUAL(2,mv[1],0.0000000000001);
    DOUBLES_EQUAL(3,mv[2],0.0000000000001);
    DOUBLES_EQUAL(4,mv[3],0.0000000000001);
}

TEST(Test_DSP_VectorMovingMax, SelfAssignmentDouble)
{
    //create
    JDsp::VectorMovingMax<double> mv=JDsp::VectorMovingMax<double>(QPair<int,int>(4,5));
    //self assignment
    mv=mv;
    DOUBLES_EQUAL(0,mv[2],0.0000000000001);
    LONGS_EQUAL(4,mv.getSize().first);
    LONGS_EQUAL(5,mv.getSize().second);
}

TEST(Test_DSP_VectorMovingMax, AssignmentDouble)
{
    //create
    JDsp::VectorMovingMax<double> mv=JDsp::VectorMovingMax<double>(QPair<int,int>(4,5));
    //assignment
    JDsp::VectorMovingMax<double> mv2=JDsp::VectorMovingMax<double>(mv);
    LONGS_EQUAL(4,mv.getSize().first);
    LONGS_EQUAL(5,mv.getSize().second);
}

TEST(Test_DSP_VectorMovingMax, CopyAssignmentDouble)
{
    //create
    JDsp::VectorMovingMax<double> mv=JDsp::VectorMovingMax<double>(QPair<int,int>(4,5));
    //copy assignment
    mv=JDsp::VectorMovingMax<double>(QPair<int,int>(7,3));;
    LONGS_EQUAL(7,mv.getSize().first);
    LONGS_EQUAL(3,mv.getSize().second);
}

TEST(Test_DSP_VectorMovingMax, CreateDouble)
{
    //create 3 ways
    JDsp::VectorMovingMax<double> mv=JDsp::VectorMovingMax<double>(QPair<int,int>(4,5));
    LONGS_EQUAL(4,mv.getSize().first);
    LONGS_EQUAL(5,mv.getSize().second);
    JDsp::VectorMovingMax<double> mb;
    LONGS_EQUAL(0,mb.getSize().first);
    LONGS_EQUAL(0,mb.getSize().second);
    JDsp::VectorMovingMax<double> mc(mv);
    LONGS_EQUAL(4,mc.getSize().first);
    LONGS_EQUAL(5,mc.getSize().second);
}




