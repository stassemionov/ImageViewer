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

// * Problem with history
// * Named list of history
// * Cutting
// * Settings (window):
//      => Load recent image at start
//      => Step of scaling
//      => Lenght of recent images list
// * Configuration of editor's buttons (active/disactive)
// * Turning between current scale and starting scale
// * Reflections
// * Pipetka!
// * Lastik!
