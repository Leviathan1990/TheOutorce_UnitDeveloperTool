#include "AIValuesWidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QScrollArea>

namespace AI {

// ============================================================================
// AIValueEditor - Single value editor
// ============================================================================

AIValueEditor::AIValueEditor(const AIValueDef& def, QWidget* parent)
    : QWidget(parent), m_def(def), m_valueSpinBox(nullptr),
    m_varianceSpinBox(nullptr), m_boolCheckBox(nullptr)
{
    QHBoxLayout* layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 2, 0, 2);

    // Name label
    QLabel* nameLabel = new QLabel(def.name, this);
    nameLabel->setMinimumWidth(160);
    nameLabel->setToolTip(def.description);
    layout->addWidget(nameLabel);

    if (def.isBool) {
        // Boolean: just a checkbox
        m_boolCheckBox = new QCheckBox(this);
        m_boolCheckBox->setToolTip(def.description);
        connect(m_boolCheckBox, &QCheckBox::toggled, this, &AIValueEditor::valueChanged);
        layout->addWidget(m_boolCheckBox);
        layout->addStretch();
    } else {
        // Value spinbox
        m_valueSpinBox = new QDoubleSpinBox(this);
        m_valueSpinBox->setToolTip(def.description);

        if (def.isFloat) {
            m_valueSpinBox->setDecimals(1);
            m_valueSpinBox->setRange(-10000.0, 100000.0);
            m_valueSpinBox->setSingleStep(1.0);
        } else {
            m_valueSpinBox->setDecimals(0);
            m_valueSpinBox->setRange(-100000, 1000000);
            m_valueSpinBox->setSingleStep(1);
        }
        m_valueSpinBox->setMinimumWidth(80);
        connect(m_valueSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
                this, &AIValueEditor::valueChanged);
        layout->addWidget(m_valueSpinBox);

        // Variance label and spinbox
        QLabel* varLabel = new QLabel("±", this);
        layout->addWidget(varLabel);

        m_varianceSpinBox = new QDoubleSpinBox(this);
        m_varianceSpinBox->setToolTip("Random variance (±)");

        if (def.isFloat) {
            m_varianceSpinBox->setDecimals(1);
            m_varianceSpinBox->setRange(0.0, 10000.0);
            m_varianceSpinBox->setSingleStep(1.0);
        } else {
            m_varianceSpinBox->setDecimals(0);
            m_varianceSpinBox->setRange(0, 100000);
            m_varianceSpinBox->setSingleStep(1);
        }
        m_varianceSpinBox->setMinimumWidth(60);
        connect(m_varianceSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
                this, &AIValueEditor::valueChanged);
        layout->addWidget(m_varianceSpinBox);

        layout->addStretch();
    }
}

void AIValueEditor::setValue(float value, float variance) {
    if (m_boolCheckBox) {
        m_boolCheckBox->blockSignals(true);
        m_boolCheckBox->setChecked(value != 0);
        m_boolCheckBox->blockSignals(false);
    } else {
        m_valueSpinBox->blockSignals(true);
        m_varianceSpinBox->blockSignals(true);
        m_valueSpinBox->setValue(value);
        m_varianceSpinBox->setValue(variance);
        m_valueSpinBox->blockSignals(false);
        m_varianceSpinBox->blockSignals(false);
    }
}

float AIValueEditor::getValue() const {
    if (m_boolCheckBox) {
        return m_boolCheckBox->isChecked() ? 1.0f : 0.0f;
    }
    return static_cast<float>(m_valueSpinBox->value());
}

float AIValueEditor::getVariance() const {
    if (m_boolCheckBox || !m_varianceSpinBox) {
        return 0.0f;
    }
    return static_cast<float>(m_varianceSpinBox->value());
}

// ============================================================================
// AIValuesWidget - Main AI Values editor
// ============================================================================

AIValuesWidget::AIValuesWidget(QWidget* parent)
    : QWidget(parent), m_values(nullptr)
{
    setupUI();
    createEditors();
}

void AIValuesWidget::setupUI() {
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    m_scrollArea = new QScrollArea(this);
    m_scrollArea->setWidgetResizable(true);
    m_scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    m_contentWidget = new QWidget();
    m_scrollArea->setWidget(m_contentWidget);

    mainLayout->addWidget(m_scrollArea);
}

void AIValuesWidget::createEditors() {
    QVBoxLayout* contentLayout = new QVBoxLayout(m_contentWidget);
    contentLayout->setSpacing(10);

    QVector<AIValueDef> knownValues = getKnownAIValues();

    // Group by category
    QMap<QString, QVector<AIValueDef>> categories;
    QStringList categoryOrder = {"Combat", "Groups", "Economy", "Distances"};

    for (const AIValueDef& def : knownValues) {
        categories[def.category].append(def);
    }

    // Create group boxes for each category
    for (const QString& category : categoryOrder) {
        if (!categories.contains(category)) continue;

        QGroupBox* groupBox = new QGroupBox(category, m_contentWidget);
        QVBoxLayout* groupLayout = new QVBoxLayout(groupBox);
        groupLayout->setSpacing(4);

        for (const AIValueDef& def : categories[category]) {
            AIValueEditor* editor = new AIValueEditor(def, groupBox);
            connect(editor, &AIValueEditor::valueChanged, this, &AIValuesWidget::onValueChanged);
            groupLayout->addWidget(editor);
            m_editors[def.name] = editor;
        }

        m_categoryGroups[category] = groupBox;
        contentLayout->addWidget(groupBox);
    }

    contentLayout->addStretch();
}

void AIValuesWidget::setAIValues(AIValues* values) {
    m_values = values;
    updateFromData();
}

void AIValuesWidget::clear() {
    m_values = nullptr;

    // Reset all editors to defaults
    QVector<AIValueDef> knownValues = getKnownAIValues();
    for (const AIValueDef& def : knownValues) {
        if (m_editors.contains(def.name)) {
            m_editors[def.name]->setValue(def.defaultValue, def.defaultVariance);
        }
    }
}

void AIValuesWidget::updateFromData() {
    if (!m_values) {
        clear();
        return;
    }

    QVector<AIValueDef> knownValues = getKnownAIValues();

    for (const AIValueDef& def : knownValues) {
        if (m_editors.contains(def.name)) {
            float value = m_values->getValue(def.name, def.defaultValue);
            float variance = m_values->getVariance(def.name, def.defaultVariance);
            m_editors[def.name]->setValue(value, variance);
        }
    }
}

void AIValuesWidget::applyChanges() {
    if (!m_values) return;

    for (auto it = m_editors.begin(); it != m_editors.end(); ++it) {
        AIValueEditor* editor = it.value();
        m_values->setValue(editor->getName(),
                           editor->getValue(),
                           editor->getVariance(),
                           editor->isFloat(),
                           editor->isBool());
    }
}

void AIValuesWidget::onValueChanged() {
    if (m_values) {
        applyChanges();
    }
    emit valuesModified();
}

} // namespace AI
