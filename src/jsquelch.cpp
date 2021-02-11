#include "jsquelch.h"
#include "ui_jsquelch.h"
#include <iostream>
#include <QtDebug>

using namespace std;


JSquelch::JSquelch(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::JSquelch)
{
    ui->setupUi(this);

    JDsp::VectorMovingVariance<double> mv(QPair<int,int>(4,5));

    QVector<double> input={1,2,3,4};
    for(int k=0;k<10;k++)
    {
        mv<<input;
        qDebug()<<mv.mean()<<mv;
        for(int i=0;i<2;i++)input[i]+=1;
    }



}

JSquelch::~JSquelch()
{
    delete ui;
}

