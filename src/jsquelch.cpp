#include "jsquelch.h"
#include "ui_jsquelch.h"
#include <iostream>
#include <QtDebug>
#include <limits>
#include <random>
#include "util/stdio_utils.h"
#include "util/file_utils.h"
#include <QFile>
#include <QDir>
#include <QTimer>
#include <QDateTime>
#include <QMessageBox>
#include <QSettings>

using namespace std;

QString settings_name;

void JSquelch::processAudio(const QVector<double> &input, QVector<double> &output)
{

    //put the audio into the algo buffer
    algo<<input;
    output.clear();

    //process the audio wile we have some
    while(!algo.process().empty())
    {
        //limit the snr to be above -100
        if(std::isnan(algo.snr_db)||(algo.snr_db<-100))algo.snr_db=-100;

        if(agc_on)
        {
            //update agc. this has a delayline in it so audio is delayed
            //and wont sync with led. using snr_db_pre_delayline mean you
            //can add a snr_db delay and get the led aligned
            if(algo.snr_db_pre_delayline>threshold_level_db)
            {
                agc.update(algo,true);
            }
            else agc.update(algo,false);
        }

        //streching the snr so it stays on for longer
        algo.snr_db=snr_strech.update(algo.snr_db);

        //silence the audio if snr is low
        //using hysteresis
        if(!audio_on_state)
        {
            algo.fill(0);
            if(algo.snr_db>(threshold_level_db+hysteresis_db))audio_on_state=true;
        }
        else
        {
            if(algo.snr_db<(threshold_level_db-hysteresis_db))audio_on_state=false;
        }

        //add the audio to the output
        output+=algo;
    }

    //there should be a compressor here
    for(int k=0;k<output.size();k++)
    {
        if(output[k]>0.999)output[k]=0.999;
        if(output[k]<-0.999)output[k]=-0.999;
    }

    //turn on led and display snr
    ui->led_snr->On(audio_on_state);
    ui->label_snr->setText(QString::asprintf("%0.1f",algo.snr_db)+"dB");

    //record audio if wanted
    if(audio_on_state)
    {
        //if there was an audio pause and the user wants to split files
        if((!audio_on_state_last)&&(ui->checkBox_split_audio_files->isChecked()))
        {
            //change audio file name if silence has been too long
            if((!audio_on_state_off_elapsed_timer.isValid())||(audio_on_state_off_elapsed_timer.hasExpired(1000.0*ui->doubleSpinBox_silence_before_file_split_in_s->value())))
            {
                if(!audio_on_state_off_elapsed_timer.isValid())
                {
                    qDebug()<<"new audio";
                }
                else
                {
                    qDebug()<<"audio gap bigger than threshold. gap was"<<audio_on_state_off_elapsed_timer.elapsed()<<"ms";
                }

                qDebug()<<"time to change file name";
                CompressAudioDiskWriter::Settings disk_writer_setings=audio_disk_writer.getSettings();
                QDateTime datetime(QDateTime::currentDateTimeUtc());
                disk_writer_setings.filename=datetime.toString("yyMMdd_hhmmss")+".ogg";
                audio_disk_writer.setSettings(disk_writer_setings);
            }
        }
        audio_on_state_off_elapsed_timer.start();

        audio_disk_writer.writeAudio(output);
    }
    audio_on_state_last=audio_on_state;

}

