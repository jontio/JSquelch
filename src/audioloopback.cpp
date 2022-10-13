#include "audioloopback.h"
#include <math.h>
#include <QDebug>
#include <QThread>

//#define TESTING_AUDIOLOOPBACK

AudioLoopback::AudioLoopback(QObject *parent) :
    QIODevice(parent),
    m_audioInput(NULL),
    m_audioOutput(NULL)
{
    errorTimer=new QTimer(this);
    connect(errorTimer,SIGNAL(timeout()),this,SLOT(onAudioErrorTimer()));
    setSettings(settings);
    clear();
}

void AudioLoopback::clear()
{
    circ_buffer.fill(0,settings.circ_buffer_size);
    circ_buffer_head=0;
    circ_buffer_tail=circ_buffer.size()/2;
    x=0;
    fract_buffer_space=0;
}

void AudioLoopback::start()
{
    open(QIODevice::ReadWrite);
    if(m_audioInput)m_audioInput->start(this);
    if(m_audioOutput)m_audioOutput->start(this);
    errorTimer->start(100);
}

void AudioLoopback::stop()
{
    errorTimer->stop();
    if(m_audioInput)m_audioInput->stop();
    if(m_audioOutput)m_audioOutput->stop();
    close();
}

void AudioLoopback::setSettings(Settings settings)
{

    bool wasopen=isOpen();
    stop();

    //if Fs has changed or the audio device doesnt exist or the input device has changed then need to redo the audio device
    if((settings.Fs!=this->settings.Fs)||
            (!m_audioInput)||
            (!m_audioOutput)||
            (settings.audio_device_out!=this->settings.audio_device_out)||
            (settings.audio_device_in!=this->settings.audio_device_in)
            )
    {
        this->settings=settings;

        if(m_audioInput)m_audioOutput->deleteLater();
        if(m_audioOutput)m_audioOutput->deleteLater();

        //set the format
        m_format.setSampleRate(settings.Fs);
        m_format.setChannelCount(1);
        m_format.setSampleSize(16);
        m_format.setCodec("audio/pcm");
        m_format.setByteOrder(QAudioFormat::LittleEndian);
        m_format.setSampleType(QAudioFormat::SignedInt);

        //setup
        m_audioInput = new QAudioInput(settings.audio_device_in, m_format, this);
        m_audioOutput = new QAudioOutput(settings.audio_device_out, m_format, this);
    }
    this->settings=settings;

    clear();

    if(wasopen)start();
}

AudioLoopback::~AudioLoopback()
{
    stop();
}

//for audio out
qint64 AudioLoopback::readData(char *data, qint64 maxlen)
{

    qint16 *ptr = reinterpret_cast<qint16 *>(data);
    int numofsamples=(maxlen/sizeof(qint16));
#ifdef TESTING_AUDIOLOOPBACK
    int org_numofsamples=numofsamples;
#endif

    //if we return to many the os may drop frames. about 64 at 8kHz seems ok on my computer
    numofsamples=qMin(settings.max_frames_to_play_per_call,numofsamples);

    //calculate space
    int forward=abs(circ_buffer_tail-circ_buffer_head);
    int backwards=circ_buffer.size()-forward;
    if(circ_buffer_tail>circ_buffer_head)
    {
        int tmp=forward;
        forward=backwards;
        backwards=tmp;
    }

    //space in -1 to 1. we want this to be about 0
    const double alpha=0.001;
    fract_buffer_space=fract_buffer_space*(1.0-alpha)+(alpha)*((2.0*(double)forward/circ_buffer.size())-1.0);

#ifdef TESTING_AUDIOLOOPBACK
    static int slow_inserts=0;
    static int fast_inserts=0;
    static int tisk=0;
    tisk++;tisk%=100;
    if(tisk==0)qDebug()<<fract_buffer_space<<forward<<x<<slow_inserts<<fast_inserts;
#endif

    //choose the max buffer you wish to process
    numofsamples=qMin(forward-1,numofsamples);

    //if we have run out of data return almost nothing
    if(numofsamples<1)
    {
        circ_buffer[circ_buffer_tail]=((double)circ_buffer[circ_buffer_tail])*0.75;
        *ptr=circ_buffer[circ_buffer_tail];
#ifdef TESTING_AUDIOLOOPBACK
        qDebug()<<"nothing to play"<<forward<<org_numofsamples;
#endif
        return sizeof(qint16);
    }

    //fill in the return data
    for(int i=0;i<numofsamples;i++)
    {

        //load early prompt and late samples
        int tmp_circ_buffer_tail=circ_buffer_tail;
        qint16 early=circ_buffer[circ_buffer_tail];
        circ_buffer_tail++;circ_buffer_tail%=circ_buffer.size();
        qint16 prompt=circ_buffer[circ_buffer_tail];
        circ_buffer_tail++;circ_buffer_tail%=circ_buffer.size();
        qint16 late=circ_buffer[circ_buffer_tail];
        circ_buffer_tail=tmp_circ_buffer_tail;

        //slow move sampling so we can do a +-1 jump of x when needed
        //will make a small wooshing sound when adjusting. bother
        x=x*(1.0-0.0005)+0.0005*fract_buffer_space;

        //interpolation
        double y=0;
        if(x<0)y=(1.0+x)*(double)prompt-x*(double)early;
        else y=(1.0-x)*(double)prompt+x*(double)late;

        //take a step if needed and adjust x so we still are using the same interpolation location
        if(x<-0.5)
        {
#ifdef TESTING_AUDIOLOOPBACK
//            qDebug()<<"playing fast";
            fast_inserts++;
#endif
            x+=1.0;
        }
        else
        {
            if(x>0.5)
            {
#ifdef TESTING_AUDIOLOOPBACK
//                qDebug()<<"playing slow";
                slow_inserts++;
#endif
                x-=1.0;
                //check we arn't going to pass the head
                //if we will then stop
                forward--;
                if(forward<1)
                {
#ifdef TESTING_AUDIOLOOPBACK
                    qDebug()<<"bailing will overflow";
#endif
                    numofsamples=i;
                    break;
                }
                circ_buffer_tail++;circ_buffer_tail%=circ_buffer.size();
            }
            //check we arn't going to pass the head
            //if we will then stop
            forward--;
            if(forward<1)
            {
#ifdef TESTING_AUDIOLOOPBACK
                qDebug()<<"bailing will overflow";
#endif
                numofsamples=i;
                break;
            }
            circ_buffer_tail++;circ_buffer_tail%=circ_buffer.size();
        }

        //return the interpolated sample
        *ptr=y;
        ptr++;
    }

    //say how much we have written
    return (numofsamples*sizeof(qint16));

}

