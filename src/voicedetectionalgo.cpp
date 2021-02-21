#include "voicedetectionalgo.h"

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

    //works in frequency domain
    fft.setInSize(settings.io_size);
    mne.setSize(fft.getOutSize(),settings.moving_noise_estimator.moving_stats_window_size,settings.moving_noise_estimator.moving_minimum_window_size,settings.moving_noise_estimator.output_moving_average_window_size);
    mse.setSize(fft.getOutSize(),settings.moving_signal_estimator.moving_stats_window_size,settings.moving_signal_estimator.min_voice_bin,settings.moving_signal_estimator.max_voice_bin,settings.moving_signal_estimator.output_moving_voice_snr_estimate_window_size,settings.moving_signal_estimator.output_moving_average_window_size);
    ifft.setOutSize(fft.getInSize());
    fft_delay1.setSize(fft,settings.fft_delay1_size);
    fft_delay2.setSize(fft,settings.fft_delay2_size);

    QVector<double> x(fft.getInSize());

    //works in time domain
    audio_out_movmax.setSize(settings.audio_out_movmax_size_in_blocks);
    audio_out_delayline.setSize(settings.audio_out_delayline_size_in_blocks*fft.getInSize());//this should be half of movave
    snr_estimate_delayline.setSize(settings.snr_estimate_delayline_size_in_blocks);//not sure how this changes for mne and mse

}
