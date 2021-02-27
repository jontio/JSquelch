#include "compressaudiodiskwriter.h"

CompressAudioDiskWriter::CompressAudioDiskWriter():
    enc(nullptr),
    comments(nullptr)
{
}
CompressAudioDiskWriter::CompressAudioDiskWriter(const Settings &settings):
    enc(nullptr),
    comments(nullptr)
{
    setSettings(settings);
}
CompressAudioDiskWriter::~CompressAudioDiskWriter()
{
    close();
}
bool CompressAudioDiskWriter::setSettings(const Settings &settings)
{
    this->settings=settings;
    buf.reserve(4096);
    comments = ope_comments_create();
    ope_comments_add(comments, "ARTIST", settings.artist.toLocal8Bit());
    ope_comments_add(comments, "TITLE", settings.title.toLocal8Bit());
    int error;

    //deal with ~/ kind of thing
    QString cpath=settings.filepath.trimmed();
    if((cpath.size()>0)&&(cpath[0]=="~"))cpath=QDir::homePath()+cpath.remove(0,1);

    enc = ope_encoder_create_file((cpath+"/"+settings.filename).toLocal8Bit(), comments, settings.bitRate, settings.nChannels, 0, &error);
    if(!enc)
    {
        ope_comments_destroy(comments);
        qDebug()<<"Cant create libopusenc opject";
        return false;
    }
    return true;
}
void CompressAudioDiskWriter::writeAudio(const QVector<double> &output)
{
    if(!enc)return;
    buf.resize(output.size());
    for(int k=0;k<buf.size();k++)buf[k]=((double)output[k])*32768.0;
    int error=ope_encoder_write(enc, buf.data(), buf.size());
    if(error<0)RUNTIME_ERROR("Encoder write error",error);
}
void CompressAudioDiskWriter::close()
{
    if(!enc)return;
    ope_encoder_drain(enc);
    ope_encoder_destroy(enc);
    ope_comments_destroy(comments);
    enc=nullptr;
    comments=nullptr;
}
bool CompressAudioDiskWriter::isOpen()
{
    if(!enc)return false;
    return true;
}
