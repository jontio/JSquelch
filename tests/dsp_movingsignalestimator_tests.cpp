#include "../src/dsp/dsp.h"
#include "../src/util/RuntimeError.h"
#include "../src/util/stdio_utils.h"
#include "test_snr_estimation_input_257_by_100.h"
#include "test_snr_estimation_output_db_1_by_100.h"
#include "test_snr_estimation_output_lin_snrs_257_by_100.h"

//important for Qt include cpputest last as it mucks up new and causes compiling to fail
#include "CppUTest/TestHarness.h"

TEST_GROUP(Test_DSP_MovingSignalEstimator)
{
    double double_tolerance=0.00001;

    void setup()
    {
        srand(1);


    }

    void teardown()
    {
        // This gets run after every test
    }
};

TEST(Test_DSP_MovingSignalEstimator, BinSNRTest)
{

    JDsp::MovingSignalEstimator mse(257,8,3,96,62,8);//4 in matlab becomes 3 here due to matlab's indexing
    QVector<double> x(257);
    QVector<double> snr_estimation_output_lin_snrs(test_snr_estimation_output_lin_snrs_257_by_100.size());
    int index_in=0;
    int index_out=0;
    for(int k=0;k<100;k++)
    {
        //fill x with what Matlab has here for abs(XBs')
//        %produce lin_snrs a moving snr estimate of the speech
//        %snr_estimate is just for plotting and signal decision
//        ZstdFast=movstd(abs(XBs'),8);%needs to be small
//        ZmeanFast=movmean(abs(XBs'),8);%needs to be small
//        mu_est=((ZmeanFast.^2+ZstdFast.^2-1))/1;
//        snr_estimate=[];
//        lin_snrs=XBs;
//        for k=1:size(ZmeanFast,1)
//            mu=mu_est(k,:);%max((ZmeanFast(k,:).^2+ZstdFast(k,:).^2-1),0)/1;%mu estimate
//            lin_snrs(:,k)=mu';
//            snr_estimate(end+1)=max(mu(4:96));%only bother about frequencies where most voice energy is
//        end
//        lin_snrs=movmean(lin_snrs',8);%reduces white noise but makes it sound echoy and strange
//        snr_estimate=movmean(snr_estimate,62);%1 sec smoothing
//        snr_estimate_db=10*log10(snr_estimate);
        for(int i=0;i<257;i++)x[i]=std::abs(test_snr_estimation_input_257_by_100[index_in++]);
        //now run the class update
        mse<<x;
        //save the bin estimates for later comparison
        for(int i=0;i<257;i++)
        {
            snr_estimation_output_lin_snrs[index_out++]=mse[i];
        }

    }

    //check bin snr matches matlab
    //there is an offset due to the way moving means are calulated; this is to be expected
    for(int k=257*8;k<257*94;k++)
    {
        DOUBLES_EQUAL(test_snr_estimation_output_lin_snrs_257_by_100[k],snr_estimation_output_lin_snrs[257*6+k],double_tolerance);
    }

}

TEST(Test_DSP_MovingSignalEstimator, VoiceSNRTest)
{

    JDsp::MovingSignalEstimator mse(257,8,3,96,62,8);//4 in matlab becomes 3 here due to matlab's indexing
    QVector<double> x(257);
    QVector<double> snr_estimation_output_db(100);
    int index=0;
    for(int k=0;k<100;k++)
    {
        //fill x with what Matlab has here for abs(XBs')
//        %produce lin_snrs a moving snr estimate of the speech
//        %snr_estimate is just for plotting and signal decision
//        ZstdFast=movstd(abs(XBs'),8);%needs to be small
//        ZmeanFast=movmean(abs(XBs'),8);%needs to be small
//        mu_est=((ZmeanFast.^2+ZstdFast.^2-1))/1;
//        snr_estimate=[];
//        lin_snrs=XBs;
//        for k=1:size(ZmeanFast,1)
//            mu=mu_est(k,:);%max((ZmeanFast(k,:).^2+ZstdFast(k,:).^2-1),0)/1;%mu estimate
//            lin_snrs(:,k)=mu';
//            snr_estimate(end+1)=max(mu(4:96));%only bother about frequencies where most voice energy is
//        end
//        lin_snrs=movmean(lin_snrs',8);%reduces white noise but makes it sound echoy and strange
//        snr_estimate=movmean(snr_estimate,62);%1 sec smoothing
//        snr_estimate_db=10*log10(snr_estimate);
        for(int i=0;i<257;i++)x[i]=std::abs(test_snr_estimation_input_257_by_100[index++]);
        //now run the class update
        mse<<x;
        //turn the voise srn into dB
        snr_estimation_output_db[k]=10.0*std::log10(mse.voice_snr_estimate);
    }

//    Stdio_Utils::Matlab::print("snr_estimation_output_db",snr_estimation_output_db);
    //check voice snr matches matlab
    //there is an offset due to the way moving means are calulated; this is to be expected
    for(int k=35;k<67;k++)
    {
//        printf("%f %f\n",test_snr_estimation_output_db_1_by_100[k],snr_estimation_output_db[33+k]);
        DOUBLES_EQUAL(test_snr_estimation_output_db_1_by_100[k],snr_estimation_output_db[33+k],double_tolerance);
    }

}



