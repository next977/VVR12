#include <QFontDatabase>
#include <QDebug>
#include <QFont>

#include <QApplication>
#include "global.h"
#include "gpiotools.h"
#include "mainsystem.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    qDebug(">> Start EP System ");
     int id3 = QFontDatabase::addApplicationFont(":/Fonts/starmedfont.ttf");
    mainsystem system;
    system.setIDstarmed(id3);
    system.doRun();
    system.show();
    return a.exec();
}
