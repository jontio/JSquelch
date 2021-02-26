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



using namespace std;

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

    //turn on led and display snr
    ui->led_snr->On(audio_on_state);
    ui->label_snr->setText(QString::asprintf("%0.1f",algo.snr_db)+"dB");



    if((audio_on_state)&&enc)
    {

        QVector<opus_int16> buf;
        buf.resize(output.size());
        for(int k=0;k<buf.size();k++)
        {
            buf[k]=((double)output[k])*32768.0;
        }
        ope_encoder_write(enc, buf.data(), buf.size());
    }

}

JSquelch::JSquelch(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::JSquelch)
{
    ui->setupUi(this);

//    diskwriter=new CompressedAudioDiskWriter(this);

    enc=nullptr;
    comments = ope_comments_create();
    ope_comments_add(comments, "ARTIST", "Someone");
    ope_comments_add(comments, "TITLE", "Some track");
    int error;
    enc = ope_encoder_create_file("delme.ogg", comments, 8000, 1, 0, &error);


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

    connect(&audioLoopback,&AudioLoopback::processAudio,this,&JSquelch::processAudio,Qt::DirectConnection);
    applySettings();
    audioLoopback.start();
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

void JSquelch::applySettings()
{
    VoiceDetectionAlgo::Settings settings=algo.getSettings();

    //delay compensation
    settings.fft_delay_size=ui->spinBox_fft_delay_size->value();
    settings.audio_out_delayline_size_in_blocks=ui->spinBox_audio_out_delayline_size_in_blocks->value();
    settings.snr_estimate_delayline_size_in_blocks=ui->spinBox_snr_estimate_delayline_size_in_blocks->value();

    //other
    hysteresis_db=ui->doubleSpinBox_hysteresis_db->value();
    threshold_level_db=ui->doubleSpinBox_snr_threshold_level_db->value();

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

#include <QMessageBox>
void JSquelch::closeEvent (QCloseEvent *event)
{
//    QMessageBox::StandardButton resBtn = QMessageBox::question( this, "APP_NAME",
//                                                                tr("Are you sure?\n"),
//                                                                QMessageBox::Cancel | QMessageBox::No | QMessageBox::Yes,
//                                                                QMessageBox::Yes);
//    if (resBtn != QMessageBox::Yes) {
//        event->ignore();
//    } else {
//        event->accept();
//    }
}

JSquelch::~JSquelch()
{
    audioLoopback.stop();
    if(enc)
    {
        ope_encoder_drain(enc);
        ope_encoder_destroy(enc);
        ope_comments_destroy(comments);
        enc=nullptr;
    }
    delete ui;
}

