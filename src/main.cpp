#include "jsquelch.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    JSquelch w;
    w.show();
    return a.exec();
}
