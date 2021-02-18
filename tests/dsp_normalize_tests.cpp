#include "../src/dsp/dsp.h"
#include "../src/util/RuntimeError.h"
#include "../src/util/stdio_utils.h"
#include <random>

//important for Qt include cpputest last as it mucks up new and causes compiling to fail
#include "CppUTest/TestHarness.h"


TEST_GROUP(Test_DSP_Normalize)
{
    double double_tolerance=0.00001;

    //from c++
    const QVector<JFFT::cpx_type> gXfull_expected={JFFT::cpx_type(0.330387,0.000000),JFFT::cpx_type(0.180771,-0.269594),JFFT::cpx_type(-0.038094,-0.444241),JFFT::cpx_type(-0.690763,-0.415689),JFFT::cpx_type(-1.002823,0.774180),JFFT::cpx_type(0.591772,1.451755),JFFT::cpx_type(1.595687,-0.297248),JFFT::cpx_type(-0.088841,-1.540616),JFFT::cpx_type(-1.478622,0.000000),JFFT::cpx_type(-0.088841,1.540616),JFFT::cpx_type(1.595687,0.297248),JFFT::cpx_type(0.591772,-1.451755),JFFT::cpx_type(-1.002823,-0.774180),JFFT::cpx_type(-0.690763,0.415689),JFFT::cpx_type(-0.038094,0.444241),JFFT::cpx_type(0.180771,0.269594)};
    const QVector<JFFT::cpx_type> gXfull={JFFT::cpx_type(4.698124,0.000000),JFFT::cpx_type(2.592814,-3.866813),JFFT::cpx_type(-0.550234,-6.416625),JFFT::cpx_type(-10.356980,-6.232650),JFFT::cpx_type(-15.179227,11.718367),JFFT::cpx_type(9.284401,22.776800),JFFT::cpx_type(26.302901,-4.899769),JFFT::cpx_type(-1.520235,-26.362698),JFFT::cpx_type(-25.845006,0.000000),JFFT::cpx_type(-1.520235,26.362698),JFFT::cpx_type(26.302901,4.899769),JFFT::cpx_type(9.284401,-22.776800),JFFT::cpx_type(-15.179227,-11.718367),JFFT::cpx_type(-10.356980,6.232650),JFFT::cpx_type(-0.550234,6.416625),JFFT::cpx_type(2.592814,3.866813)};
    const QVector<double> gsome_noise_estimate={14.220066,14.343108,14.444011,14.993540,15.136496,15.689145,16.483747,17.111796,17.479121};
    const QVector<double> gXabs_expected={0.330387,0.324590,0.445872,0.806195,1.266889,1.567733,1.623137,1.543175,1.478622};
    const QVector<double> gXabs={4.698124,4.655634,6.440173,12.087719,19.176263,24.596397,26.755380,26.406495,25.845006};
    //from matlab done by flipping some_noise_estimate and appending after trimming both ends
    const QVector<double> gsome_noise_estimate_from_matlab={14.2201,14.3431,14.4440,14.9935,15.1365,15.6891,16.4837,17.1118,17.4791,17.1118,16.4837,15.6891,15.1365,14.9935,14.4440,14.3431};

    //for 1 bin ,3 moving stats window,3 moving min window,4 moving final window
    class TestMovingNoiseEstimator : public JDsp::MovingNoiseEstimator
    {
    public:
        TestMovingNoiseEstimator(): JDsp::MovingNoiseEstimator(1,3,3,4){}
        inline JDsp::VectorMovingMinWithAssociate<double> &getmm(){return JDsp::MovingNoiseEstimator::mm;}
        inline JDsp::VectorMovingVariance<double> &getmv(){return JDsp::MovingNoiseEstimator::mv;};
    };

    std::default_random_engine generator;
    std::normal_distribution<double> distribution;

    void reset_random_generation()
    {
        generator=std::default_random_engine(1);
        distribution=std::normal_distribution<double>(0.0,10.0);
    }

    void setup()
    {
        reset_random_generation();
    }

    void teardown()
    {
        // This gets run after every test
    }
};

TEST(Test_DSP_Normalize, NormalizationTest)
{
    JDsp::OverlappedRealFFT fft(4);//for an fft that takes 128 as in will output 2*3+1=257
    QVector<double> x(fft.getInSize());//this would be the input signal split up into blocks
    JDsp::MovingNoiseEstimator mne(fft.getOutSize(),16,16,32);
    //code used to create vectors
//    for(int k=0;k<640;k++)
//    {
//        for(int i=0;i<x.size();i++)x[i]=distribution(generator);
//        fft<<x;//put time domain signal into the fft
//        mne<<fft;//put output from fft into the noise estimator
//        fft/=mne;//normalize the fft output by the noise estimate
//    }
//    QVector<JFFT::cpx_type> Xfull_expected=fft.Xfull;
//    QVector<double> Xabs_expected=fft.Xabs;
//    reset_random_generation();
//    for(int k=0;k<640;k++)
//    {
//        for(int i=0;i<x.size();i++)x[i]=distribution(generator);
//        fft<<x;//put time domain signal into the fft
//        mne<<fft;//put output from fft into the noise estimator
//    }
//    QVector<JFFT::cpx_type> Xfull=fft.Xfull;
//    QVector<double> Xabs=fft.Xabs;
//    Stdio_Utils::CPP::print("gXfull_expected",Xfull_expected);
//    Stdio_Utils::CPP::print("gXfull",Xfull);
//    print_real_cpp("gXabs_expected",Xabs_expected);
//    print_real_cpp("gXabs",Xabs);
//    print_real_cpp("gsome_noise_estimate",mne.val);

    //load some of the state of fft and mne
    fft.Xfull=gXfull;
    fft.Xabs=gXabs;
    mne.val=gsome_noise_estimate;
    //do the normalization
    fft/=mne;
    //check it is as expected
    for(int k=0;k<fft.Xfull.size();k++)
    {
        //check fft/=mne still works as intended for Xfull
        DOUBLES_EQUAL(fft.Xfull[k].real(),gXfull_expected[k].real(),double_tolerance);
        DOUBLES_EQUAL(fft.Xfull[k].imag(),gXfull_expected[k].imag(),double_tolerance);

        //check the divide trim and flipping works assuming matlab dpes this right
        DOUBLES_EQUAL(fft.Xfull[k].real(),gXfull[k].real()/gsome_noise_estimate_from_matlab[k],double_tolerance);
        DOUBLES_EQUAL(fft.Xfull[k].imag(),gXfull[k].imag()/gsome_noise_estimate_from_matlab[k],double_tolerance);

    }
    for(int k=0;k<fft.Xabs.size();k++)
    {
        //check fft/=mne still works as intended for Xabs
        DOUBLES_EQUAL(fft.Xabs[k],gXabs_expected[k],double_tolerance);
        //check the divide
        DOUBLES_EQUAL(fft.Xabs[k],gXabs[k]/gsome_noise_estimate[k],double_tolerance);
    }

}
