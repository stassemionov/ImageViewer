#include "service.h"
#include <mainwindow.h>
#include "ui_mainwindow.h"

#include <QTransform>
#include <QPair>
#include <functional>
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
    this->setSavedStatus(
        m_is_saved && ((m_angle == 0.0) || (m_angle == 360.0)));
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

    ImageViewerService::convertToColoured(*m_intermediate_image);

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

    ImageViewerService::convertToColoured(*m_intermediate_image);

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

    ImageViewerService::convertToColoured(*m_intermediate_image);

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

    ImageViewerService::convertToColoured(*m_intermediate_image);

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

//    ImageViewerService::convertToColoured(*m_intermediate_image);

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

void MainWindow::onLinearSmoothing()
{
    if (m_intermediate_image.isNull())
    {
        return;
    }

    int radius = ui->edit_linear_blur_spinBox->value();
    int mat_size = 2 * radius + 1;
    double el = 1.0 / (mat_size * mat_size);
    QVector<double> mat_data(mat_size * mat_size);

    for (int i = 0; i < mat_size; ++i)
    {
        for (int j = 0; j < mat_size; ++j)
        {
            mat_data[i*mat_size+j] = el;
        }
    }
    this->applyMatrixFilter(mat_data, mat_size);
    this->writeActionName(tr("Linear blur") +
        QString::fromUtf8(" %1 px").
        arg(radius));
}

void MainWindow::onGaussFilterApplying()
{
    if (m_intermediate_image.isNull())
    {
        return;
    }

    int radius = ui->edit_gauss_radius_spinBox->value();
    int mat_size = 2 * radius + 1;
    double param = ui->edit_gauss_parameter_spinBox->value();
    double sigma2 = 2 * param * param;
    double lin_coef = 1.0 / (M_PIl * sigma2);
    double exp_coef = -1.0 / sigma2;
    QVector<double> mat_data(mat_size * mat_size);

    double sum = 0.0;
    for (int i = -radius; i <= radius; ++i)
    {
        for (int j = -radius; j <= radius; ++j)
        {
            double r2 = i*i + j*j;
            double val = lin_coef * exp(r2 * exp_coef);
            mat_data[(i+radius)*mat_size+(j+radius)] = val;
            sum += val;
        }
    }
    for (int i = 0; i < mat_data.size(); ++i)
    {
        mat_data[i] /= sum;
    }

    this->applyMatrixFilter(mat_data, mat_size);
    this->writeActionName(tr("Gauss filter") +
        QString::fromUtf8(" %1 px").arg(radius));
}

void MainWindow::onMedianFilterApplying()
{
    if (m_intermediate_image.isNull())
    {
        return;
    }

    ImageViewerService::convertToColoured(*m_intermediate_image);

    int radius = ui->edit_median_filter_spinBox->value();
    int mat_size = 2 * radius + 1;
    int median_index = (mat_size * mat_size) / 2;
    QVector<QPair<int,QRgb> > neigh_points(mat_size * mat_size);
    auto op_less = [](QPair<int,QRgb> p1, QPair<int,QRgb> p2) -> bool
                        {return p1.first < p2.first;};
    auto my_sort = std::bind(
            qSort<QVector<QPair<int,QRgb> >::iterator, typeof(op_less)>,
            std::placeholders::_1, std::placeholders::_2,
            op_less);
    QImage temp_image = ImageViewerService::getExpandedImage(
                *m_intermediate_image, radius);

    const int w = m_intermediate_image->width();
    const int h = m_intermediate_image->height();
    const int temp_w = w + 2*radius;
    m_showed_image.clear();
    m_showed_image.reset(new QImage(m_intermediate_image->size(),
                           m_intermediate_image->format()));

    QRgb* src_colors_line = reinterpret_cast<QRgb*>(
            temp_image.scanLine(radius));
    QRgb* dst_colors_line = reinterpret_cast<QRgb*>(
            m_showed_image->scanLine(0));

    #pragma omp parallel for\
            firstprivate(neigh_points)\
            schedule(dynamic, h/omp_get_num_threads())
    for (int i = 0; i < h; ++i)
    {
        for (int j = 0; j < w; ++j)
        {
            int alpha = qAlpha(src_colors_line[i*temp_w + radius + j]);
            if (alpha == 0)
            {
                continue;
            }

            int pos = 0;
            QRgb temp_col;
            for (int ii = -radius; ii <= radius; ++ii)
            {
                for (int jj = -radius; jj <= radius; ++jj)
                {
                    temp_col = src_colors_line[(i+ii)*temp_w + radius+(j+jj)];
                    neigh_points[pos++] = qMakePair(
                        qRed(temp_col) + qGreen(temp_col) + qBlue(temp_col),
                        temp_col);
                }
            }

            my_sort(neigh_points.begin(), neigh_points.end());
            temp_col = neigh_points[median_index].second;

            dst_colors_line[i*w+j] = qRgba(
                qRed(temp_col), qGreen(temp_col), qBlue(temp_col), alpha);
        }
    }

    m_edit_history->add(*m_showed_image);
    m_intermediate_image.clear();
    m_intermediate_image.reset(new QImage(m_showed_image->copy()));
    this->updateView();
    this->setSavedStatus(false);
    this->updateUndoRedoStatus();
    this->writeActionName(tr("Median filter") +
        QString::fromUtf8(" %1 px").
        arg(radius));
}


