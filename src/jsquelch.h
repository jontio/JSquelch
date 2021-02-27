#ifndef JSQUELCH_H
#define JSQUELCH_H

#include <QMainWindow>
#include "src/dsp/dsp.h"
#include "audioloopback.h"
#include "voicedetectionalgo.h"
#include <QCloseEvent>
#include "src/compressaudiodiskwriter.h"

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
    void applySettings();
    void processAudio(const QVector<double> &input, QVector<double> &output);
    void spinBoxChanged(int value);
    void doubleSpinBoxChanged(double value);
    void checkBoxStateChange(int value);
    void lineEditTextChanged(const QString &text);
protected:
    void closeEvent (QCloseEvent *event);
private:
    Ui::JSquelch *ui;
    double hysteresis_db=2;
    double threshold_level_db=6;
    bool audio_on_state=false;
    bool agc_on=false;

    CompressAudioDiskWriter audio_disk_writer;
    JDsp::AGC agc;

    void loadSettings();
    void saveSettings();

};
#endif // JSQUELCH_H
