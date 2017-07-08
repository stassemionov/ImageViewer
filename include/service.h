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

// Creates new image with size [h + dif, w + dif].
// New areas are filled with border pixels.
QImage getExpandedImage(const QImage& image, int dif);

#endif // SERVICE_H