void MainWindow::onDilatationFilterApplying()
{
    if (m_intermediate_image.isNull())
    {
        return;
    }

    ImageViewerService::convertToColoured(*m_intermediate_image);

    int radius = ui->edit_morphology_spinBox->value();
    QImage temp_image = ImageViewerService::getExpandedImage(
                *m_intermediate_image, radius);

    const int w = m_intermediate_image->width();
    const int h = m_intermediate_image->height();
    const int temp_w = w + 2*radius;
    m_showed_image.clear();
    m_showed_image.reset(new QImage(m_intermediate_image->size(),
                           m_intermediate_image->format()));

    QRgb* src_colors_line = reinterpret_cast<QRgb*>(
            temp_image.scanLine(radius));
    QRgb* dst_colors_line = reinterpret_cast<QRgb*>(
            m_showed_image->scanLine(0));

    #pragma omp parallel for schedule(dynamic, h/omp_get_num_threads())
    for (int i = 0; i < h; ++i)
    {
        for (int j = 0; j < w; ++j)
        {
            int alpha = qAlpha(src_colors_line[i*temp_w + radius + j]);
            if (alpha == 0)
            {
                continue;
            }

            int color_sum, max_color_sum = 0;
            QRgb temp_col, max_col = 0;
            for (int ii = -radius; ii <= radius; ++ii)
            {
                for (int jj = -radius; jj <= radius; ++jj)
                {
                    temp_col = src_colors_line[(i+ii)*temp_w + radius+(j+jj)];
                    color_sum = qRed(temp_col) + qGreen(temp_col) + qBlue(temp_col);
                    if (color_sum > max_color_sum)
                    {
                        max_col = temp_col;
                        max_color_sum = color_sum;
                    }
                }
            }

            dst_colors_line[i*w+j] = qRgba(
                qRed(max_col), qGreen(max_col), qBlue(max_col), alpha);
        }
    }

    m_edit_history->add(*m_showed_image);
    m_intermediate_image.clear();
    m_intermediate_image.reset(new QImage(m_showed_image->copy()));
    this->updateView();
    this->setSavedStatus(false);
    this->updateUndoRedoStatus();
    this->writeActionName(tr("Dilatation filter") +
        QString::fromUtf8(" %1 px").
        arg(radius));
}

