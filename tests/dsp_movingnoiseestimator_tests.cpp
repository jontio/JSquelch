#include "../src/dsp/dsp.h"
#include "../src/util/RuntimeError.h"

//important for Qt include cpputest last as it mucks up new and causes compiling to fail
#include "CppUTest/TestHarness.h"


TEST_GROUP(Test_DSP_MovingNoiseEstimator)
{
    double double_tolerance=0.00001;

    //from matlab
    const QVector<double> Z={0.833894,3.223326,0.389431,1.383422,0.555315,1.196348,3.814775,0.044234,2.615933,3.664520,0.815501,1.085864,6.636838,3.163211,1.531994,2.042662,0.673483,3.858356,2.922775,0.496248,0.627097,1.261229,2.835334,2.030545,0.521712,0.451895,0.338598,1.806875,5.031214,0.458211};
    const QVector<double> Zmean={2.028610,1.482217,1.665393,0.776056,1.045028,1.855479,1.685119,2.158314,2.108229,2.365318,1.855295,2.846068,3.628638,3.777348,2.245956,1.416046,2.191500,2.484871,2.425793,1.348707,0.794858,1.574554,2.042370,1.795864,1.001384,0.437402,0.865789,2.392229,2.432100,2.744713};
    const QVector<double> Zstd={1.689584,1.524133,1.437835,0.532493,0.434296,1.726806,1.932205,1.926475,1.862777,1.440948,1.572655,3.285685,2.804603,2.607246,0.834394,0.691914,1.597645,1.636971,1.735277,1.364752,0.409154,1.136973,0.787119,1.174529,0.891963,0.092413,0.816971,2.400446,2.349738,3.233601};
    const QVector<double> Zmin_mean={1.482217,0.776056,0.776056,0.776056,1.045028,1.685119,1.685119,2.108229,1.855295,1.855295,1.855295,2.846068,2.245956,1.416046,1.416046,1.416046,2.191500,1.348707,0.794858,0.794858,0.794858,1.574554,1.001384,0.437402,0.437402,0.437402,0.865789,2.392229,2.432100,2.744713};
    const QVector<double> Zmin_std={1.524133,0.532493,0.532493,0.532493,0.434296,1.932205,1.932205,1.862777,1.572655,1.572655,1.572655,3.285685,0.834394,0.691914,0.691914,0.691914,1.597645,1.364752,0.409154,0.409154,0.409154,1.136973,0.891963,0.092413,0.092413,0.092413,0.816971,2.400446,2.349738,3.233601};
    const QVector<double> moving_sigma_estimate={1.533597,1.336123,1.237386,0.988802,1.394456,1.800111,2.268138,2.593256,2.560346,2.527435,2.910847,2.901795,2.687769,2.473743,1.781022,1.860046,1.945719,1.775202,1.604686,1.150172,1.156024,1.267786,1.156055,1.044323,0.670551,0.632891,1.368361,2.102040,3.050630,3.670709};
    //for 1 bin ,3 moving stats window,3 moving min window,4 moving final window
    class TestMovingNoiseEstimator : public JDsp::MovingNoiseEstimator
    {
    public:
        TestMovingNoiseEstimator(): JDsp::MovingNoiseEstimator(1,3,3,4){}
        inline JDsp::VectorMovingMinWithAssociate<double> &getmm(){return JDsp::MovingNoiseEstimator::mm;}
        inline JDsp::VectorMovingVariance<double> &getmv(){return JDsp::MovingNoiseEstimator::mv;};
    };


    void setup()
    {
        srand(1);


    }

    void teardown()
    {
        // This gets run after every test
    }
};

TEST(Test_DSP_MovingNoiseEstimator, NoiseEstimateTest)
{

    TestMovingNoiseEstimator mne;
    QVector<double> x(1);
    for(int k=0;k<9;k++)
    {
        x={Z[k+1]};
        mne<<x;
        if(k<6)continue;

        //check noise estimate
        DOUBLES_EQUAL(moving_sigma_estimate[k-3],mne[0],double_tolerance);

    }
}

