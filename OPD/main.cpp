#include <QtGui/QApplication>
#include <QTranslator>

#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QTranslator translator;
    translator.load("./opd_lv");
    a.installTranslator(&translator);

    MainWindow w;
    w.show();

    return a.exec();
}
