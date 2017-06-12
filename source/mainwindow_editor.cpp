#include <mainwindow.h>
#include "ui_mainwindow.h"

#include <QTransform>

#include <QDebug>

void MainWindow::onUndo()
{
    QImage* prev_image = m_edit_history->back();
    if (prev_image == nullptr)
    {
        return;
    }
    m_intermediate_image = *prev_image;
    this->updateScale();
}

void MainWindow::onRedo()
{
    QImage* prev_image = m_edit_history->forward();
    if (prev_image == nullptr)
    {
        return;
    }
    m_intermediate_image = *prev_image;
    this->updateScale();
}


void MainWindow::onRotateLeft()
{
    if (m_intermediate_image.isNull())
    {
        return;
    }

    QTransform transform;
    m_intermediate_image =
            m_intermediate_image.transformed(transform.rotate(-90));
    m_edit_history->add(m_intermediate_image);
    this->updateScale();
}

void MainWindow::onRotateRight()
{
    if (m_intermediate_image.isNull())
    {
        return;
    }

    QTransform transform;
    m_intermediate_image =
            m_intermediate_image.transformed(transform.rotate(90));
    m_edit_history->add(m_intermediate_image);
    this->updateScale();
}

void MainWindow::onBrightnessInc()
{
    onBrightnessEdited(1);
}

void MainWindow::onSaturationInc()
{
    onSaturationEdited(1);
}

void MainWindow::onRedInc()
{
    onRedEdited(1);
}

void MainWindow::onGreenInc()
{
    onGreenEdited(1);
}

void MainWindow::onBlueInc()
{
    onBlueEdited(1);
}

void MainWindow::onBrightnessDec()
{
    onBrightnessEdited(-1);
}

void MainWindow::onSaturationDec()
{
    onSaturationEdited(-1);
}

void MainWindow::onRedDec()
{
    onRedEdited(-1);
}

void MainWindow::onGreenDec()
{
    onGreenEdited(-1);
}

void MainWindow::onBlueDec()
{
    onBlueEdited(-1);
}

void MainWindow::onBrightnessDoubleInc()
{
    onBrightnessEdited(10);
}

void MainWindow::onSaturationDoubleInc()
{
    onSaturationEdited(10);
}

void MainWindow::onRedDoubleInc()
{
    onRedEdited(10);
}

void MainWindow::onGreenDoubleInc()
{
    onGreenEdited(10);
}

void MainWindow::onBlueDoubleInc()
{
    onBlueEdited(10);
}

void MainWindow::onBrightnessDoubleDec()
{
    onBrightnessEdited(-10);
}

void MainWindow::onSaturationDoubleDec()
{
    onSaturationEdited(-10);
}

void MainWindow::onRedDoubleDec()
{
    onRedEdited(-10);
}

void MainWindow::onGreenDoubleDec()
{
    onGreenEdited(-10);
}

void MainWindow::onBlueDoubleDec()
{
    onBlueEdited(-10);
}

void MainWindow::onRedEdited(int dif)
{
    if (m_intermediate_image.isNull())
    {
        return;
    }

    for (int i = 0; i < m_intermediate_image.height(); ++i)
    {
        QRgb* dst_colors_line = reinterpret_cast<QRgb*>(
                    m_intermediate_image.scanLine(i));
        for (int j = 0; j < m_intermediate_image.width(); ++j)
        {
            QRgb col = dst_colors_line[j];
            int r = qRed(col) + dif;
            int g = qGreen(col);
            int b = qBlue(col);
            dst_colors_line[j] = qRgba(
                        (r > 255) ? 255 : ((r < 0) ? 0 : r),
                        g, b, qAlpha(col));
        }
    }
    m_edit_history->add(m_intermediate_image);
    this->updateScale();
}

void MainWindow::onGreenEdited(int dif)
{
    if (m_intermediate_image.isNull())
    {
        return;
    }

    for (int i = 0; i < m_intermediate_image.height(); ++i)
    {
        QRgb* dst_colors_line = reinterpret_cast<QRgb*>(
                    m_intermediate_image.scanLine(i));
        for (int j = 0; j < m_intermediate_image.width(); ++j)
        {
            QRgb col = dst_colors_line[j];
            int r = qRed(col);
            int g = qGreen(col) + dif;
            int b = qBlue(col);
            dst_colors_line[j] = qRgba(
                        r,
                        (g > 255) ? 255 : ((g < 0) ? 0 : g),
                        b, qAlpha(col));
        }
    }
    m_edit_history->add(m_intermediate_image);
    this->updateScale();
}

void MainWindow::onBlueEdited(int dif)
{
    if (m_intermediate_image.isNull())
    {
        return;
    }

    for (int i = 0; i < m_intermediate_image.height(); ++i)
    {
        QRgb* dst_colors_line = reinterpret_cast<QRgb*>(
                    m_intermediate_image.scanLine(i));
        for (int j = 0; j < m_intermediate_image.width(); ++j)
        {
            QRgb col = dst_colors_line[j];
            int r = qRed(col);
            int g = qGreen(col);
            int b = qBlue(col) + dif;
            dst_colors_line[j] = qRgba(
                        r, g,
                        (b > 255) ? 255 : ((b < 0) ? 0 : b),
                        qAlpha(col));
        }
    }
    m_edit_history->add(m_intermediate_image);
    this->updateScale();
}

void MainWindow::onSaturationEdited(int dif)
{
    if (m_intermediate_image.isNull())
    {
        return;
    }

    for (int j = 0; j < m_intermediate_image.width(); ++j)
    {
        for (int i = 0; i < m_intermediate_image.height(); ++i)
        {
            QColor pix_col = m_intermediate_image.pixelColor(j, i);
            int h,s,v;
            pix_col.getHsv(&h, &s, &v);
            s += dif;
            if (s < 0)
            {
                s = 0;
            }
            else if (s > 255)
            {
                s = 255;
            }
            pix_col.setHsv(h, s, v);
            m_intermediate_image.setPixelColor(j, i, pix_col);
        }
    }
    m_edit_history->add(m_intermediate_image);
    this->updateScale();
}

void MainWindow::onBrightnessEdited(int dif)
{
    if (m_intermediate_image.isNull())
    {
        return;
    }

    for (int j = 0; j < m_intermediate_image.width(); ++j)
    {
        for (int i = 0; i < m_intermediate_image.height(); ++i)
        {
            QColor pix_col = m_intermediate_image.pixelColor(j, i);
            int h,s,v;
            pix_col.getHsv(&h, &s, &v);
            v += dif;
            if (v < 0)
            {
                v = 0;
            }
            else if (v > 255)
            {
                v = 255;
            }
            pix_col.setHsv(h, s, v);
            m_intermediate_image.setPixelColor(j, i, pix_col);
        }
    }
    m_edit_history->add(m_intermediate_image);
    this->updateScale();
}

void MainWindow::onUncolourized()
{
    if (m_intermediate_image.isNull())
    {
        return;
    }

    for (int j = 0; j < m_intermediate_image.width(); ++j)
    {
        for (int i = 0; i < m_intermediate_image.height(); ++i)
        {
            QColor pix_col = m_intermediate_image.pixelColor(j, i);
            pix_col.setHsv(pix_col.hue(), 0, pix_col.value());
            m_intermediate_image.setPixelColor(j, i, pix_col);
        }
    }
    m_edit_history->add(m_intermediate_image);
    this->updateScale();
}

void MainWindow::onNegatived()
{
    if (m_intermediate_image.isNull())
    {
        return;
    }

    m_intermediate_image.invertPixels();
    m_edit_history->add(m_intermediate_image);
    this->updateScale();
}
