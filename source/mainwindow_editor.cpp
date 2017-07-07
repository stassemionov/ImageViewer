#include "service.h"
#include <mainwindow.h>
#include "ui_mainwindow.h"

#include <QTransform>
#include <omp.h>
#include <cmath>

#include <QDebug>
#include <QPainter>

void MainWindow::onUndo()
{
    QSharedPointer<QImage> prev_image = m_edit_history->back();
    if (prev_image.isNull())
    {
        return;
    }
    m_intermediate_image = prev_image;
    m_showed_image.reset(new QImage(prev_image->copy()));
    this->updateView();
    this->setSavedStatus(false);
    this->updateUndoRedoStatus();
}

void MainWindow::onRedo()
{
    QSharedPointer<QImage> prev_image = m_edit_history->forward();
    if (prev_image.isNull())
    {
        return;
    }
    m_intermediate_image = prev_image;
    m_showed_image.reset(new QImage(prev_image->copy()));
    this->updateView();
    this->setSavedStatus(false);
    this->updateUndoRedoStatus();
}

void MainWindow::onHistoryJump(int index)
{
    if (!m_edit_history->jumpToVersion(index))
    {
        return;
    }
    QSharedPointer<QImage> pix = m_edit_history->getCurrentVersion();
    m_intermediate_image = pix;
    m_showed_image.reset(new QImage(pix->copy()));
    this->updateView();
    this->updateUndoRedoStatus();
}

void MainWindow::onRotateLeft()
{
    if (m_intermediate_image.isNull())
    {
        return;
    }

    ui->edit_accuracy_low_radioButton->setChecked(true);
    int v = ui->edit_rotate_dial->value() - 90;
    ui->edit_rotate_dial->setValue((v >= 0) ? v : (360+v));
}

void MainWindow::onRotateRight()
{
    if (m_intermediate_image.isNull())
    {
        return;
    }

    ui->edit_accuracy_low_radioButton->setChecked(true);
    int v = ui->edit_rotate_dial->value() + 90;
    ui->edit_rotate_dial->setValue((v < 360) ? v : (v-360));
}

void MainWindow::onRotate(int value)
{
    if (m_intermediate_image.isNull())
    {
        return;
    }

    if (ui->edit_accuracy_high_radioButton->isChecked())
    {
        // Convert dial angle to real angle
        double new_angle = m_bisectr_angle + (value - 180) / 4.0;
        m_angle = (new_angle < 360) ?
                    ((new_angle >= 0) ? new_angle : (360 + new_angle)) :
                    (new_angle - 360);
        ui->edit_rotate_angle_lcd->display(
            m_angle <= 180 ? m_angle : (m_angle - 360));
    }
    else
    {
        // Convert dial angle to real angle
        m_angle = (value <= 180) ? (value + 180) : (value - 180);
        ui->edit_rotate_angle_lcd->display(value - 180);
    }

    QTransform transform;
    m_showed_image.reset(new QImage(m_intermediate_image->
                transformed(transform.rotate(m_angle))));
    m_screne_label.setPixmap(QPixmap::fromImage(*m_showed_image));
    this->updateScale();
    this->setSavedStatus(false);
}

void MainWindow::onRestoreOriginalAngle()
{
    if (m_showed_image.isNull())
    {
        return;
    }

    if (m_angle != 0.0 && m_angle != 360.0)
    {
        if (ui->edit_accuracy_low_radioButton->isChecked())
        {
            ui->edit_rotate_dial->setValue(180);
        }
        else
        {
            m_angle = 0.0;
            // Generates rotation.
            ui->edit_accuracy_low_radioButton->setChecked(true);
        }
    }
    else
    {
        // Explicit call of scale updating.
        this->updateScale();
    }
}

void MainWindow::onBrightnessInc()
{
    onBrightnessEdited(1);
    this->writeActionName(tr("Brightness + 1"));
}

void MainWindow::onSaturationInc()
{
    onSaturationEdited(1);
    this->writeActionName(tr("Saturation + 1"));
}

void MainWindow::onRedInc()
{
    onRedEdited(1);
    this->writeActionName(tr("Red colour + 1"));
}

void MainWindow::onGreenInc()
{
    onGreenEdited(1);
    this->writeActionName(tr("Green colour + 1"));
}

