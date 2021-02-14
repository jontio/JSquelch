#include "jsquelch.h"
#include "ui_jsquelch.h"
#include <iostream>
#include <QtDebug>
#include <limits>

using namespace std;


JSquelch::JSquelch(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::JSquelch)
{
    ui->setupUi(this);


    QVector<double> x={2,3,1,4};
//    JDsp::MovingNoiseEstimator mne(257,16,16,32);
    JDsp::MovingNoiseEstimator mne(4,3,3,0);
    for(int k=0;k<6;k++)
    {
        mne<<x;
        qDebug()<<mne;
    }

}

JSquelch::~JSquelch()
{
    delete ui;
}

