# JSquelch

![CI](https://github.com/jontio/JSquelch/workflows/CI/badge.svg)

Next step of the voice detection code.

I have added heaps of classes to the JDsp library, we now have...

Hann
StrangeSineCorrectionWindow
VectorDelayLine
ScalarDelayLine
VectorMovingAverage
VectorMovingVariance
VectorMovingMax
MovingMax
VectorMovingMin
VectorMovingMinWithAssociate
OverlappedRealFFT
OverlappedRealFFTDelayLine
InverseOverlappedRealFFT
InverseOverlappedRealFFTDelayLine
MovingNoiseEstimator
Normalize
MovingAverage
MovingSignalEstimator

I'm not going to explain all of them. There are various operator overloading for ease of use but you don't have to use them if you don't want to. Almost all classes have at least some unit testing code.

I have now added the VoiceDetectionAlgo class. This is going to be the main algorithm class and uses all the classes in the JDsp library to do it's job. VoiceDetectionAlgo doesn't care about the input vector size so can be used for the arbitrary size buffers that come from soundcard drivers. To use it, it goes something like this (I've removed file error checking for easier reading)...

```C++
    VoiceDetectionAlgo algo;

    //load test file
    QFile file("test_signal_time.raw");
    file.open(QIODevice::ReadOnly);
    QDataStream datastream(&file);
    datastream.setByteOrder(QDataStream::LittleEndian);
    datastream.setFloatingPointPrecision (QDataStream::SinglePrecision);

    QVector<double> x;
    x.fill(0,128);

    //what we want from the algo output
    QVector<double> actual_snr_estimate_db_signal;
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

        //add the audio to the algo
        algo+=x;

        //process the audio wile we have some
        while(!algo.process().empty())
        {
            //skip nan blocks
            if(isnan(algo.snr_db))continue;

            //keep snr_db
            actual_snr_estimate_db_signal+=algo.snr_db;

            //keep output signal when snr_db is good
            if(algo.snr_db<5.0) continue;
            actual_output_signal+=algo.ifft;
        }

    }
    file.close();


    file.setFileName("audio_out.raw");
    file.open(QIODevice::WriteOnly|QIODevice::Truncate));
    for(int k=0;k<actual_output_signal.size();k++)datastream<<actual_output_signal[k];
    file.close();
```    

