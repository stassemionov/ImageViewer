#include <mainwindow.h>
#include "ui_mainwindow.h"

#include <QDebug>

void MainWindow::onColourEdited()
{
    m_edit_history->add(*m_intermediate_image);

    int v_val = ui->edit_brightness_slider->value();
    int s_val = ui->edit_saturation_slider->value();
    int r_val = ui->edit_red_slider->value()   + v_val;
    int g_val = ui->edit_green_slider->value() + v_val;
    int b_val = ui->edit_blue_slider->value()  + v_val;

    for (int i = 0; i < m_intermediate_image->height(); ++i)
    {
        QRgb* src_colors_line = reinterpret_cast<QRgb*>(
                    m_colour_buffer_image->scanLine(i));
        QRgb* dst_colors_line = reinterpret_cast<QRgb*>(
                    m_intermediate_image->scanLine(i));
        for (int j = 0; j < m_intermediate_image->width(); ++j)
        {
            QRgb col = src_colors_line[j];
            int r = qRed(col) + r_val;
            int g = qGreen(col) + g_val;
            int b = qBlue(col) + b_val;
            dst_colors_line[j] = qRgba(
                        (r > 255) ? 255 : ((r < 0) ? 0 : r),
                        (g > 255) ? 255 : ((g < 0) ? 0 : g),
                        (b > 255) ? 255 : ((b < 0) ? 0 : b),
                        qAlpha(col));
        }
    }

    if (s_val != 0)
    {
        for (int j = 0; j < m_intermediate_image->width(); ++j)
        {
            for (int i = 0; i < m_intermediate_image->height(); ++i)
            {
                QColor pix_col = m_intermediate_image->pixelColor(j, i);
                int h,s,v;
                pix_col.getHsv(&h, &s, &v);
                s += s_val;
                if (s < 0)
                {
                    s = 0;
                }
                else if (s > 255)
                {
                    s = 255;
                }
                pix_col.setHsv(h, s, v);
                m_intermediate_image->setPixelColor(j, i, pix_col);
            }
        }
    }

    // Uncolourize
    if (m_uncolourized)
    {
        for (int j = 0; j < m_intermediate_image->width(); ++j)
        {
            for (int i = 0; i < m_intermediate_image->height(); ++i)
            {
                QColor pix_col = m_intermediate_image->pixelColor(j, i);
                pix_col.setHsv(pix_col.hue(), 0, pix_col.value());
                m_intermediate_image->setPixelColor(j, i, pix_col);
            }
        }
    }

    // Negative
    if (m_negatived)
    {
        m_intermediate_image->invertPixels();
    }

    m_showed_image = *m_intermediate_image;
    m_screne_label.setPixmap(QPixmap::fromImage(m_showed_image));
}

void MainWindow::onUncolourized()
{
    m_uncolourized = !m_uncolourized;
    this->onColourEdited();
}

void MainWindow::onNegatived()
{
    m_negatived = !m_negatived;
    this->onColourEdited();
}
