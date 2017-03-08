#include <service.h>

#include <QTextStream>
#include <QFile>

QString loadTextFileData(const QString& filepath)
{
    QFile file{filepath};
    file.open(QIODevice::ReadOnly);
    QTextStream stream{&file};
    QString result = stream.readAll();
    file.close();
    return result;
}
