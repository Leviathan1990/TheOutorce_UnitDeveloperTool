#include "BuildFitnessWidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QHeaderView>
#include <QMessageBox>

namespace AI {

BuildFitnessWidget::BuildFitnessWidget(QWidget* parent)
    : QWidget(parent), m_fitness(nullptr)
{
    setupUI();
}

void BuildFitnessWidget::setupUI() {
    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    m_infoLabel = new QLabel("Build priority for units. Higher values = AI builds more often.", this);
    mainLayout->addWidget(m_infoLabel);

    m_table = new QTableWidget(this);
    m_table->setColumnCount(2);
    m_table->setHorizontalHeaderLabels({"Unit Name", "Fitness"});
    m_table->horizontalHeader()->setStretchLastSection(false);
    m_table->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    m_table->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Fixed);
    m_table->setColumnWidth(1, 80);
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->setSelectionMode(QAbstractItemView::SingleSelection);
    m_table->setAlternatingRowColors(true);

    connect(m_table, &QTableWidget::cellChanged, this, &BuildFitnessWidget::onCellChanged);
    connect(m_table, &QTableWidget::itemSelectionChanged, this, &BuildFitnessWidget::onSelectionChanged);

    mainLayout->addWidget(m_table, 1);

    // Add entry panel
    QGroupBox* addGroup = new QGroupBox("Add Unit", this);
    QHBoxLayout* addLayout = new QHBoxLayout(addGroup);

    addLayout->addWidget(new QLabel("Unit:", this));
    m_unitCombo = new QComboBox(this);
    m_unitCombo->setEditable(true);
    m_unitCombo->setMinimumWidth(200);
    addLayout->addWidget(m_unitCombo, 1);

    addLayout->addWidget(new QLabel("Fitness:", this));
    m_fitnessSpinBox = new QSpinBox(this);
    m_fitnessSpinBox->setRange(0, 1000);
    m_fitnessSpinBox->setValue(50);
    addLayout->addWidget(m_fitnessSpinBox);

    m_addButton = new QPushButton("Add", this);
    connect(m_addButton, &QPushButton::clicked, this, &BuildFitnessWidget::onAddEntry);
    addLayout->addWidget(m_addButton);

    mainLayout->addWidget(addGroup);

    // Button bar
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();

    m_removeButton = new QPushButton("Remove Selected", this);
    m_removeButton->setEnabled(false);
    connect(m_removeButton, &QPushButton::clicked, this, &BuildFitnessWidget::onRemoveEntry);
    buttonLayout->addWidget(m_removeButton);

    mainLayout->addLayout(buttonLayout);

    updateButtonStates();
}

void BuildFitnessWidget::setBuildFitness(BuildFitness* fitness) {
    m_fitness = fitness;
    refreshTable();
    updateButtonStates();
}

void BuildFitnessWidget::setAvailableUnits(const QStringList& units) {
    m_availableUnits = units;
    m_unitCombo->clear();
    m_unitCombo->addItems(units);
}

void BuildFitnessWidget::clear() {
    m_fitness = nullptr;
    m_table->setRowCount(0);
    m_infoLabel->setText("No file loaded");
    updateButtonStates();
}

void BuildFitnessWidget::refreshTable() {
    m_table->blockSignals(true);
    m_table->setRowCount(0);

    if (!m_fitness) {
        m_table->blockSignals(false);
        return;
    }

    m_table->setRowCount(m_fitness->entries.size());

    for (int i = 0; i < m_fitness->entries.size(); ++i) {
        const BuildFitnessEntry& entry = m_fitness->entries[i];

        QTableWidgetItem* nameItem = new QTableWidgetItem(entry.unitName);
        QTableWidgetItem* fitnessItem = new QTableWidgetItem(QString::number(entry.fitness));
        fitnessItem->setTextAlignment(Qt::AlignCenter);

        // Color code by fitness level
        QColor bgColor;
        if (entry.fitness >= 300) {
            bgColor = QColor(100, 200, 100);      // Green - very high
        } else if (entry.fitness >= 100) {
            bgColor = QColor(150, 200, 150);      // Light green - high
        } else if (entry.fitness >= 50) {
            bgColor = QColor(200, 200, 150);      // Yellow - medium
        } else if (entry.fitness >= 10) {
            bgColor = QColor(200, 180, 150);      // Orange - low
        } else {
            bgColor = QColor(200, 150, 150);      // Red - very low
        }

        nameItem->setBackground(bgColor);
        fitnessItem->setBackground(bgColor);

        m_table->setItem(i, 0, nameItem);
        m_table->setItem(i, 1, fitnessItem);
    }

    m_table->blockSignals(false);

    m_infoLabel->setText(QString("BuildFitness: %1 units defined").arg(m_fitness->entries.size()));
}

void BuildFitnessWidget::updateButtonStates() {
    bool hasFitness = (m_fitness != nullptr);
    bool hasSelection = !m_table->selectedItems().isEmpty();

    m_addButton->setEnabled(hasFitness);
    m_removeButton->setEnabled(hasFitness && hasSelection);
    m_unitCombo->setEnabled(hasFitness);
    m_fitnessSpinBox->setEnabled(hasFitness);
}

void BuildFitnessWidget::onAddEntry() {
    if (!m_fitness) return;

    QString unitName = m_unitCombo->currentText().trimmed();
    if (unitName.isEmpty()) {
        QMessageBox::warning(this, "Warning", "Please enter a unit name.");
        return;
    }

    int existingIdx = m_fitness->findEntry(unitName);
    if (existingIdx >= 0) {
        m_fitness->entries[existingIdx].fitness = m_fitnessSpinBox->value();
    } else {
        m_fitness->entries.append(BuildFitnessEntry(unitName, m_fitnessSpinBox->value()));
    }

    refreshTable();
    m_unitCombo->setCurrentText("");
    emit fitnessModified();
}

void BuildFitnessWidget::onRemoveEntry() {
    if (!m_fitness) return;

    int row = m_table->currentRow();
    if (row < 0 || row >= m_fitness->entries.size()) return;

    m_fitness->entries.removeAt(row);
    refreshTable();
    emit fitnessModified();
}

void BuildFitnessWidget::onCellChanged(int row, int column) {
    if (!m_fitness || row < 0 || row >= m_fitness->entries.size()) return;

    QTableWidgetItem* item = m_table->item(row, column);
    if (!item) return;

    if (column == 0) {
        m_fitness->entries[row].unitName = item->text();
    } else if (column == 1) {
        bool ok;
        int value = item->text().toInt(&ok);
        if (ok) {
            m_fitness->entries[row].fitness = value;
        }
    }

    refreshTable();
    emit fitnessModified();
}

void BuildFitnessWidget::onSelectionChanged() {
    updateButtonStates();
}

} // namespace AI
