#include "InitParamsWidget.h"
#include <QVBoxLayout>
#include <QHeaderView>

namespace AI {

InitParamsWidget::InitParamsWidget(QWidget* parent)
    : QWidget(parent), m_params(nullptr)
{
    setupUI();
}

void InitParamsWidget::setupUI() {
    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    m_infoLabel = new QLabel("AI behavior parameters. Format: Name, Value, Variance (±)", this);
    mainLayout->addWidget(m_infoLabel);

    m_table = new QTableWidget(this);
    m_table->setColumnCount(3);
    m_table->setHorizontalHeaderLabels({"Parameter", "Value", "Variance (±)"});
    m_table->horizontalHeader()->setStretchLastSection(false);
    m_table->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    m_table->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Fixed);
    m_table->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Fixed);
    m_table->setColumnWidth(1, 100);
    m_table->setColumnWidth(2, 100);
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->setAlternatingRowColors(true);

    connect(m_table, &QTableWidget::cellChanged, this, &InitParamsWidget::onCellChanged);

    mainLayout->addWidget(m_table, 1);
}

void InitParamsWidget::setInitParameters(InitParameters* params) {
    m_params = params;
    refreshTable();
}

void InitParamsWidget::clear() {
    m_params = nullptr;
    m_table->setRowCount(0);
    m_infoLabel->setText("No file loaded");
}

void InitParamsWidget::refreshTable() {
    m_table->blockSignals(true);
    m_table->setRowCount(0);

    if (!m_params) {
        m_table->blockSignals(false);
        return;
    }

    // Use known parameter order
    QStringList knownNames = getKnownInitParamNames();
    QStringList allNames;

    // Add known params first
    for (const QString& name : knownNames) {
        if (m_params->params.contains(name)) {
            allNames.append(name);
        }
    }

    // Add any custom params
    for (auto it = m_params->params.begin(); it != m_params->params.end(); ++it) {
        if (!allNames.contains(it.key())) {
            allNames.append(it.key());
        }
    }

    m_table->setRowCount(allNames.size());

    for (int i = 0; i < allNames.size(); ++i) {
        const QString& name = allNames[i];
        const InitParameter& p = m_params->params[name];

        QTableWidgetItem* nameItem = new QTableWidgetItem(p.name);
        nameItem->setFlags(nameItem->flags() & ~Qt::ItemIsEditable);  // Name not editable

        QString valueStr = p.isFloat ? QString::number(p.value, 'f', 2) : QString::number(static_cast<int>(p.value));
        QString varStr = p.isFloat ? QString::number(p.variance, 'f', 2) : QString::number(static_cast<int>(p.variance));

        QTableWidgetItem* valueItem = new QTableWidgetItem(valueStr);
        QTableWidgetItem* varItem = new QTableWidgetItem(varStr);

        valueItem->setTextAlignment(Qt::AlignCenter);
        varItem->setTextAlignment(Qt::AlignCenter);

        // Color code special parameters
        if (name.toLower().contains("cheat")) {
            nameItem->setBackground(QColor(255, 200, 200));  // Red-ish for cheating
            valueItem->setBackground(QColor(255, 200, 200));
            varItem->setBackground(QColor(255, 200, 200));
        } else if (name.toLower().contains("attack")) {
            nameItem->setBackground(QColor(200, 200, 255));  // Blue-ish for attack
            valueItem->setBackground(QColor(200, 200, 255));
            varItem->setBackground(QColor(200, 200, 255));
        } else if (name.toLower().contains("defend")) {
            nameItem->setBackground(QColor(200, 255, 200));  // Green-ish for defense
            valueItem->setBackground(QColor(200, 255, 200));
            varItem->setBackground(QColor(200, 255, 200));
        }

        m_table->setItem(i, 0, nameItem);
        m_table->setItem(i, 1, valueItem);
        m_table->setItem(i, 2, varItem);
    }

    m_table->blockSignals(false);

    m_infoLabel->setText(QString("InitParameters: %1 parameters loaded").arg(m_params->params.size()));
}

void InitParamsWidget::onCellChanged(int row, int column) {
    if (!m_params || column == 0) return;

    QTableWidgetItem* nameItem = m_table->item(row, 0);
    QTableWidgetItem* item = m_table->item(row, column);
    if (!nameItem || !item) return;

    QString name = nameItem->text();
    if (!m_params->params.contains(name)) return;

    InitParameter& p = m_params->params[name];

    bool ok;
    float newValue = item->text().toFloat(&ok);
    if (!ok) return;

    if (column == 1) {
        p.value = newValue;
    } else if (column == 2) {
        p.variance = newValue;
    }

    emit paramsModified();
}

} // namespace AI
