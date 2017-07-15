#ifndef SERVICE_H
#define SERVICE_H

#include <QString>
#include <QImage>

namespace ImageViewerService
{

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

// Converts list of values of type T to list of QVariant.
template<template<class> class Cont, class T>
QList<QVariant> toListOfVariant(const Cont<T>& list)
{
    QList<QVariant> result;
    foreach (T val, list)
    {
        result << QVariant(val);
    }
    return result;
}

}

#endif // SERVICE_H
