#ifndef VOICEDETECTIONALGO_H
#define VOICEDETECTIONALGO_H

#include "dsp/dsp.h"

class VoiceDetectionAlgo
{
public:

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
    //main settings struct
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
    };

    VoiceDetectionAlgo();
    inline void addBuffer(const QVector<double> &input){buffer+=input;}
    QVector<double> &process();
    void setSettings(Settings settings);
    inline const Settings &getSettings(){return settings;}
    inline VoiceDetectionAlgo &operator<<(const QVector<double> &input)
    {
        addBuffer(input);
        return *this;
    }

    double snr_db;
    JDsp::InverseOverlappedRealFFT ifft;//maybe should do a copy or use a getter as the user shouldn't really be able to change ifft

private:

    Settings settings;

    JDsp::OverlappedRealFFT fft;
    JDsp::MovingNoiseEstimator mne;
    JDsp::MovingSignalEstimator mse;
    JDsp::OverlappedRealFFTDelayLine fft_delay1;
    JDsp::OverlappedRealFFTDelayLine fft_delay2;
    QVector<double> x;
    QVector<double> buffer;
    QVector<double> empty;
    JDsp::MovingMax<double> audio_out_movmax;
    JDsp::InverseOverlappedRealFFTDelayLine audio_out_delayline;//this should be half of movave
    JDsp::ScalarDelayLine<double> snr_estimate_delayline;//not sure how this changes for mne and mse

};

#endif // VOICEDETECTIONALGO_H
