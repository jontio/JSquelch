#include "jsquelch.h"

#include <QApplication>
#include <QCommandLineParser>
#include "jsquelch.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QApplication::setApplicationName("JSquelch");

    QCommandLineParser cmdparser;
    cmdparser.setApplicationDescription("Records audio to files when present");
    cmdparser.addHelpOption();

    QCommandLineOption settingsnameoption(QStringList() << "s" << "settings-name",QApplication::translate("main", "Run with setting name <name>."),QApplication::translate("main", "name"));
    settingsnameoption.setDefaultValue("");
    cmdparser.addOption(settingsnameoption);

    cmdparser.process(a);

    settings_name=cmdparser.value(settingsnameoption).trimmed();
    if(settings_name.isEmpty())
    {
        QApplication::setApplicationDisplayName("JSquelch");
        settings_name="JSquelch";
    }
    else
    {
        QApplication::setApplicationDisplayName(settings_name);
        settings_name="JSquelch ["+settings_name+"]";
    }

    JSquelch w;
    w.show();
    return a.exec();
}
