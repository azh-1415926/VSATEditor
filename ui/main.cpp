#include "mainwindow.h"

#include <QApplication>
#include <QFile>

#include "azh/version.hpp"
#include "utils.hpp"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    qApp->setApplicationVersion(std2qstring(AZH_VERSION));

    QFile resFile(":/res/style.qss");
    if (resFile.open(QIODevice::ReadOnly))
        qApp->setStyleSheet(resFile.readAll());
    resFile.close();

    MainWindow w;
    w.setWindowIcon(QIcon(":/res/atrribute_table.png"));
    w.show();
    return a.exec();
}
