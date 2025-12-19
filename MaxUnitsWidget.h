#ifndef MAXUNITSWIDGET_H
#define MAXUNITSWIDGET_H

#include <QWidget>
#include <QTableWidget>
#include <QPushButton>
#include <QComboBox>
#include <QSpinBox>
#include <QLabel>
#include "AIStructs.h"

namespace AI {

class MaxUnitsWidget : public QWidget {
    Q_OBJECT
public:
    explicit MaxUnitsWidget(QWidget* parent = nullptr);

    void setMaxUnits(MaxUnits* maxUnits);
    void setAvailableUnits(const QStringList& units);
    void clear();

signals:
    void maxUnitsModified();

private slots:
    void onAddEntry();
    void onRemoveEntry();
    void onCellChanged(int row, int column);
    void onSelectionChanged();

private:
    void setupUI();
    void refreshTable();
    void updateButtonStates();

    MaxUnits* m_maxUnits;
    QStringList m_availableUnits;

    QTableWidget* m_table;
    QComboBox* m_unitCombo;
    QSpinBox* m_maxSpinBox;
    QPushButton* m_addButton;
    QPushButton* m_removeButton;
    QLabel* m_infoLabel;
};

} // namespace AI

#endif // MAXUNITSWIDGET_H
