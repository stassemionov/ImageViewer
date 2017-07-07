#ifndef SERVICE_H
#define SERVICE_H

#include <QString>
#include <QImage>

typedef struct RgbColor
{
    double r;
    double g;
    double b;
} RgbColor;

QString loadTextFileData(const QString& filepath);

// Convert input image to format RGB32 if it has uncoloured format.
void convertToColoured(QImage& image);

#endif // SERVICE_H