JSquelch::JSquelch(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::JSquelch)
{
    ui->setupUi(this);

    foreach (const QAudioDeviceInfo &deviceInfo, QAudioDeviceInfo::availableDevices(QAudio::AudioInput))
    {
        ui->soundcard_input->addItem(deviceInfo.deviceName());
    }

    loadSettings();

    connect(ui->spinBox_fft_delay_size,SIGNAL(valueChanged(int)),this,SLOT(spinBoxChanged(int)));
    connect(ui->spinBox_mse_max_voice_bin,SIGNAL(valueChanged(int)),this,SLOT(spinBoxChanged(int)));
    connect(ui->spinBox_mse_min_voice_bin,SIGNAL(valueChanged(int)),this,SLOT(spinBoxChanged(int)));
    connect(ui->spinBox_mne_moving_stats_window_size,SIGNAL(valueChanged(int)),this,SLOT(spinBoxChanged(int)));
    connect(ui->spinBox_mse_moving_stats_window_size,SIGNAL(valueChanged(int)),this,SLOT(spinBoxChanged(int)));
    connect(ui->spinBox_mne_moving_minimum_window_size,SIGNAL(valueChanged(int)),this,SLOT(spinBoxChanged(int)));
    connect(ui->spinBox_audio_out_delayline_size_in_blocks,SIGNAL(valueChanged(int)),this,SLOT(spinBoxChanged(int)));
    connect(ui->spinBox_mne_output_moving_average_window_size,SIGNAL(valueChanged(int)),this,SLOT(spinBoxChanged(int)));
    connect(ui->spinBox_snr_estimate_delayline_size_in_blocks,SIGNAL(valueChanged(int)),this,SLOT(spinBoxChanged(int)));
    connect(ui->spinBox_mse_output_moving_snr_estimate_window_size,SIGNAL(valueChanged(int)),this,SLOT(spinBoxChanged(int)));

    connect(ui->doubleSpinBox_hysteresis_db,SIGNAL(valueChanged(double)),this,SLOT(doubleSpinBoxChanged(double)));
    connect(ui->doubleSpinBox_snr_threshold_level_db,SIGNAL(valueChanged(double)),this,SLOT(doubleSpinBoxChanged(double)));
    connect(ui->checkBox_agc,SIGNAL(stateChanged(int)),this,SLOT(checkBoxStateChange(int)));
    connect(ui->checkBox_save_audio,SIGNAL(stateChanged(int)),this,SLOT(checkBoxStateChange(int)));
    connect(ui->lineEdit_audiopath,SIGNAL(textChanged(const QString &)),this,SLOT(lineEditTextChanged(const QString &)));
    connect(ui->spinBox_snr_streching_in_blocks,SIGNAL(valueChanged(int)),this,SLOT(spinBoxChanged(int)));

    connect(&audioLoopback,&AudioLoopback::processAudio,this,&JSquelch::processAudio,Qt::DirectConnection);
    applySettings();

#ifdef WIN32
    //windows seems to like a bit bigger number of frames per call
    AudioLoopback::Settings loopbacksettings=audioLoopback.getSettings();
    loopbacksettings.max_frames_to_play_per_call=160;
    audioLoopback.setSettings(loopbacksettings);
#endif

    audioLoopback.start();

#ifdef WIN32
    //for some reasion on my windows computer I have to start 2 times else sample rates are wrong
    audioLoopback.start();
#endif

    setMaximumSize(minimumWidth(), minimumHeight());
    resize(minimumWidth(), minimumHeight());
}

void JSquelch::lineEditTextChanged(const QString &text)
{
    Q_UNUSED(text);
    if(ui->checkBox_save_audio->isChecked())
    {
        ui->checkBox_save_audio->click();
    }
}

void JSquelch::spinBoxChanged(int value)
{
    Q_UNUSED(value);
    applySettings();
}

void JSquelch::doubleSpinBoxChanged(double value)
{
    Q_UNUSED(value);
    applySettings();
}

void JSquelch::checkBoxStateChange(int value)
{
    Q_UNUSED(value);
    applySettings();
}