TEST(Test_DSP_MovingNoiseEstimator, MovingMinAndAssociateTest)
{
    TestMovingNoiseEstimator mne;
    QVector<double> x(1);
    for(int k=0;k<9;k++)
    {
        x={Z[k+1]};
        mne<<x;
        if(k<6)continue;

        //check moving min and associate
        DOUBLES_EQUAL(Zmin_mean[k-2],mne.getmm()[0],double_tolerance);
        DOUBLES_EQUAL(Zmin_std[k-2]*Zmin_std[k-2],mne.getmm().getAssociate()[0],double_tolerance);

    }
}

TEST(Test_DSP_MovingNoiseEstimator, MovingStatsTest)
{
    TestMovingNoiseEstimator mne;
    QVector<double> x(1);
    for(int k=0;k<9;k++)
    {
        x={Z[k+1]};
        mne<<x;
        if(k<6)continue;

        //check moving stats
        DOUBLES_EQUAL(Zmean[k],mne.getmv().mean()[0],double_tolerance);
        DOUBLES_EQUAL(Zstd[k]*Zstd[k],mne.getmv()[0],double_tolerance);

    }
}

//matlab code...

//% //c++ test

//short_moving_window_size=3;
//long_moving_window_size=3;

//XBs=XBs(1:1,1:30);

//%double moving windows
//Zstd=movstd(abs(XBs'),short_moving_window_size);
//Zmean=movmean(abs(XBs'),short_moving_window_size);
//Zmin_std=Zstd;
//Zmin_mean=Zmean;
//for k=1:size(Zmean,1)
//    for n=1:size(Zmean,2)
//        [~,idx]=min(Zmean(k:min(k+long_moving_window_size-1,size(Zmean,1)),n));
//        Zmin_mean(k,n)=Zmean(idx+k-1,n);
//        Zmin_std(k,n)=Zstd(idx+k-1,n);
//    end
//end

//%calculate normalizing factor over a window
//%the bigger this window is the less agile the noise estimateion will be
//moving_sigma_estimate=(sqrt((Zmin_std).^2+(Zmin_mean).^2));
//moving_sigma_estimate=movmean(moving_sigma_estimate,4);%62);

//Z=abs(XBs');

//fprintf("\n");

//for k=1:numel(Z)
//    if(k==1)
//        fprintf("QVector<double> Z={");
//    end
//    fprintf("%f",Z(k));
//    if(k<numel(Z))
//        fprintf(",");
//    else
//        fprintf("};\n");
//    end
//end

//for k=1:numel(Z)
//    if(k==1)
//        fprintf("QVector<double> Zmean={");
//    end
//    fprintf("%f",Zmean(k));
//    if(k<numel(Z))
//        fprintf(",");
//    else
//        fprintf("};\n");
//    end
//end

//for k=1:numel(Z)
//    if(k==1)
//        fprintf("QVector<double> Zstd={");
//    end
//    fprintf("%f",Zstd(k));
//    if(k<numel(Z))
//        fprintf(",");
//    else
//        fprintf("};\n");
//    end
//end

//for k=1:numel(Z)
//    if(k==1)
//        fprintf("QVector<double> Zmin_mean={");
//    end
//    fprintf("%f",Zmin_mean(k));
//    if(k<numel(Z))
//        fprintf(",");
//    else
//        fprintf("};\n");
//    end
//end


//for k=1:numel(Z)
//    if(k==1)
//        fprintf("QVector<double> Zmin_std={");
//    end
//    fprintf("%f",Zmin_std(k));
//    if(k<numel(Z))
//        fprintf(",");
//    else
//        fprintf("};\n");
//    end
//end

//for k=1:numel(Z)
//    if(k==1)
//        fprintf("QVector<double> moving_sigma_estimate={");
//    end
//    fprintf("%f",moving_sigma_estimate(k));
//    if(k<numel(Z))
//        fprintf(",");
//    else
//        fprintf("};\n");
//    end
//end


//fprintf("\n");


