#ifndef QLED_H
#define QLED_H

#include <QObject>
#include <QIcon>
#include <QLabel>

class QLed : public QLabel
{
    Q_OBJECT
public:
    explicit QLed(QWidget *parent = 0);
    void setLED( QIcon::State state = QIcon::Off, QIcon::Mode mode = QIcon::Normal );
    ~QLed();
    QIcon icon;
public slots:
    void On(bool on)
    {
        if(on)setLED(QIcon::On);
         else setLED(QIcon::Off);
    }
};

#endif // QLED_H