void JSquelch::applySettings()
{
    VoiceDetectionAlgo::Settings settings=algo.getSettings();

    //delay compensation
    settings.fft_delay_size=ui->spinBox_fft_delay_size->value();
    settings.audio_out_delayline_size_in_blocks=ui->spinBox_audio_out_delayline_size_in_blocks->value();
    settings.snr_estimate_delayline_size_in_blocks=ui->spinBox_snr_estimate_delayline_size_in_blocks->value();

    //other
    snr_strech.setSize(ui->spinBox_snr_streching_in_blocks->value());
    hysteresis_db=ui->doubleSpinBox_hysteresis_db->value();
    threshold_level_db=ui->doubleSpinBox_snr_threshold_level_db->value();
    agc_on=ui->checkBox_agc->isChecked();
    on_checkBox_split_audio_files_clicked();
    //just use some hard coded settings for the agc
    JDsp::AGC::Settings agc_settings=agc.getSettings();
    agc_settings.K=0.01;
    agc_settings.agc_level=1.0;
    agc_settings.max_gain=100;
    agc_settings.moving_max_window_size=8000;
    agc_settings.delayline_size=8000;
    agc.setSettings(agc_settings);
    //for file saving
    if(ui->checkBox_save_audio->isChecked())
    {
        CompressAudioDiskWriter::Settings disk_writer_setings=audio_disk_writer.getSettings();
        if((!audio_disk_writer.isOpen())|| //not open
                (disk_writer_setings.filepath!=ui->lineEdit_audiopath->text()))//or path name change
        {

            //create a file name
            QDir dir;
            QString cpath=ui->lineEdit_audiopath->text().trimmed();
            if((cpath.size()>0)&&(cpath[0]=="~"))cpath=QDir::homePath()+cpath.remove(0,1);
            dir.mkpath(cpath);
            QDateTime datetime(QDateTime::currentDateTimeUtc());
            QString filename_without_path=datetime.toString("yyMMdd_hhmmss")+".ogg";

            disk_writer_setings.filepath=ui->lineEdit_audiopath->text();
            disk_writer_setings.filename=filename_without_path;
            disk_writer_setings.bitRate=8000;
            disk_writer_setings.nChannels=1;
            audio_disk_writer.setSettings(disk_writer_setings);
        }
    }
    else audio_disk_writer.close();
    if(audio_disk_writer.isOpen()!=ui->checkBox_save_audio->isChecked())
    {
        ui->checkBox_save_audio->click();
    }

    //noise estimator
    settings.moving_noise_estimator.moving_stats_window_size=ui->spinBox_mne_moving_stats_window_size->value();
    settings.moving_noise_estimator.moving_minimum_window_size=ui->spinBox_mne_moving_minimum_window_size->value();
    settings.moving_noise_estimator.output_moving_average_window_size=ui->spinBox_mne_output_moving_average_window_size->value();

    //signal estimator
    settings.moving_signal_estimator.max_voice_bin=ui->spinBox_mse_max_voice_bin->value();
    settings.moving_signal_estimator.min_voice_bin=ui->spinBox_mse_min_voice_bin->value();
    settings.moving_signal_estimator.moving_stats_window_size=ui->spinBox_mse_moving_stats_window_size->value();
    settings.moving_signal_estimator.output_moving_voice_snr_estimate_window_size=ui->spinBox_mse_output_moving_snr_estimate_window_size->value();

    algo.setSettings(settings);
}

void JSquelch::loadSettings()
{

    QSettings settings("JontiSoft",settings_name);

    //delay compensation
    ui->spinBox_fft_delay_size->setValue(settings.value("spinBox_fft_delay_size",51).toInt());
    ui->spinBox_audio_out_delayline_size_in_blocks->setValue(settings.value("spinBox_audio_out_delayline_size_in_blocks",92).toInt());
    ui->spinBox_snr_estimate_delayline_size_in_blocks->setValue(settings.value("spinBox_snr_estimate_delayline_size_in_blocks",62).toInt());

    //other
    ui->doubleSpinBox_hysteresis_db->setValue(settings.value("doubleSpinBox_hysteresis_db",2.0).toDouble());
    ui->doubleSpinBox_snr_threshold_level_db->setValue(settings.value("doubleSpinBox_snr_threshold_level_db",7.0).toDouble());
    ui->checkBox_agc->setChecked(settings.value("checkBox_agc",true).toBool());
    ui->lineEdit_audiopath->setText(settings.value("lineEdit_audiopath","~/Documents/JSquelch/").toString());
    ui->checkBox_save_audio->setChecked(settings.value("checkBox_save_audio",false).toBool());
    ui->spinBox_snr_streching_in_blocks->setValue(settings.value("spinBox_snr_streching_in_blocks",0).toInt());
    ui->soundcard_input->setCurrentText(settings.value("soundcard_input").toString());
    ui->doubleSpinBox_silence_before_file_split_in_s->setValue(settings.value("doubleSpinBox_silence_before_file_split_in_s",86400.0).toDouble());
    ui->checkBox_split_audio_files->setChecked(settings.value("checkBox_split_audio_files",false).toBool());

    //noise estimator
    ui->spinBox_mne_moving_stats_window_size->setValue(settings.value("spinBox_mne_moving_stats_window_size",16).toInt());
    ui->spinBox_mne_moving_minimum_window_size->setValue(settings.value("spinBox_mne_moving_minimum_window_size",16).toInt());
    ui->spinBox_mne_output_moving_average_window_size->setValue(settings.value("spinBox_mne_output_moving_average_window_size",62).toInt());

    //signal estimator
    ui->spinBox_mse_max_voice_bin->setValue(settings.value("spinBox_mse_max_voice_bin",96).toInt());
    ui->spinBox_mse_min_voice_bin->setValue(settings.value("spinBox_mse_min_voice_bin",3).toInt());
    ui->spinBox_mse_moving_stats_window_size->setValue(settings.value("spinBox_mse_moving_stats_window_size",8).toInt());
    ui->spinBox_mse_output_moving_snr_estimate_window_size->setValue(settings.value("spinBox_mse_output_moving_snr_estimate_window_size",62).toInt());
}