void MainWindow::onErosionFilterApplying()
{
    if (m_intermediate_image.isNull())
    {
        return;
    }

    ImageViewerService::convertToColoured(*m_intermediate_image);

    int radius = ui->edit_morphology_spinBox->value();
    QImage temp_image = ImageViewerService::getExpandedImage(
                *m_intermediate_image, radius);

    const int w = m_intermediate_image->width();
    const int h = m_intermediate_image->height();
    const int temp_w = w + 2*radius;
    m_showed_image.clear();
    m_showed_image.reset(new QImage(m_intermediate_image->size(),
                           m_intermediate_image->format()));

    QRgb* src_colors_line = reinterpret_cast<QRgb*>(
            temp_image.scanLine(radius));
    QRgb* dst_colors_line = reinterpret_cast<QRgb*>(
            m_showed_image->scanLine(0));

    #pragma omp parallel for schedule(dynamic, h/omp_get_num_threads())
    for (int i = 0; i < h; ++i)
    {
        for (int j = 0; j < w; ++j)
        {
            int alpha = qAlpha(src_colors_line[i*temp_w + radius + j]);
            if (alpha == 0)
            {
                continue;
            }

            int color_sum, min_color_sum = 1000;
            QRgb temp_col, min_col = 0;
            for (int ii = -radius; ii <= radius; ++ii)
            {
                for (int jj = -radius; jj <= radius; ++jj)
                {
                    temp_col = src_colors_line[(i+ii)*temp_w + radius+(j+jj)];
                    color_sum = qRed(temp_col) + qGreen(temp_col) + qBlue(temp_col);
                    if (color_sum < min_color_sum)
                    {
                        min_col = temp_col;
                        min_color_sum = color_sum;
                    }
                }
            }

            dst_colors_line[i*w+j] = qRgba(
                qRed(min_col), qGreen(min_col), qBlue(min_col), alpha);
        }
    }

    m_edit_history->add(*m_showed_image);
    m_intermediate_image.clear();
    m_intermediate_image.reset(new QImage(m_showed_image->copy()));
    this->updateView();
    this->setSavedStatus(false);
    this->updateUndoRedoStatus();
    this->writeActionName(tr("Erosion filter") +
        QString::fromUtf8(" %1 px").
        arg(radius));
}

void MainWindow::onCustomFilterApplied()
{
    this->onCustomFilterUpdated();
    this->applyMatrixFilter(
        m_filter_customizer->getMatrixData(),
        m_filter_customizer->getMatrixSize());
    this->writeActionName(tr("Custom filter"));
}

void MainWindow::onCustomFilterUpdated()
{
    ui->edit_custom_filter_apply_button->setEnabled(true);
}

void MainWindow::onClarityIncreasing()
{
    if (m_intermediate_image.isNull())
    {
        return;
    }

    QVector<double> mat_data = QVector<double>(9, -1.0);
    double kernel = 9 + 8 * (1 - ui->edit_clarity_slider->value()/100.0);
    double norm = kernel - 8;
    mat_data[4] = kernel;
    if (norm > 1.0)
    {
        for (int i = 0; i < mat_data.size(); ++i)
        {
            mat_data[i] /= norm;
        }
    }

    this->applyMatrixFilter(mat_data, 3);
    this->writeActionName(tr("Clarity increase") +
        QString::fromUtf8(" %1 %").
        arg(ui->edit_clarity_slider->value()));
}

void MainWindow::applyMatrixFilter(
        const QVector<double>& mat_data, int mat_size)
{
    ImageViewerService::convertToColoured(*m_intermediate_image);

    int radius = mat_size / 2;
    QImage temp_image = ImageViewerService::getExpandedImage(
                *m_intermediate_image, radius);

    const int w = m_intermediate_image->width();
    const int h = m_intermediate_image->height();
    const int temp_w = w + 2*radius;
    m_showed_image.clear();
    m_showed_image.reset(new QImage(m_intermediate_image->size(),
                           m_intermediate_image->format()));

    QRgb* src_colors_line = reinterpret_cast<QRgb*>(
            temp_image.scanLine(radius));
    QRgb* dst_colors_line = reinterpret_cast<QRgb*>(
            m_showed_image->scanLine(0));

//    double t = omp_get_wtime();

    #pragma omp parallel for schedule(dynamic, h/omp_get_num_threads())
    for (int i = 0; i < h; ++i)
    {
        for (int j = 0; j < w; ++j)
        {
            int alpha = qAlpha(src_colors_line[i*temp_w + radius + j]);
            if (alpha == 0)
            {
                continue;
            }

            ImageViewerService::RgbColor dst_col{0.0, 0.0, 0.0};
            for (int ii = -radius; ii <= radius; ++ii)
            {
                for (int jj = -radius; jj <= radius; ++jj)
                {
                    double coef = mat_data[(ii+radius)*mat_size + (jj+radius)];
                    QRgb src_col = src_colors_line[(i+ii)*temp_w + radius+(j+jj)];
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

//    qDebug() << "WORK:" << omp_get_wtime() - t;

    m_edit_history->add(*m_showed_image);
    m_intermediate_image.clear();
    m_intermediate_image.reset(new QImage(m_showed_image->copy()));
    this->updateView();
    this->setSavedStatus(false);
    this->updateUndoRedoStatus();
}
