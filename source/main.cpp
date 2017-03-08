#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    if (QCoreApplication::organizationName().isEmpty())
    {
        QCoreApplication::setOrganizationName(QString::fromUtf8("MyComputer"));
    }
    if (QCoreApplication::applicationName().isEmpty())
    {
        QCoreApplication::setApplicationName(QString::fromUtf8("ImageViewer"));
    }

    MainWindow w;
    w.show();

    return a.exec();
}