void MainWindow::onBlueInc()
{
    onBlueEdited(1);
    this->writeActionName(tr("Blue colour + 1"));
}

void MainWindow::onBrightnessDec()
{
    onBrightnessEdited(-1);
    this->writeActionName(tr("Brightness - 1"));
}

void MainWindow::onSaturationDec()
{
    onSaturationEdited(-1);
    this->writeActionName(tr("Saturation - 1"));
}

void MainWindow::onRedDec()
{
    onRedEdited(-1);
    this->writeActionName(tr("Red colour - 1"));
}

void MainWindow::onGreenDec()
{
    onGreenEdited(-1);
    this->writeActionName(tr("Green colour - 1"));
}

void MainWindow::onBlueDec()
{
    onBlueEdited(-1);
    this->writeActionName(tr("Blue colour - 1"));
}

void MainWindow::onBrightnessDoubleInc()
{
    onBrightnessEdited(10);
    this->writeActionName(tr("Brightness + 10"));
}

void MainWindow::onSaturationDoubleInc()
{
    onSaturationEdited(10);
    this->writeActionName(tr("Saturation + 10"));
}

void MainWindow::onRedDoubleInc()
{
    onRedEdited(10);
    this->writeActionName(tr("Red colour + 10"));
}

void MainWindow::onGreenDoubleInc()
{
    onGreenEdited(10);
    this->writeActionName(tr("Green colour + 10"));
}

void MainWindow::onBlueDoubleInc()
{
    onBlueEdited(10);
    this->writeActionName(tr("Blue colour + 10"));
}

void MainWindow::onBrightnessDoubleDec()
{
    onBrightnessEdited(-10);
    this->writeActionName(tr("Red colour - 10"));
}

void MainWindow::onSaturationDoubleDec()
{
    onSaturationEdited(-10);
    this->writeActionName(tr("Saturation - 10"));
}

void MainWindow::onRedDoubleDec()
{
    onRedEdited(-10);
    this->writeActionName(tr("Red colour - 10"));
}

void MainWindow::onGreenDoubleDec()
{
    onGreenEdited(-10);
    this->writeActionName(tr("Green colour - 10"));
}

void MainWindow::onBlueDoubleDec()
{
    onBlueEdited(-10);
    this->writeActionName(tr("Blue colour - 10"));
}

void MainWindow::onRedEdited(int dif)
{
    if (m_intermediate_image.isNull())
    {
        return;
    }

    convertToColoured(*m_intermediate_image);

    const int w = m_intermediate_image->width();
    const int h = m_intermediate_image->height();
    m_showed_image.clear();
    m_showed_image.reset(new QImage(m_intermediate_image->size(),
                           m_intermediate_image->format()));

    QRgb* src_colors_line = reinterpret_cast<QRgb*>(
            m_intermediate_image->scanLine(0));
    QRgb* dst_colors_line = reinterpret_cast<QRgb*>(
            m_showed_image->scanLine(0));

    #pragma omp parallel for schedule(dynamic, (w*h)/omp_get_num_threads())
    for (int i = 0; i < w*h; ++i)
    {
        QRgb col = src_colors_line[i];
        int r = qRed(col) + dif;
        int g = qGreen(col);
        int b = qBlue(col);
        dst_colors_line[i] = qRgba(
                    (r > 255) ? 255 : ((r < 0) ? 0 : r),
                    g, b, qAlpha(col));
    }

    m_edit_history->add(*m_showed_image);
    m_intermediate_image.clear();
    m_intermediate_image.reset(new QImage(m_showed_image->copy()));
    this->updateView();
    this->setSavedStatus(false);
    this->updateUndoRedoStatus();
}

void MainWindow::onGreenEdited(int dif)
{
    if (m_intermediate_image.isNull())
    {
        return;
    }

    convertToColoured(*m_intermediate_image);

    const int w = m_intermediate_image->width();
    const int h = m_intermediate_image->height();
    m_showed_image.clear();
    m_showed_image.reset(new QImage(m_intermediate_image->size(),
                           m_intermediate_image->format()));

    QRgb* src_colors_line = reinterpret_cast<QRgb*>(
            m_intermediate_image->scanLine(0));
    QRgb* dst_colors_line = reinterpret_cast<QRgb*>(
            m_showed_image->scanLine(0));

    #pragma omp parallel for schedule(dynamic, (w*h)/omp_get_num_threads())
    for (int i = 0; i < w*h; ++i)
    {
        QRgb col = src_colors_line[i];
        int r = qRed(col);
        int g = qGreen(col) + dif;
        int b = qBlue(col);
        dst_colors_line[i] = qRgba(
                    r,
                    (g > 255) ? 255 : ((g < 0) ? 0 : g),
                    b, qAlpha(col));
    }

    m_edit_history->add(*m_showed_image);
    m_intermediate_image.clear();
    m_intermediate_image.reset(new QImage(m_showed_image->copy()));
    this->updateView();
    this->setSavedStatus(false);
    this->updateUndoRedoStatus();
}

