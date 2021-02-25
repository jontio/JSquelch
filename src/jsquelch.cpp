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
        if(algo.snr_db<5.0)algo.fill(0);
        //add the audio to the output
        output+=algo;
    }

    //turn on led and display snr
    if(algo.snr_db>5)ui->led_snr->On(true);
    else ui->led_snr->On(false);
    ui->label_snr->setText(QString::asprintf("%0.1f",algo.snr_db)+"dB");

}

JSquelch::JSquelch(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::JSquelch)
{
    ui->setupUi(this);
    connect(&audioLoopback,&AudioLoopback::processAudio,this,&JSquelch::processAudio,Qt::DirectConnection);

    audioLoopback.start();
}

JSquelch::~JSquelch()
{
    delete ui;
}