void JSquelch::saveSettings()
{
    QSettings settings("JontiSoft",settings_name);

    //delay compensation
    settings.setValue("spinBox_fft_delay_size",ui->spinBox_fft_delay_size->value());
    settings.setValue("spinBox_audio_out_delayline_size_in_blocks",ui->spinBox_audio_out_delayline_size_in_blocks->value());
    settings.setValue("spinBox_snr_estimate_delayline_size_in_blocks",ui->spinBox_snr_estimate_delayline_size_in_blocks->value());

    //other
    settings.setValue("doubleSpinBox_hysteresis_db",ui->doubleSpinBox_hysteresis_db->value());
    settings.setValue("doubleSpinBox_snr_threshold_level_db",ui->doubleSpinBox_snr_threshold_level_db->value());
    settings.setValue("checkBox_agc",ui->checkBox_agc->isChecked());
    settings.setValue("lineEdit_audiopath",ui->lineEdit_audiopath->text());
    settings.setValue("checkBox_save_audio",ui->checkBox_save_audio->isChecked());
    settings.setValue("spinBox_snr_streching_in_blocks",ui->spinBox_snr_streching_in_blocks->value());
    settings.setValue("soundcard_input",ui->soundcard_input->currentText());
    settings.setValue("doubleSpinBox_silence_before_file_split_in_s",ui->doubleSpinBox_silence_before_file_split_in_s->value());
    settings.setValue("checkBox_split_audio_files",ui->checkBox_split_audio_files->isChecked());

    //noise estimator
    settings.setValue("spinBox_mne_moving_stats_window_size",ui->spinBox_mne_moving_stats_window_size->value());
    settings.setValue("spinBox_mne_moving_minimum_window_size",ui->spinBox_mne_moving_minimum_window_size->value());
    settings.setValue("spinBox_mne_output_moving_average_window_size",ui->spinBox_mne_output_moving_average_window_size->value());

    //signal estimator
    settings.setValue("spinBox_mse_max_voice_bin",ui->spinBox_mse_max_voice_bin->value());
    settings.setValue("spinBox_mse_min_voice_bin",ui->spinBox_mse_min_voice_bin->value());
    settings.setValue("spinBox_mse_moving_stats_window_size",ui->spinBox_mse_moving_stats_window_size->value());
    settings.setValue("spinBox_mse_output_moving_snr_estimate_window_size",ui->spinBox_mse_output_moving_snr_estimate_window_size->value());
}

void JSquelch::closeEvent (QCloseEvent *event)
{
    Q_UNUSED(event);
}

JSquelch::~JSquelch()
{
    audioLoopback.stop();
    saveSettings();
    delete ui;
}

void JSquelch::on_soundcard_input_currentIndexChanged(const QString &deviceName)
{
    foreach (const QAudioDeviceInfo &deviceInfo, QAudioDeviceInfo::availableDevices(QAudio::AudioInput))
    {
        if(deviceInfo.deviceName()==deviceName)
        {
            AudioLoopback::Settings loopbacksettings=audioLoopback.getSettings();
            loopbacksettings.audio_device_in=deviceInfo;
            audioLoopback.setSettings(loopbacksettings);
            audioLoopback.start();
            break;
        }
    }
}

void JSquelch::on_checkBox_split_audio_files_clicked()
{
    ui->doubleSpinBox_silence_before_file_split_in_s->setEnabled(ui->checkBox_split_audio_files->isChecked());
}
