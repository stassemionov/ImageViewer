#ifndef FILTERCUSTOMIZER_H
#define FILTERCUSTOMIZER_H

#include <QShowEvent>
#include <QWidget>

namespace Ui {
class FilterCustomizer;
}

class FilterCustomizer : public QWidget
{
    Q_OBJECT

public:
    explicit FilterCustomizer(QWidget *parent = 0);
    ~FilterCustomizer();

    void setMatrixSize(int size);
    int getMatrixSize();
    const QVector<double>& getMatrixData();

signals:
    void matrixApplied();
    void matrixUpdated();

protected:
    void showEvent(QShowEvent* e);

protected slots:
    void onApply();
    void onClose();
    void onCancel();
    void onSizeChanged(int size);
    void onChangeNormalizationMode();

protected:
    void showErrorMessage(int x, int y);
    bool updateData();
    void saveState();
    void restoreState();

private:
    Ui::FilterCustomizer *ui;
    QVector<double> m_data;
    QMap<QString, QVariant> m_state_data;
};

#endif // FILTERCUSTOMIZER_H
