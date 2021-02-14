#include "../src/dsp/dsp.h"
#include "../src/util/RuntimeError.h"
#include <complex>

//important for Qt include cpputest last as it mucks up new and causes compiling to fail
#include "CppUTest/TestHarness.h"


TEST_GROUP(Test_DSP_VectorMovingMinWithAssociate)
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

TEST(Test_DSP_VectorMovingMinWithAssociate, BigTestDouble)
{
    class TestVectorMovingMinWithAssociate : public JDsp::VectorMovingMinWithAssociate<double>
    {
    public:
        TestVectorMovingMinWithAssociate(const QPair<int,int> mn):
            JDsp::VectorMovingMinWithAssociate<double>(mn){}
        inline const QVector<QVector<double>> &getMv() {return JDsp::VectorMovingMin<double>::getMv();}
    };
    int mv_sz=10;
    TestVectorMovingMinWithAssociate mv=TestVectorMovingMinWithAssociate(QPair<int,int>(2,mv_sz));
    for(int k=0;k<7*mv_sz-2;k++)
    {
        double f1 = 2.0*((double)rand() / RAND_MAX)-1.0;
        f1*=100000000000000000.0;
        double f2 = 2.0*((double)rand() / RAND_MAX)-1.0;
        f2*=100000000000000000.0;
        double a1 = 2.0*((double)rand() / RAND_MAX)-1.0;
        a1*=100000000000000000.0;
        double a2 = 2.0*((double)rand() / RAND_MAX)-1.0;
        a2*=100000000000000000.0;
        QVector<double> input={f1,f2};
        QVector<double> input_associate={a1,a2};
        mv.update(input,input_associate);
    }

    double ma1=100000000000;
    double ma2=100000000000;
    double mv1=100000000000;
    double mv2=100000000000;
    for(int k=0;k<mv_sz;k++)
    {
        double f1 = 2.0*((double)rand() / RAND_MAX)-1.0;
        double f2 = 2.0*((double)rand() / RAND_MAX)-1.0;
        double a1 = 2.0*((double)rand() / RAND_MAX)-1.0;
        double a2 = 2.0*((double)rand() / RAND_MAX)-1.0;
        QVector<double> input={f1,f2};
        QVector<double> input_associate={a1,a2};
        mv.update(input,input_associate);
        if(f1<mv1)
        {
            mv1=f1;
            ma1=a1;
        }
        if(f2<mv2)
        {
            mv2=f2;
            ma2=a2;
        }
    }

    //check associates are as expected
    DOUBLES_EQUAL(mv1,mv.getMin()[0],0.000000000000000001);
    DOUBLES_EQUAL(mv2,mv.getMin()[1],0.000000000000000001);
    DOUBLES_EQUAL(ma1,mv.getAssociate()[0],0.000000000000000001);
    DOUBLES_EQUAL(ma2,mv.getAssociate()[1],0.000000000000000001);

    //check output index is as expected
    int expected_index=std::min_element(mv.getMv()[0].begin(),mv.getMv()[0].end())-mv.getMv()[0].begin();
    LONGS_EQUAL(expected_index,mv.getMinIndex()[0]);
    expected_index=std::min_element(mv.getMv()[1].begin(),mv.getMv()[1].end())-mv.getMv()[1].begin();
    LONGS_EQUAL(expected_index,mv.getMinIndex()[1]);

    //check the values of the elements
    DOUBLES_EQUAL(*std::min_element(mv.getMv()[0].begin(),mv.getMv()[0].end()),mv[0],0.0000000000001);
    DOUBLES_EQUAL(*std::min_element(mv.getMv()[1].begin(),mv.getMv()[1].end()),mv[1],0.0000000000001);
}
