#ifndef VOICEDETECTIONALGO_H
#define VOICEDETECTIONALGO_H

#include "dsp/dsp.h"

class VoiceDetectionAlgo : public QVector<double>
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
    };
    //main settings struct
    struct Settings
    {
        int io_size=0;
        int fft_delay_size=0;
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

    double snr_db_pre_delayline;

private:

    Settings settings;

    JDsp::OverlappedRealFFT fft;
    JDsp::MovingNoiseEstimator mne;
    JDsp::MovingSignalEstimator mse;
    JDsp::OverlappedRealFFTDelayLine fft_delay;
    QVector<double> buffer;
    QVector<double> empty;
    JDsp::AudioDelayLine audio_out_delayline;//this should be half of movave
    JDsp::ScalarDelayLine<double> snr_db_delayline;//not sure how this changes for mne and mse

};

#endif // VOICEDETECTIONALGO_H