void MainWindow::onBlueEdited(int dif)
{
    if (m_intermediate_image.isNull())
    {
        return;
    }

    convertToColoured(*m_intermediate_image);

    const int w = m_intermediate_image->width();
    const int h = m_intermediate_image->height();
    m_showed_image.clear();
    m_showed_image.reset(new QImage(m_intermediate_image->size(),
                           m_intermediate_image->format()));

    QRgb* src_colors_line = reinterpret_cast<QRgb*>(
            m_intermediate_image->scanLine(0));
    QRgb* dst_colors_line = reinterpret_cast<QRgb*>(
            m_showed_image->scanLine(0));

    #pragma omp parallel for schedule(dynamic, (w*h)/omp_get_num_threads())
    for (int i = 0; i < w*h; ++i)
    {
        QRgb col = src_colors_line[i];
        int r = qRed(col);
        int g = qGreen(col);
        int b = qBlue(col) + dif;
        dst_colors_line[i] = qRgba(
                    r, g,
                    (b > 255) ? 255 : ((b < 0) ? 0 : b),
                    qAlpha(col));
    }

    m_edit_history->add(*m_showed_image);
    m_intermediate_image.clear();
    m_intermediate_image.reset(new QImage(m_showed_image->copy()));
    this->updateView();
    this->setSavedStatus(false);
    this->updateUndoRedoStatus();
}

void MainWindow::onSaturationEdited(int dif)
{
    if (m_intermediate_image.isNull())
    {
        return;
    }

    const int w = m_intermediate_image->width();
    const int h = m_intermediate_image->height();
    m_showed_image.clear();
    m_showed_image.reset(new QImage(m_intermediate_image->size(),
                           m_intermediate_image->format()));

    #pragma omp parallel for schedule(dynamic, (w*h)/omp_get_num_threads())
    for (int i = 0; i < w*h; ++i)
    {
        int row = i / w, col = i % w;
        QColor pix_col = m_intermediate_image->pixelColor(col, row);
        int alpha = pix_col.alpha();
        if (alpha > 0)
        {
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
            pix_col.setAlpha(alpha);
        }
        m_showed_image->setPixelColor(col, row, pix_col);
    }

    m_edit_history->add(*m_showed_image);
    m_intermediate_image.clear();
    m_intermediate_image.reset(new QImage(m_showed_image->copy()));
    this->updateView();
    this->setSavedStatus(false);
    this->updateUndoRedoStatus();
}

void MainWindow::onBrightnessEdited(int dif)
{
    if (m_intermediate_image.isNull())
    {
        return;
    }

    const int w = m_intermediate_image->width();
    const int h = m_intermediate_image->height();
    m_showed_image.clear();
    m_showed_image.reset(new QImage(m_intermediate_image->size(),
                           m_intermediate_image->format()));

    QRgb* src_colors_line = reinterpret_cast<QRgb*>(
            m_intermediate_image->scanLine(0));
    QRgb* dst_colors_line = reinterpret_cast<QRgb*>(
            m_showed_image->scanLine(0));

    #pragma omp parallel for schedule(dynamic, (w*h)/omp_get_num_threads())
    for (int i = 0; i < w*h; ++i)
    {
        QRgb col = src_colors_line[i];
        int r = qRed(col)   + dif;
        int g = qGreen(col) + dif;
        int b = qBlue(col)  + dif;
        dst_colors_line[i] = qRgba(
                    (r > 255) ? 255 : ((r < 0) ? 0 : r),
                    (g > 255) ? 255 : ((g < 0) ? 0 : g),
                    (b > 255) ? 255 : ((b < 0) ? 0 : b),
                    qAlpha(col));
    }

    m_edit_history->add(*m_showed_image);
    m_intermediate_image.clear();
    m_intermediate_image.reset(new QImage(m_showed_image->copy()));
    this->updateView();
    this->setSavedStatus(false);
    this->updateUndoRedoStatus();
}

