#include <service.h>

#include <QTextStream>
#include <QFile>

#include <omp.h>

#include <QDebug>

namespace ImageViewerService
{

QString loadTextFileData(const QString& filepath)
{
    QFile file{filepath};
    file.open(QIODevice::ReadOnly | QIODevice::Text);
    QTextStream stream{&file};
    stream.setCodec("UTF-8");
    QString result = stream.readAll();
    file.close();
    return result;
}

void convertToColoured(QImage& image)
{
    if ((image.format() == QImage::Format_Grayscale8) ||
        (image.format() == QImage::Format_Mono) ||
        (image.format() == QImage::Format_MonoLSB) ||
        (image.format() == QImage::Format_Indexed8) /*||
        (image.format() == QImage::Format_Alpha8)*/)
    {
        image = image.convertToFormat(QImage::Format_RGB32);
    }
}

QImage getExpandedImage(const QImage& image, int dif)
{
    int w = image.width();
    int h = image.height();
    int new_w = w + 2*dif;
    int new_h = h + 2*dif;
    QImage new_image{new_w, new_h, image.format()};
    new_image.fill(Qt::white);

    const QRgb* src_colors_line = reinterpret_cast<const QRgb*>(
            image.scanLine(0));
    QRgb* dst_colors_line = reinterpret_cast<QRgb*>(
            new_image.scanLine(0));

    for (int i = 0; i < dif; ++i)
    {
        memcpy(dst_colors_line + i*new_w + dif,
               src_colors_line,
               w * sizeof(QRgb));
    }

    dst_colors_line = reinterpret_cast<QRgb*>(
                new_image.scanLine(dif));

    for (int i = 0; i < h; ++i)
    {
        QRgb col_left = src_colors_line[i*w];
        for (int j = 0; j < dif; ++j)
        {
            dst_colors_line[i*new_w + j] = col_left;
        }
        memcpy(dst_colors_line + i*new_w + dif,
               src_colors_line + i*w,
               w * sizeof(QRgb));
        QRgb col_right = src_colors_line[i*w + w - 1];
        for (int j = 0; j < dif; ++j)
        {
            dst_colors_line[i*new_w + dif + w + j] = col_right;
        }
    }

    src_colors_line = reinterpret_cast<const QRgb*>(
            image.scanLine(h-1));
    dst_colors_line = reinterpret_cast<QRgb*>(
                new_image.scanLine(dif + h));

    for (int i = 0; i < dif; ++i)
    {
        memcpy(dst_colors_line + i*new_w + dif,
               src_colors_line,
               w * sizeof(QRgb));
    }

    return new_image;
}

}
//typedef struct RgbColor
//{
//    unsigned char r;
//    unsigned char g;
//    unsigned char b;
//} RgbColor;
//
//typedef struct HsvColor
//{
//    unsigned char h;
//    unsigned char s;
//    unsigned char v;
//} HsvColor;
//
//RgbColor HsvToRgb(HsvColor hsv)
//{
//    RgbColor rgb;
//    unsigned char region, remainder, p, q, t;
//
//    if (hsv.s == 0)
//    {
//        rgb.r = hsv.v;
//        rgb.g = hsv.v;
//        rgb.b = hsv.v;
//        return rgb;
//    }
//
//    region = hsv.h / 43;
//    remainder = (hsv.h - (region * 43)) * 6;
//
//    p = (hsv.v * (255 - hsv.s)) >> 8;
//    q = (hsv.v * (255 - ((hsv.s * remainder) >> 8))) >> 8;
//    t = (hsv.v * (255 - ((hsv.s * (255 - remainder)) >> 8))) >> 8;
//
//    switch (region)
//    {
//        case 0:
//            rgb.r = hsv.v; rgb.g = t; rgb.b = p;
//            break;
//        case 1:
//            rgb.r = q; rgb.g = hsv.v; rgb.b = p;
//            break;
//        case 2:
//            rgb.r = p; rgb.g = hsv.v; rgb.b = t;
//            break;
//        case 3:
//            rgb.r = p; rgb.g = q; rgb.b = hsv.v;
//            break;
//        case 4:
//            rgb.r = t; rgb.g = p; rgb.b = hsv.v;
//            break;
//        default:
//            rgb.r = hsv.v; rgb.g = p; rgb.b = q;
//            break;
//    }
//
//    return rgb;
//}
//
//HsvColor RgbToHsv(RgbColor rgb)
//{
//    HsvColor hsv;
//    unsigned char rgbMin, rgbMax;
//
//    rgbMin = rgb.r < rgb.g ? (rgb.r < rgb.b ? rgb.r : rgb.b) : (rgb.g < rgb.b ? rgb.g : rgb.b);
//    rgbMax = rgb.r > rgb.g ? (rgb.r > rgb.b ? rgb.r : rgb.b) : (rgb.g > rgb.b ? rgb.g : rgb.b);
//
//    hsv.v = rgbMax;
//    if (hsv.v == 0)
//    {
//        hsv.h = 0;
//        hsv.s = 0;
//        return hsv;
//    }
//
//    hsv.s = 255 * long(rgbMax - rgbMin) / hsv.v;
//    if (hsv.s == 0)
//    {
//        hsv.h = 0;
//        return hsv;
//    }
//
//    if (rgbMax == rgb.r)
//        hsv.h = 0 + 43 * (rgb.g - rgb.b) / (rgbMax - rgbMin);
//    else if (rgbMax == rgb.g)
//        hsv.h = 85 + 43 * (rgb.b - rgb.r) / (rgbMax - rgbMin);
//    else
//        hsv.h = 171 + 43 * (rgb.r - rgb.g) / (rgbMax - rgbMin);
//
//    return hsv;
//}
