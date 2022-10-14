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
    if(enc!=nullptr)
    {
        if(writtenAudio)ope_encoder_drain(enc);
        ope_encoder_destroy(enc);
        enc=nullptr;
    }
    if(comments!=nullptr)ope_comments_destroy(comments);

    writtenAudio=false;

    removeOldFileIfZeroBytes();

    this->settings=settings;
    buf.reserve(4096);
    comments = ope_comments_create();
    ope_comments_add(comments, "ARTIST", settings.artist.toLocal8Bit());
    ope_comments_add(comments, "TITLE", settings.title.toLocal8Bit());
    int error;

    //deal with ~/ kind of thing
    QString cpath=settings.filepath.trimmed();
    if((cpath.size()>0)&&(cpath[0]=="~"))cpath=QDir::homePath()+cpath.remove(0,1);

    qDebug()<<"creating enc"<<(cpath+"/"+settings.filename);

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
    writtenAudio=true;
}
void CompressAudioDiskWriter::close()
{
    if(!enc)return;
    if(writtenAudio)ope_encoder_drain(enc);
    ope_encoder_destroy(enc);
    ope_comments_destroy(comments);
    enc=nullptr;
    comments=nullptr;

    writtenAudio=false;

    removeOldFileIfZeroBytes();

}
bool CompressAudioDiskWriter::isOpen()
{
    if(!enc)return false;
    return true;
}

void CompressAudioDiskWriter::removeOldFileIfZeroBytes()
{
    //get old filename and delete it if it's empty
    QString cpath_old=this->settings.filepath.trimmed();
    if((cpath_old.size()>0)&&(cpath_old[0]=="~"))cpath_old=QDir::homePath()+cpath_old.remove(0,1);
    QString filename_full_old=(cpath_old+"/"+this->settings.filename);
    if(filename_full_old!="/")
    {
        qDebug()<<"finished with enc"<<filename_full_old;
        QFileInfo info(filename_full_old);
        if(info.exists()&&(info.size()==0))
        {
            qDebug()<<"deleting old file as it's zero bytes";
            QFile file(filename_full_old);
            file.remove();
        }
    }
}
