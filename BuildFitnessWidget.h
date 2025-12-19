#ifndef BUILDFITNESSWIDGET_H
#define BUILDFITNESSWIDGET_H

#include <QWidget>
#include <QTableWidget>
#include <QPushButton>
#include <QLineEdit>
#include <QComboBox>
#include <QSpinBox>
#include <QLabel>
#include "AIStructs.h"

namespace AI {

class BuildFitnessWidget : public QWidget {
    Q_OBJECT
public:
    explicit BuildFitnessWidget(QWidget* parent = nullptr);

    void setBuildFitness(BuildFitness* fitness);
    void setAvailableUnits(const QStringList& units);
    void clear();

signals:
    void fitnessModified();

private slots:
    void onAddEntry();
    void onRemoveEntry();
    void onCellChanged(int row, int column);
    void onSelectionChanged();

private:
    void setupUI();
    void refreshTable();
    void updateButtonStates();

    BuildFitness* m_fitness;
    QStringList m_availableUnits;

    QTableWidget* m_table;
    QComboBox* m_unitCombo;
    QSpinBox* m_fitnessSpinBox;
    QPushButton* m_addButton;
    QPushButton* m_removeButton;
    QLabel* m_infoLabel;
};

} // namespace AI

#endif // BUILDFITNESSWIDGET_H