void MainWindow::onUncolourized()
{
    if (m_intermediate_image.isNull())
    {
        return;
    }

    const int w = m_intermediate_image->width();
    const int h = m_intermediate_image->height();
    m_showed_image.clear();
    m_showed_image.reset(new QImage(m_intermediate_image->size(),
                           m_intermediate_image->format()));

    #pragma omp parallel for schedule(dynamic, (w*h)/omp_get_num_threads())
    for (int i = 0; i < w*h; ++i)
    {
        int row = i / w, col = i % w;
        QColor pix_col = m_intermediate_image->pixelColor(col, row);
        int alpha = pix_col.alpha();
        if (alpha > 0)
        {
            pix_col.setHsv(pix_col.hue(), 0, pix_col.value());
            pix_col.setAlpha(alpha);
        }
        m_showed_image->setPixelColor(col, row, pix_col);
    }

  //  QRgb* src_colors_line = reinterpret_cast<QRgb*>(
  //          m_intermediate_image->scanLine(0));
  //  QRgb* dst_colors_line = reinterpret_cast<QRgb*>(
  //          m_showed_image->scanLine(0));
  //
  //  #pragma omp parallel for schedule(dynamic, (w*h)/omp_get_num_threads())
  //  for (int i = 0; i < w*h; ++i)
  //  {
  //      // (11B+30R+59G)/100 )
  //      QRgb col = src_colors_line[i];
  //      int r = 30 * qRed(col);
  //      int g = 59 * qGreen(col);
  //      int b = 11 * qBlue(col);
  //      int s = (r + g + b) / 100;
  //      dst_colors_line[i] = qRgba(s,s,s,
  //                  qAlpha(col));
  //  }

    m_edit_history->add(*m_showed_image);
    m_intermediate_image.clear();
    m_intermediate_image.reset(new QImage(m_showed_image->copy()));
    this->updateView();
    this->setSavedStatus(false);
    this->updateUndoRedoStatus();
    this->writeActionName(tr("Uncolourization"));
}

void MainWindow::onNegatived()
{
    if (m_intermediate_image.isNull())
    {
        return;
    }

    m_intermediate_image->invertPixels();
    m_edit_history->add(*m_intermediate_image);
    m_showed_image.clear();
    m_showed_image.reset(new QImage(m_intermediate_image->copy()));
    this->updateView();
    this->setSavedStatus(false);
    this->updateUndoRedoStatus();
    this->writeActionName(tr("Negative"));
}

