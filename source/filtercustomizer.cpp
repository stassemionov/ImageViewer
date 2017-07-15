#include "filtercustomizer.h"
#include "ui_filtercustomizer.h"

#include <QVariant>

#include "service.h"

#include <QDebug>

FilterCustomizer::FilterCustomizer(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FilterCustomizer)
{
    ui->setupUi(this);
    ui->message_label->hide();
    ui->size_spinBox->setValue(3);
    ui->tableWidget->setRowCount(3);
    ui->tableWidget->setColumnCount(3);
    m_data.resize(9);
    m_data.fill(0);
    m_data[4] = 1;
    for (int i = 0; i < 3; ++i)
    {
        for (int j = 0; j < 3; ++j)
        {
            int value = (i * j == 1) ? 1 : 0;
            QTableWidgetItem* item = new QTableWidgetItem(
                QString::number(value));
            item->setTextAlignment(Qt::AlignCenter);
            ui->tableWidget->setItem(i, j, item);
        }
    }

    connect(ui->cancel_button, SIGNAL(clicked(bool)),
            this, SLOT(onCancel()));
    connect(ui->close_button, SIGNAL(clicked(bool)),
            this, SLOT(onClose()));
    connect(ui->apply_button, SIGNAL(clicked(bool)),
            this, SLOT(onApply()));
    connect(ui->automatically_radioButton, SIGNAL(toggled(bool)),
            this, SLOT(onChangeNormalizationMode()));
    connect(ui->size_spinBox, SIGNAL(valueChanged(int)),
            this, SLOT(onSizeChanged(int)));
}

FilterCustomizer::~FilterCustomizer()
{
    delete ui;
}

void FilterCustomizer::setMatrixSize(int size)
{
    if (size < 0)
    {
        return;
    }
    ui->tableWidget->setRowCount(size);
    ui->tableWidget->setColumnCount(size);
}

int FilterCustomizer::getMatrixSize()
{
    return ui->size_spinBox->value();
}

const QVector<double> &FilterCustomizer::getMatrixData()
{
    return m_data;
}

bool FilterCustomizer::updateData()
{
    int size = this->getMatrixSize();
    for (int i = 0; i < size; ++i)
    {
        for (int j = 0; j < size; ++j)
        {
            bool is_ok = false;
            QVariant data_var =
                ui->tableWidget->item(i, j)->data(Qt::EditRole);
            double data = data_var.toDouble(&is_ok);
            if (!is_ok)
            {
                this->showErrorMessage(i, j);
                return false;
            }
            m_data[i*size+j] = data;
        }
    }

    if (ui->automatically_radioButton->isChecked())
    {
        double norm = 0.0;
        QString norm_name = ui->norms_names_comboBox->currentText();
        if (norm_name == QString::fromUtf8("Summa"))
        {
            foreach (double el, m_data)
            {
                norm += el;
            }
            norm = qAbs(norm);
        }
        else if (norm_name == QString::fromUtf8("Summa of absolute values"))
        {
            foreach (double el, m_data)
            {
                norm += qAbs(el);
            }
        }
        else if (norm_name == QString::fromUtf8("Euclid norm"))
        {
            foreach (double el, m_data)
            {
                norm += el * el;
            }
            norm = sqrt(norm);
        }
        else if (norm_name == QString::fromUtf8("Maximum absolute value"))
        {
            foreach (double el, m_data)
            {
                norm = qMax(norm, qAbs(el));
            }
        }

        if (norm == 0.0)
        {
            this->showErrorMessage(-1, -1);
            return false;
        }
        for (int i = 0; i < m_data.size(); ++i)
        {
            m_data[i] /= norm;
        }
    }
    else
    {
        double norm_val = 1.0 / ui->factor_doubleSpinBox->value();
        if (norm_val != 1.0)
        {
            for (int i = 0; i < m_data.size(); ++i)
            {
                m_data[i] *= norm_val;
            };
        }
    }
    return true;
}

void FilterCustomizer::onApply()
{
    if (this->updateData())
    {
        m_state_data.clear();
        emit matrixApplied();
        this->close();
    }
}

void FilterCustomizer::onClose()
{
    if (this->updateData())
    {
        m_state_data.clear();
        emit matrixUpdated();
        this->close();
    }
}

void FilterCustomizer::onCancel()
{
    this->restoreState();
    this->close();
}

void FilterCustomizer::showErrorMessage(int x, int y)
{
    if ((x < 0) || (y < 0))
    {
        ui->message_label->setText(
            tr("Chosen norm of matrix equals 0. Normalizing is impossible."));
    }
    else
    {
        ui->message_label->setText(tr("Incorrect data in cell") +
            QString::fromUtf8(" [%1, %2]").arg(x+1).arg(y+1));
    }
    ui->message_label->show();
}


void FilterCustomizer::onChangeNormalizationMode()
{
    bool is_auto_mode =
        ui->automatically_radioButton->isChecked();
    ui->norms_names_comboBox->setEnabled(is_auto_mode);
    ui->factor_doubleSpinBox->setDisabled(is_auto_mode);
}

void FilterCustomizer::showEvent(QShowEvent* e)
{
    ui->message_label->hide();
    this->saveState();
    e->accept();
}

void FilterCustomizer::onSizeChanged(int size)
{
    ui->tableWidget->setRowCount(size);
    ui->tableWidget->setColumnCount(size);
    m_data.resize(size*size);
    int center = ceil(size / 2);
    for (int i = 0; i < size; ++i)
    {
        for (int j = 0; j < size; ++j)
        {
            int value = (i == j && i == center) ? 1 : 0;
            QTableWidgetItem* item = new QTableWidgetItem(
                QString::number(value));
            item->setTextAlignment(Qt::AlignCenter);
            ui->tableWidget->setItem(i, j, item);
        }
    };
}

void FilterCustomizer::saveState()
{
    m_state_data[QString::fromUtf8("size")] =
            QVariant(ui->size_spinBox->value());
    m_state_data[QString::fromUtf8("matrix")] =
            QVariant(ImageViewerService::toListOfVariant(m_data));
    m_state_data[QString::fromUtf8("normalization")] =
            QVariant(ui->automatically_radioButton->isChecked());
    m_state_data[QString::fromUtf8("norma")] =
            QVariant(ui->norms_names_comboBox->currentIndex());
    m_state_data[QString::fromUtf8("factor")] =
            QVariant(ui->factor_doubleSpinBox->value());
}

void FilterCustomizer::restoreState()
{
    int size = m_state_data[QString::fromUtf8("size")].toInt();
    ui->size_spinBox->setValue(size);
    ui->tableWidget->setRowCount(size);
    ui->tableWidget->setColumnCount(size);
    QList<QVariant> l = m_state_data[QString::fromUtf8("matrix")].toList();
    for (int i = 0; i < size; ++i)
    {
        for (int j = 0; j < size; ++j)
        {
            double val = l[i*size + j].toDouble();
            QTableWidgetItem* item = new QTableWidgetItem(
                QString::number(val));
            m_data[i*size+j] = val;
            item->setTextAlignment(Qt::AlignCenter);
            ui->tableWidget->setItem(i, j, item);
        }
    }
    ui->automatically_radioButton->setChecked(
        m_state_data[QString::fromUtf8("normalization")].toBool());
    ui->norms_names_comboBox->setCurrentIndex(
        m_state_data[QString::fromUtf8("norma")].toInt());
    ui->factor_doubleSpinBox->setValue(
        m_state_data[QString::fromUtf8("factor")].toDouble());
}
