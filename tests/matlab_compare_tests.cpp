#include "../src/dsp/dsp.h"
#include "../src/util/RuntimeError.h"
#include "../src/util/stdio_utils.h"
#include "../src/util/file_utils.h"
#include <QFile>
#include <QDataStream>

//important for Qt include cpputest last as it mucks up new and causes compiling to fail
#include "CppUTest/TestHarness.h"


//this unit test is the big one and tests the C++ algo implimentation with
//that of matlab. the output signal and snr are compared


TEST_GROUP(Test_MatlabCompare)
{
    const double double_tolerance_for_db_snr=0.13;
    const double double_tolerance_for_output_signal=0.05;

    //cpputest does not seem to work for qt accessing io such as qfile qdebug etc
    //i get momory leak messages so im turning it off for this unit where i really want qfile
    void setup()
    {
        MemoryLeakWarningPlugin::turnOffNewDeleteOverloads();
    }

    void teardown()
    {
        MemoryLeakWarningPlugin::turnOnNewDeleteOverloads();
    }
};

TEST(Test_MatlabCompare, SignalOutputTest)
{

    //algo things
    //these settings have to match exactly with the create.m Matlab script
    JDsp::OverlappedRealFFT fft(128);
    JDsp::MovingNoiseEstimator mne(fft.getOutSize(),16,16,62);
    JDsp::MovingSignalEstimator mse(fft.getOutSize(),8,3,96,62,8);
    JDsp::InverseOverlappedRealFFT ifft(fft.getInSize());
    JDsp::OverlappedRealFFTDelayLine fft_delay1(fft,51);
    JDsp::OverlappedRealFFTDelayLine fft_delay2(fft,6);
    QVector<double> x(fft.getInSize());

    //load test file
    QFile file(QString(MATLAB_PATH)+"/test_signal_time.raw");
    CHECK(file.open(QIODevice::ReadOnly));
    QDataStream datastream(&file);
    datastream.setByteOrder(QDataStream::LittleEndian);
    datastream.setFloatingPointPrecision (QDataStream::SinglePrecision);

    //what we want from the algo output
    QVector<double> actual_output_signal;

    //run algo over file
    while(!file.atEnd())
    {
        //read some audio
        for(int i=0;i<x.size();i++)
        {
            if(file.atEnd())break;
            datastream>>x[i];
        }
        if(file.atEnd())break;

        //do algo
        fft<<x;//signal in
        mne<<fft;//output of fft to input of mne
        fft_delay1.delay(fft);//1st delayline to match matlab
        fft/=mne;//normalize output of fft
        mse<<fft;//output of fft into mse
        fft_delay2.delay(fft);//2nd delayline to match matlab
        fft*=mse;//scale fft output by signal estimate
        ifft<<fft;//output of fft to inverse fft to produce reconstuted signal

        //keep output signal
        actual_output_signal+=ifft;
    }
    file.close();

    //save actual output signal to disk in the matlab folder for use in matlab
    File_Utils::Matlab::print((QString(MATLAB_PATH)+"/actual_output_signal_include.m").toLocal8Bit().data(),"actual_output_signal",actual_output_signal);

    //load expected_snr_estimate_db_signal file
    file.setFileName(QString(MATLAB_PATH)+"/expected_output_signal.raw");
    CHECK(file.open(QIODevice::ReadOnly));

    //compare the two
    int ncompares=0;
    for(int k=(61*257+0-252-1);(k<actual_output_signal.size())&&(!file.atEnd());k++)
    {
        double expected;
        datastream>>expected;
        ncompares++;
        DOUBLES_EQUAL(expected,actual_output_signal[k],double_tolerance_for_output_signal);
    }
    CHECK(ncompares>0);

    file.close();

}

TEST(Test_MatlabCompare, SNRTest)
{

    //algo things
    //these settings have to match exactly with the create.m Matlab script
    JDsp::OverlappedRealFFT fft(128);
    JDsp::MovingNoiseEstimator mne(fft.getOutSize(),16,16,62);
    JDsp::MovingSignalEstimator mse(fft.getOutSize(),8,3,96,62,8);
    JDsp::InverseOverlappedRealFFT ifft(fft.getInSize());
    JDsp::OverlappedRealFFTDelayLine fft_delay1(fft,51);
    JDsp::OverlappedRealFFTDelayLine fft_delay2(fft,6);
    QVector<double> x(fft.getInSize());

    //load test file
    QFile file(QString(MATLAB_PATH)+"/test_signal_time.raw");
    CHECK(file.open(QIODevice::ReadOnly));
    QDataStream datastream(&file);
    datastream.setByteOrder(QDataStream::LittleEndian);
    datastream.setFloatingPointPrecision (QDataStream::SinglePrecision);

    //what we want from the algo output
    QVector<double> actual_snr_estimate_db_signal;

    //run algo over file
    while(!file.atEnd())
    {
        //read some audio
        for(int i=0;i<x.size();i++)
        {
            if(file.atEnd())break;
            datastream>>x[i];
        }
        if(file.atEnd())break;

        //do algo
        fft<<x;//signal in
        mne<<fft;//output of fft to input of mne
        fft_delay1.delay(fft);//1st delayline to match matlab
        fft/=mne;//normalize output of fft
        mse<<fft;//output of fft into mse
        fft_delay2.delay(fft);//2nd delayline to match matlab
        fft*=mse;//scale fft output by signal estimate
        ifft<<fft;//output of fft to inverse fft to produce reconstuted signal

        //keep snr db
        actual_snr_estimate_db_signal+=10.0*log10(mse.voice_snr_estimate);
    }
    file.close();

    //save actual snr estimate db signal to disk in the matlab folder for use in matlab
    File_Utils::Matlab::print((QString(MATLAB_PATH)+"/actual_snr_estimate_db_signal_include.m").toLocal8Bit().data(),"actual_snr_estimate_db_signal",actual_snr_estimate_db_signal);

    //load expected_snr_estimate_db_signal file
    file.setFileName(QString(MATLAB_PATH)+"/expected_snr_estimate_db_signal.raw");
    CHECK(file.open(QIODevice::ReadOnly));

    //compare the two
    int ncompares=0;
    for(int k=85;(k<actual_snr_estimate_db_signal.size())&&(!file.atEnd());k++)
    {
        double expected;
        datastream>>expected;
        if(k<150)continue;
        ncompares++;
        DOUBLES_EQUAL(expected,actual_snr_estimate_db_signal[k],double_tolerance_for_db_snr);
    }
    CHECK(ncompares>0);

    file.close();

}
