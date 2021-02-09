#include "jsquelch.h"
#include "ui_jsquelch.h"

JSquelch::JSquelch(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::JSquelch)
{
    ui->setupUi(this);
}

JSquelch::~JSquelch()
{
    delete ui;
}

