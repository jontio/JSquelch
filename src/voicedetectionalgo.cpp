#include "voicedetectionalgo.h"
#include <iostream>

VoiceDetectionAlgo::VoiceDetectionAlgo()
{
    //general
    settings.io_size=128;

    //moving_noise_estimator
    settings.moving_noise_estimator.moving_stats_window_size=16;
    settings.moving_noise_estimator.moving_minimum_window_size=16;
    settings.moving_noise_estimator.output_moving_average_window_size=62;

    //moving_signal_estimator
    settings.moving_signal_estimator.moving_stats_window_size=8;
    settings.moving_signal_estimator.min_voice_bin=3;
    settings.moving_signal_estimator.max_voice_bin=96;
    settings.moving_signal_estimator.output_moving_voice_snr_estimate_window_size=62;
    settings.moving_signal_estimator.output_moving_average_window_size=8;

    //fft delay lines
    settings.fft_delay1_size=51;
    settings.fft_delay2_size=6;

    //audio output
    settings.audio_out_movmax_size_in_blocks=62;
    settings.audio_out_delayline_size_in_blocks=31;
    settings.snr_estimate_delayline_size_in_blocks=0;

    //apply the settings
    setSettings(settings);
}

void VoiceDetectionAlgo::setSettings(Settings settings)
{
    //copy settings over
    this->settings=settings;

    //works in input time domain
    x.reserve(fft.getInSize());
    buffer.clear();

    //works in frequency domain
    fft.setInSize(settings.io_size);
    mne.setSize(fft.getOutSize(),settings.moving_noise_estimator.moving_stats_window_size,settings.moving_noise_estimator.moving_minimum_window_size,settings.moving_noise_estimator.output_moving_average_window_size);
    mse.setSize(fft.getOutSize(),settings.moving_signal_estimator.moving_stats_window_size,settings.moving_signal_estimator.min_voice_bin,settings.moving_signal_estimator.max_voice_bin,settings.moving_signal_estimator.output_moving_voice_snr_estimate_window_size,settings.moving_signal_estimator.output_moving_average_window_size);
    ifft.setOutSize(fft.getInSize());
    fft_delay1.setSize(fft,settings.fft_delay1_size);
    fft_delay2.setSize(fft,settings.fft_delay2_size);

    //works in output time domain
    audio_out_movmax.setSize(settings.audio_out_movmax_size_in_blocks);
    audio_out_delayline.setSize(settings.audio_out_delayline_size_in_blocks*fft.getInSize());//this should be half of movave
    snr_estimate_delayline.setSize(settings.snr_estimate_delayline_size_in_blocks);//not sure how this changes for mne and mse
}

QVector<double> &VoiceDetectionAlgo::process()
{
    //if we dont have enough in the buffer then return an empty vector
    if(buffer.size()<fft.getInSize())
    {
        empty.clear();
        return empty;
    }
    x=buffer.mid(0,fft.getInSize());
    buffer.remove(0,fft.getInSize());

    //do main algo
    fft<<x;//signal in
    mne<<fft;//output of fft to input of mne
    fft_delay1.delay(fft);//1st delayline to match matlab
    fft/=mne;//normalize output of fft
    mse<<fft;//output of fft into mse
    fft_delay2.delay(fft);//2nd delayline to match matlab
    fft*=mse;//scale fft output by signal estimate

    //now reconstruct a signal and get volume and adjust timing etc
    ifft<<fft;//output of fft to inverse fft to produce reconstuted signal
    audio_out_movmax<<ifft;//update moving max with time domain output signal
    audio_out_delayline.delay(ifft);//delay the output by half the moving window length so we are looking forward and back in time.
    ifft/=(1.25*audio_out_movmax);//normalize the time domain output signal

    snr_estimate_delayline.delay(mse.voice_snr_estimate);//delay snr as audio is delayed
    snr_db=10.0*log10(mse.voice_snr_estimate);//convert snr to db

    return ifft;
}
