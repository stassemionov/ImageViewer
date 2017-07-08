#include "filtercustomizer.h"
#include "ui_filtercustomizer.h"

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
    for (int i = 0; i < 3; ++i)
    {
        for (int j = 0; j < 3; ++j)
        {
            int value = (i * j == 1) ? 1 : 0;
            QTableWidgetItem* item = new QTableWidgetItem(
                QString::fromUtf8("%1").arg(value));
            item->setTextAlignment(Qt::AlignCenter);
            ui->tableWidget->setItem(i, j, item);
        }
    }

    connect(ui->cancel_button, SIGNAL(clicked(bool)),
            this, SLOT(close()));
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

void FilterCustomizer::onApply()
{
    int size = this->getMatrixSize();
    double norm = 0.0;
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
                return;
            }
            m_data[i*size+j] = data;
            norm += data;
        }
    }

    if (ui->automatically_radioButton->isChecked())
    {
        norm = qAbs(norm);
        if (norm == 0.0)
        {
            this->showErrorMessage(-1, -1);
            return;
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
    emit matrixUpdated();
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
    ui->factor_doubleSpinBox->setDisabled(
        ui->automatically_radioButton->isChecked());
}

void FilterCustomizer::showEvent(QShowEvent* e)
{
    ui->message_label->hide();
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
                QString::fromUtf8("%1").arg(value));
            item->setTextAlignment(Qt::AlignCenter);
            ui->tableWidget->setItem(i, j, item);
        }
    };
}