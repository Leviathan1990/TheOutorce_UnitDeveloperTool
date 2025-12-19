#ifndef INITPARAMSWIDGET_H
#define INITPARAMSWIDGET_H

#include <QWidget>
#include <QTableWidget>
#include <QPushButton>
#include <QLabel>
#include "AIStructs.h"

namespace AI {

class InitParamsWidget : public QWidget {
    Q_OBJECT
public:
    explicit InitParamsWidget(QWidget* parent = nullptr);

    void setInitParameters(InitParameters* params);
    void clear();

signals:
    void paramsModified();

private slots:
    void onCellChanged(int row, int column);

private:
    void setupUI();
    void refreshTable();

    InitParameters* m_params;
    QTableWidget* m_table;
    QLabel* m_infoLabel;
};

} // namespace AI

#endif // INITPARAMSWIDGET_H
