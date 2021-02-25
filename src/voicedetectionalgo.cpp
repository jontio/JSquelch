#include "voicedetectionalgo.h"
#include <iostream>

VoiceDetectionAlgo::VoiceDetectionAlgo()
{
    //general
    settings.io_size=128;

    //moving_noise_estimator
    settings.moving_noise_estimator.moving_stats_window_size=16;
    settings.moving_noise_estimator.moving_minimum_window_size=16;//*4;
    settings.moving_noise_estimator.output_moving_average_window_size=62;//*8;

    //moving_signal_estimator
    settings.moving_signal_estimator.moving_stats_window_size=8;
    settings.moving_signal_estimator.min_voice_bin=3;
    settings.moving_signal_estimator.max_voice_bin=96;
    settings.moving_signal_estimator.output_moving_voice_snr_estimate_window_size=62;//*2;

    //fft delay line
    settings.fft_delay_size=51;

    //audio output
    settings.audio_out_delayline_size_in_blocks=31*2;
    settings.snr_estimate_delayline_size_in_blocks=0;

    //apply the settings
    setSettings(settings);
}

void VoiceDetectionAlgo::setSettings(Settings settings)
{
    //copy settings over
    this->settings=settings;

    //works in input time domain
    QVector<double>::reserve(fft.getInSize());
    buffer.clear();

    //works in frequency domain
    fft.setInSize(settings.io_size);
    mne.setSize(fft.getOutSize(),settings.moving_noise_estimator.moving_stats_window_size,settings.moving_noise_estimator.moving_minimum_window_size,settings.moving_noise_estimator.output_moving_average_window_size);
    mse.setSize(fft.getOutSize(),settings.moving_signal_estimator.moving_stats_window_size,settings.moving_signal_estimator.min_voice_bin,settings.moving_signal_estimator.max_voice_bin,settings.moving_signal_estimator.output_moving_voice_snr_estimate_window_size,0);
    ifft.setOutSize(fft.getInSize());
    fft_delay.setSize(fft,settings.fft_delay_size);

    //works in output time domain
    audio_out_delayline.setSize(settings.audio_out_delayline_size_in_blocks*fft.getInSize());
    snr_estimate_delayline.setSize(settings.snr_estimate_delayline_size_in_blocks);
}

QVector<double> &VoiceDetectionAlgo::process()
{
    //if we dont have enough in the buffer then return an empty vector
    if(buffer.size()<fft.getInSize())
    {
        empty.clear();
        return empty;
    }
    QVector<double>::operator=(buffer.mid(0,fft.getInSize()));
    buffer.remove(0,fft.getInSize());

    //do main algo
    fft<<(*this);//signal in
    mne<<fft;//output of fft to input of mne. calculates noise
    fft_delay.delay(fft);//fft delayline
    fft/=mne;//normalize output of fft
    mse<<fft;//output of fft into mse. calculates signal
    snr_estimate_delayline.delay(mse.voice_snr_estimate);//delay snr
    snr_db=10.0*log10(mse.voice_snr_estimate);//convert snr to db

    //delay the audio to align with snr_db and return the audio
    audio_out_delayline.delay(*this);

    return *this;
}
