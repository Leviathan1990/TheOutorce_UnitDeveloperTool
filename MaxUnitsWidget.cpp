#include "MaxUnitsWidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QHeaderView>
#include <QMessageBox>

namespace AI {

MaxUnitsWidget::MaxUnitsWidget(QWidget* parent)
    : QWidget(parent), m_maxUnits(nullptr)
{
    setupUI();
}

void MaxUnitsWidget::setupUI() {
    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    m_infoLabel = new QLabel("Maximum number of units AI can build. Default is 10000 (unlimited).", this);
    mainLayout->addWidget(m_infoLabel);

    m_table = new QTableWidget(this);
    m_table->setColumnCount(2);
    m_table->setHorizontalHeaderLabels({"Unit Name", "Max Count"});
    m_table->horizontalHeader()->setStretchLastSection(false);
    m_table->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    m_table->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Fixed);
    m_table->setColumnWidth(1, 100);
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->setSelectionMode(QAbstractItemView::SingleSelection);
    m_table->setAlternatingRowColors(true);

    connect(m_table, &QTableWidget::cellChanged, this, &MaxUnitsWidget::onCellChanged);
    connect(m_table, &QTableWidget::itemSelectionChanged, this, &MaxUnitsWidget::onSelectionChanged);

    mainLayout->addWidget(m_table, 1);

    // Add entry panel
    QGroupBox* addGroup = new QGroupBox("Add Unit", this);
    QHBoxLayout* addLayout = new QHBoxLayout(addGroup);

    addLayout->addWidget(new QLabel("Unit:", this));
    m_unitCombo = new QComboBox(this);
    m_unitCombo->setEditable(true);
    m_unitCombo->setMinimumWidth(200);
    addLayout->addWidget(m_unitCombo, 1);

    addLayout->addWidget(new QLabel("Max:", this));
    m_maxSpinBox = new QSpinBox(this);
    m_maxSpinBox->setRange(0, 10000);
    m_maxSpinBox->setValue(10);
    addLayout->addWidget(m_maxSpinBox);

    m_addButton = new QPushButton("Add", this);
    connect(m_addButton, &QPushButton::clicked, this, &MaxUnitsWidget::onAddEntry);
    addLayout->addWidget(m_addButton);

    mainLayout->addWidget(addGroup);

    // Button bar
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();

    m_removeButton = new QPushButton("Remove Selected", this);
    m_removeButton->setEnabled(false);
    connect(m_removeButton, &QPushButton::clicked, this, &MaxUnitsWidget::onRemoveEntry);
    buttonLayout->addWidget(m_removeButton);

    mainLayout->addLayout(buttonLayout);

    updateButtonStates();
}

void MaxUnitsWidget::setMaxUnits(MaxUnits* maxUnits) {
    m_maxUnits = maxUnits;
    refreshTable();
    updateButtonStates();
}

void MaxUnitsWidget::setAvailableUnits(const QStringList& units) {
    m_availableUnits = units;
    m_unitCombo->clear();
    m_unitCombo->addItems(units);
}

void MaxUnitsWidget::clear() {
    m_maxUnits = nullptr;
    m_table->setRowCount(0);
    m_infoLabel->setText("No file loaded");
    updateButtonStates();
}

void MaxUnitsWidget::refreshTable() {
    m_table->blockSignals(true);
    m_table->setRowCount(0);

    if (!m_maxUnits) {
        m_table->blockSignals(false);
        return;
    }

    m_table->setRowCount(m_maxUnits->entries.size());

    for (int i = 0; i < m_maxUnits->entries.size(); ++i) {
        const MaxUnitsEntry& entry = m_maxUnits->entries[i];

        QTableWidgetItem* nameItem = new QTableWidgetItem(entry.unitName);
        QTableWidgetItem* maxItem = new QTableWidgetItem(QString::number(entry.maxCount));
        maxItem->setTextAlignment(Qt::AlignCenter);

        // Color code by max count
        QColor bgColor;
        if (entry.maxCount == 0) {
            bgColor = QColor(200, 150, 150);      // Red - disabled
        } else if (entry.maxCount <= 2) {
            bgColor = QColor(200, 180, 150);      // Orange - very limited
        } else if (entry.maxCount <= 10) {
            bgColor = QColor(200, 200, 150);      // Yellow - limited
        } else if (entry.maxCount <= 50) {
            bgColor = QColor(180, 200, 180);      // Light green - moderate
        } else {
            bgColor = QColor(150, 200, 150);      // Green - high/unlimited
        }

        nameItem->setBackground(bgColor);
        maxItem->setBackground(bgColor);

        m_table->setItem(i, 0, nameItem);
        m_table->setItem(i, 1, maxItem);
    }

    m_table->blockSignals(false);

    m_infoLabel->setText(QString("MaxNumberOfUnits: %1 units defined").arg(m_maxUnits->entries.size()));
}

void MaxUnitsWidget::updateButtonStates() {
    bool hasData = (m_maxUnits != nullptr);
    bool hasSelection = !m_table->selectedItems().isEmpty();

    m_addButton->setEnabled(hasData);
    m_removeButton->setEnabled(hasData && hasSelection);
    m_unitCombo->setEnabled(hasData);
    m_maxSpinBox->setEnabled(hasData);
}

void MaxUnitsWidget::onAddEntry() {
    if (!m_maxUnits) return;

    QString unitName = m_unitCombo->currentText().trimmed();
    if (unitName.isEmpty()) {
        QMessageBox::warning(this, "Warning", "Please enter a unit name.");
        return;
    }

    int existingIdx = m_maxUnits->findEntry(unitName);
    if (existingIdx >= 0) {
        m_maxUnits->entries[existingIdx].maxCount = m_maxSpinBox->value();
    } else {
        m_maxUnits->entries.append(MaxUnitsEntry(unitName, m_maxSpinBox->value()));
    }

    refreshTable();
    m_unitCombo->setCurrentText("");
    emit maxUnitsModified();
}

void MaxUnitsWidget::onRemoveEntry() {
    if (!m_maxUnits) return;

    int row = m_table->currentRow();
    if (row < 0 || row >= m_maxUnits->entries.size()) return;

    m_maxUnits->entries.removeAt(row);
    refreshTable();
    emit maxUnitsModified();
}

void MaxUnitsWidget::onCellChanged(int row, int column) {
    if (!m_maxUnits || row < 0 || row >= m_maxUnits->entries.size()) return;

    QTableWidgetItem* item = m_table->item(row, column);
    if (!item) return;

    if (column == 0) {
        m_maxUnits->entries[row].unitName = item->text();
    } else if (column == 1) {
        bool ok;
        int value = item->text().toInt(&ok);
        if (ok) {
            m_maxUnits->entries[row].maxCount = value;
        }
    }

    refreshTable();
    emit maxUnitsModified();
}

void MaxUnitsWidget::onSelectionChanged() {
    updateButtonStates();
}

} // namespace AI
