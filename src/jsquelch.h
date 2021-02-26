#ifndef JSQUELCH_H
#define JSQUELCH_H

#include <QMainWindow>
#include "src/dsp/dsp.h"
#include "audioloopback.h"
#include "voicedetectionalgo.h"
#include <QCloseEvent>

extern "C"
{
#include <opus/opusenc.h>
}


QT_BEGIN_NAMESPACE
namespace Ui { class JSquelch; }
QT_END_NAMESPACE

class JSquelch : public QMainWindow
{
    Q_OBJECT

public:
    JSquelch(QWidget *parent = nullptr);
    ~JSquelch();
    VoiceDetectionAlgo algo;
    AudioLoopback audioLoopback;
    void applySettings();
public slots:
    void processAudio(const QVector<double> &input, QVector<double> &output);
    void spinBoxChanged(int value);
    void doubleSpinBoxChanged(double value);
protected:
    void closeEvent (QCloseEvent *event);
private:
    Ui::JSquelch *ui;
    double hysteresis_db=2;
    double threshold_level_db=6;
    bool audio_on_state=false;

    OggOpusEnc *enc;
    OggOpusComments *comments;

};
#endif // JSQUELCH_H
