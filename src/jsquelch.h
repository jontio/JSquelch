#ifndef JSQUELCH_H
#define JSQUELCH_H

#include <QMainWindow>
#include "src/dsp/dsp.h"
#include "audioloopback.h"
#include "voicedetectionalgo.h"

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
public slots:
    void processAudio(const QVector<double> &input, QVector<double> &output);

private:
    Ui::JSquelch *ui;
//    JDsp::VectorMovingAverage<double> mv;
};
#endif // JSQUELCH_H