//for audio in
qint64 AudioLoopback::writeData(const char *data, qint64 len)
{

#ifdef TESTING_AUDIOLOOPBACK
    static int counter=0;
    counter++;counter%=2000;//2000->500ppm 1000->1000ppm, 500->2000ppm etc
    const bool playslow=false;//true;

    const qint16 *ptr = reinterpret_cast<const qint16 *>(data);
    int numofsamples=(len/sizeof(qint16));
    static int si=0;
    for(int i=0;i<numofsamples;i++)
    {

        int forward=abs(circ_buffer_tail-circ_buffer_head);
        int backwards=circ_buffer.size()-forward;
        if(circ_buffer_tail>circ_buffer_head)
        {
            int tmp=forward;
            forward=backwards;
            backwards=tmp;
        }

        //if wanting to play fast
        if((!playslow)&&(counter==0))continue;

        //if wanting to play slow
        if(( playslow)&&(counter==0))
        {
            //make sure we don't pass the tail
            if(backwards>1){circ_buffer_head++;circ_buffer_head%=circ_buffer.size();}

            double dval=0.1*sin(2.0*M_PI*500.0*((double)si)/(((double)(settings.Fs))));
            si+=1.0;
            qint16 sval=floor(0.75*dval*32767.0+0.5);
            circ_buffer[circ_buffer_head]=sval;
        }

        //make sure we don't pass the tail
        if(backwards>1){circ_buffer_head++;circ_buffer_head%=circ_buffer.size();}

        double dval=0.1*sin(2.0*M_PI*500.0*((double)si)/(((double)(settings.Fs))));
        si+=1.0;
        qint16 sval=floor(0.75*dval*32767.0+0.5);
        circ_buffer[circ_buffer_head]=sval;

        ptr++;
    }

    while(si>0)si-=settings.Fs;

    return (numofsamples*sizeof(qint16));
#else

    //get pointer and len to input signal
    const qint16 *ptr = reinterpret_cast<const qint16 *>(data);
    int numofsamples=(len/sizeof(qint16));
    int org_numofsamples=numofsamples;

    //request that someone uses the input and generates some output for us

    //put input audio into double qvector and pass to user for processing
    m_processAudio_input.resize(numofsamples);
    for(int k=0;k<numofsamples;k++)m_processAudio_input[k]=((double)(*ptr++))/32768.0;
    processAudio(m_processAudio_input,m_processAudio_output);

    //process the users returned audio
    numofsamples=m_processAudio_output.size();
    for(int k=0;k<numofsamples;k++)
    {

        int forward=abs(circ_buffer_tail-circ_buffer_head);
        int backwards=circ_buffer.size()-forward;
        if(circ_buffer_tail>circ_buffer_head)
        {
            int tmp=forward;
            forward=backwards;
            backwards=tmp;
        }

        //make sure we don't pass the tail
        if(backwards>1){circ_buffer_head++;circ_buffer_head%=circ_buffer.size();}
        circ_buffer[circ_buffer_head]=(m_processAudio_output[k]*32768.0);//*ptr;

        //ptr++;
    }

    return (org_numofsamples*sizeof(qint16));

#endif

}

//for error testing and restarting if something goes wrong
//this catches when RDP server kills the audio input device
void AudioLoopback::onAudioErrorTimer()
{
    if(m_audioInput)
    {
        QAudio::Error error=m_audioInput->error();
        if(error!=QAudio::NoError)
        {
            qDebug()<<"Audio input error trying to start input again";
            start();
        }
    }
    if(m_audioOutput)
    {
        QAudio::Error error=m_audioOutput->error();
        if(error!=QAudio::NoError)
        {
            qDebug()<<"Audio output error trying to start output again";
            start();
        }
    }
}
