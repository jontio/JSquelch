#ifndef AUDIOLOOPBACK_H
#define AUDIOLOOPBACK_H

#include <QObject>
#include <QAudioInput>
#include <QAudioOutput>
#include <QVector>

class AudioLoopback : public QIODevice
{
    Q_OBJECT
public:
    struct Settings
    {
        QAudioDeviceInfo audio_device_in;
        QAudioDeviceInfo audio_device_out;
        int max_frames_to_play_per_call;
        int circ_buffer_size;
        double Fs;
        Settings()
        {
            audio_device_in=QAudioDeviceInfo::defaultInputDevice();
            audio_device_out=QAudioDeviceInfo::defaultOutputDevice();
            max_frames_to_play_per_call=64;//if we return to many the os may drop frames. about 64 at 8kHz seems ok on my computer
            circ_buffer_size=4096;//if too small then a bit of clicking
            Fs=8000;
        }
    };
    explicit AudioLoopback(QObject *parent = nullptr);
    ~AudioLoopback();
    void start();
    void stop();
    void setSettings(Settings settings);
    const Settings &setSettings(){return settings;}
    qint64 readData(char *data, qint64 maxlen);
    qint64 writeData(const char *data, qint64 len);

signals:
    void processAudio(const QVector<double> &input, QVector<double> &output);
public slots:
private:
    Settings settings;
    QAudioFormat m_format;
    QAudioInput *m_audioInput;
    QAudioOutput *m_audioOutput;
    QVector<qint16> circ_buffer;
    int circ_buffer_head;
    int circ_buffer_tail;
    double x;
    double fract_buffer_space;
    void clear();
    QVector<double> m_processAudio_input;
    QVector<double> m_processAudio_output;
};

#endif // AUDIOLOOPBACK_H