void MainWindow::onSmoothing()
{
    if (m_intermediate_image.isNull())
    {
        return;
    }

    convertToColoured(*m_intermediate_image);

    const int w = m_intermediate_image->width();
    const int h = m_intermediate_image->height();
    m_showed_image.clear();
    m_showed_image.reset(new QImage(m_intermediate_image->size(),
                           m_intermediate_image->format()));

    QRgb* src_colors_line = reinterpret_cast<QRgb*>(
            m_intermediate_image->scanLine(0));
    QRgb* dst_colors_line = reinterpret_cast<QRgb*>(
            m_showed_image->scanLine(0));

//    #pragma omp parallel for schedule(dynamic, (w*h)/omp_get_num_threads())
    for (int i = 1; i < (h-1); ++i)
    {
        for (int j = 1; j < (w-1); ++j)
        {
            if (qAlpha(src_colors_line[i*w+j]) == 0)
            {
                continue;
            }

            /*

       //     QRgb col11 = src_colors_line[(i-1)*w+j-1];
            QRgb col12 = src_colors_line[(i-1)*w+j];
       //     QRgb col13 = src_colors_line[(i-1)*w+j+1];
            QRgb col21 = src_colors_line[i*w+j-1];
            QRgb col22 = src_colors_line[i*w+j];
            QRgb col23 = src_colors_line[i*w+j+1];
     //       QRgb col31 = src_colors_line[(i+1)*w+j-1];
            QRgb col32 = src_colors_line[(i+1)*w+j];
     //       QRgb col33 = src_colors_line[(i+1)*w+j+1];

            int r = (qRed(col12) +
                     qRed(col21) + qRed(col22) + qRed(col23) +
                      qRed(col32) ) / 5;
            int g = (qGreen(col12) +
                     qGreen(col21) + qGreen(col22) + qGreen(col23) +
                     qGreen(col32) ) / 5;
            int b = (qBlue(col12) +
                     qBlue(col21) + qBlue(col22) + qBlue(col23) +
                     qBlue(col32)) / 5;
        */

            QRgb col11 = src_colors_line[(i-1)*w+j-1];
            QRgb col12 = src_colors_line[(i-1)*w+j];
            QRgb col13 = src_colors_line[(i-1)*w+j+1];
            QRgb col21 = src_colors_line[i*w+j-1];
            QRgb col22 = src_colors_line[i*w+j];
            QRgb col23 = src_colors_line[i*w+j+1];
            QRgb col31 = src_colors_line[(i+1)*w+j-1];
            QRgb col32 = src_colors_line[(i+1)*w+j];
            QRgb col33 = src_colors_line[(i+1)*w+j+1];

            int r = (qRed(col11) + qRed(col12) + qRed(col13) +
                     qRed(col21) + qRed(col22) + qRed(col23) +
                     qRed(col31) + qRed(col32) + qRed(col33)) / 9;
            int g = (qGreen(col11) + qGreen(col12) + qGreen(col13) +
                     qGreen(col21) + qGreen(col22) + qGreen(col23) +
                     qGreen(col31) + qGreen(col32) + qGreen(col33)) / 9;
            int b = (qBlue(col11) + qBlue(col12) + qBlue(col13) +
                     qBlue(col21) + qBlue(col22) + qBlue(col23) +
                     qBlue(col31) + qBlue(col32) + qBlue(col33)) / 9;
            dst_colors_line[i*w+j] = qRgba(r, g, b, qAlpha(col22));
        }
    }

    m_edit_history->add(*m_showed_image);
    m_intermediate_image.clear();
    m_intermediate_image.reset(new QImage(m_showed_image->copy()));
    this->updateView();
    this->setSavedStatus(false);
    this->updateUndoRedoStatus();
    this->writeActionName(tr("Smoothing"));
}

void MainWindow::onCustomFilterApplied()
{
    const int w = m_intermediate_image->width();
    const int h = m_intermediate_image->height();
    m_showed_image.clear();
    m_showed_image.reset(new QImage(m_intermediate_image->size(),
                           m_intermediate_image->format()));

    QRgb* src_colors_line = reinterpret_cast<QRgb*>(
            m_intermediate_image->scanLine(0));
    QRgb* dst_colors_line = reinterpret_cast<QRgb*>(
            m_showed_image->scanLine(0));

    int mat_size = m_filter_customizer->getMatrixSize();
    int border = mat_size / 2;
    const QVector<double>& mat_data = m_filter_customizer->getMatrixData();

    for (int i = border; i < (h-border); ++i)
    {
        for (int j = border; j < (w-border); ++j)
        {
            int alpha = qAlpha(src_colors_line[i*w+j]);
            if (alpha == 0)
            {
                continue;
            }

            RgbColor dst_col{0.0, 0.0, 0.0};
            for (int ii = -border; ii <= border; ++ii)
            {
                for (int jj = -border; jj <= border; ++jj)
                {
                    double coef = mat_data[(ii+border)*mat_size + (jj+border)];
                    QRgb src_col = src_colors_line[(i+ii)*w+(j+jj)];
                    dst_col.r += qRed(src_col) * coef;
                    dst_col.g += qGreen(src_col) * coef;
                    dst_col.b += qBlue(src_col) * coef;
                }
            }
            dst_colors_line[i*w+j] = qRgba(
                ((dst_col.r < 255) ? ((dst_col.r >= 0) ? dst_col.r : 0) : 255),
                ((dst_col.g < 255) ? ((dst_col.g >= 0) ? dst_col.g : 0) : 255),
                ((dst_col.b < 255) ? ((dst_col.b >= 0) ? dst_col.b : 0) : 255),
                alpha);
        }
    }

    m_edit_history->add(*m_showed_image);
    m_intermediate_image.clear();
    m_intermediate_image.reset(new QImage(m_showed_image->copy()));
    this->updateView();
    this->setSavedStatus(false);
    this->updateUndoRedoStatus();
    this->writeActionName(tr("Custom filter"));
}
