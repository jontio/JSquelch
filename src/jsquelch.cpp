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

    JDsp::VectorMovingVariance<double> mv(QPair<int,int>(4,5));
    JDsp::VectorMovingMin<double> mm(QPair<int,int>(4,5));
    JDsp::VectorMovingMax<double> mM(QPair<int,int>(4,5));


    QVector<double> x={5,3,6,1};
    qDebug()<<mm<<mM<<mv<<mv.mean();
    for(int k=0;k<10;k++)
    {
        mm<<x;
        mM<<x;
        mv<<x;
        qDebug()<<mm<<mM<<mv<<mv.mean();
        x[0]+=1;
        x[1]+=2;
    }



}

JSquelch::~JSquelch()
{
    delete ui;
}

