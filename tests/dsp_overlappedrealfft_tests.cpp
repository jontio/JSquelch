#include "../src/dsp/dsp.h"
#include "../src/util/RuntimeError.h"

//important for Qt include cpputest last as it mucks up new and causes compiling to fail
#include "CppUTest/TestHarness.h"


TEST_GROUP(Test_DSP_OverlappedRealFFT)
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


TEST(Test_DSP_OverlappedRealFFT, BufferShiftingWindowWithPadding)
{

//    scale=1;
//    b=[1,2,3,4,5,6,7,8]';
//    xb=b.*hann(8);
//    xb=[xb;zeros(numel(xb),1)];
//    Xfull=fft(xb)./scale
//    Xabs=abs(Xfull(1:numel(Xfull)/2+1))

    JDsp::OverlappedRealFFT offt;
    offt.setSize(4);
    QVector<double> x={42,64,23,1.2};
    offt.update(x);
    x={1,2,3,4};
    offt.update(x);
    x={5,6,7,8};
    offt.update(x);

    QVector<double> expected_real_Xfull={1.575000e+01,7.641372e-01,-9.767924e+00,-3.449261e-01,1.600855e+00,-3.848020e-01,2.630794e-01,-3.440906e-02,5.797853e-02,-3.440906e-02,2.630794e-01,-3.848020e-01,1.600855e+00,-3.449261e-01,-9.767924e+00,7.641372e-01};
    QVector<double> expected_imag_Xfull={0,-1.402592e+01,-8.772460e-01,5.034532e+00,-2.421353e-01,-1.332367e-02,1.547454e-01,-6.408625e-02,0,6.408625e-02,-1.547454e-01,1.332367e-02,2.421353e-01,-5.034532e+00,8.772460e-01,1.402592e+01};

    for(int k=0;k<16;k++)
    {
        DOUBLES_EQUAL(expected_real_Xfull[k],offt.Xfull[k].real(),0.000001);
        DOUBLES_EQUAL(expected_imag_Xfull[k],offt.Xfull[k].imag(),0.000001);
    }

    QVector<double> expected_abs={15.7500,  14.0467, 9.8072, 5.0463,1.6191, 0.3850,0.3052, 0.0727,0.0580};
    for(int k=0;k<9;k++)
    {
        DOUBLES_EQUAL(expected_abs[k],offt.Xabs[k],0.0001);
    }

    x={1,2,3};
    offt.setSize(3);
    offt.update(x);

    x={1};
    offt.setSize(1);
    offt.update(x);

    offt.setSize(0);
}

TEST(Test_DSP_OverlappedRealFFT, SmallRealFFT)
{
    JFFT fft;
    QVector<double> x={5,6,4,8};
    QVector<JFFT::cpx_type> XB;
    fft.fft_real(x,XB);
    QVector<double> expected_real={23,1,-5,1};
    QVector<double> expected_imag={0,2,0,-2};
    for(int k=0;k<4;k++)
    {
        DOUBLES_EQUAL(expected_real[k],XB[k].real(),0.000001);
        DOUBLES_EQUAL(expected_imag[k],XB[k].imag(),0.000001);
    }

}
