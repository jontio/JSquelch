#ifndef VOICEDETECTIONALGO_H
#define VOICEDETECTIONALGO_H

#include "dsp/dsp.h"


class VoiceDetectionAlgo
{
public:
    VoiceDetectionAlgo();

    struct Settings_MovingNoiseEstimator
    {
        int moving_stats_window_size=0;
        int moving_minimum_window_size=0;
        int output_moving_average_window_size=0;
    };

    struct Settings_MovingSignalEstimator
    {
        int moving_stats_window_size=0;
        int min_voice_bin=0;
        int max_voice_bin=0;
        int output_moving_voice_snr_estimate_window_size=0;
        int output_moving_average_window_size=0;
    };

    struct Settings
    {
        int io_size=0;
        int fft_delay1_size=0;
        int fft_delay2_size=0;
        int audio_out_movmax_size_in_blocks=0;
        int audio_out_delayline_size_in_blocks=0;
        int snr_estimate_delayline_size_in_blocks=0;
        Settings_MovingNoiseEstimator moving_noise_estimator;
        Settings_MovingSignalEstimator moving_signal_estimator;

        Settings()
        {

        }
    };

//private:

    Settings settings;

    JDsp::OverlappedRealFFT fft;
    JDsp::MovingNoiseEstimator mne;
    JDsp::MovingSignalEstimator mse;
    JDsp::InverseOverlappedRealFFT ifft;
    JDsp::OverlappedRealFFTDelayLine fft_delay1;
    JDsp::OverlappedRealFFTDelayLine fft_delay2;
    QVector<double> x;
    double audio_out_movmax_delay_in_frames=8000*1;
    JDsp::MovingMax<double> audio_out_movmax;
    JDsp::InverseOverlappedRealFFTDelayLine audio_out_delayline;//this should be half of movave
    JDsp::ScalarDelayLine<double> snr_estimate_delayline;//not sure how this changes for mne and mse

};

#endif // VOICEDETECTIONALGO_H
