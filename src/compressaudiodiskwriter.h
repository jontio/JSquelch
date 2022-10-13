#ifndef COMPRESSAUDIODISKWRITER_H
#define COMPRESSAUDIODISKWRITER_H

#include <QDir>
#include <QDebug>
#include "util/RuntimeError.h"

extern "C"
{
#include <opus/opusenc.h>
}

class CompressAudioDiskWriter
{
public:
    struct Settings
    {
        QString filepath="";
        QString filename="";
        int bitRate=48000;
        int nChannels=2;
        QString artist="CompressAudioWriter";
        QString title="Item 1";
    };
    CompressAudioDiskWriter();
    CompressAudioDiskWriter(const Settings &settings);
    ~CompressAudioDiskWriter();
    bool setSettings(const Settings &settings);
    const Settings &getSettings(){return settings;}
    void writeAudio(const QVector<double> &output);
    void close();
    bool isOpen();
private:
    Settings settings;
    OggOpusEnc *enc;
    OggOpusComments *comments;
    QVector<opus_int16> buf;
};

#endif // COMPRESSAUDIODISKWRITER_H
