#ifndef AIVALUESWIDGET_H
#define AIVALUESWIDGET_H

#include <QWidget>
#include <QScrollArea>
#include <QGroupBox>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QCheckBox>
#include <QLabel>
#include <QMap>
#include "AIStructs.h"

namespace AI {

// Widget for editing a single AI value with variance
class AIValueEditor : public QWidget {
    Q_OBJECT
public:
    explicit AIValueEditor(const AIValueDef& def, QWidget* parent = nullptr);

    void setValue(float value, float variance);
    float getValue() const;
    float getVariance() const;
    QString getName() const { return m_def.name; }
    bool isBool() const { return m_def.isBool; }
    bool isFloat() const { return m_def.isFloat; }

signals:
    void valueChanged();

private:
    AIValueDef m_def;

    // For int/float values
    QDoubleSpinBox* m_valueSpinBox;
    QDoubleSpinBox* m_varianceSpinBox;

    // For bool values
    QCheckBox* m_boolCheckBox;
};

// Main widget for editing all AI Values
class AIValuesWidget : public QWidget {
    Q_OBJECT
public:
    explicit AIValuesWidget(QWidget* parent = nullptr);

    void setAIValues(AIValues* values);
    void clear();

    // Apply UI changes to data
    void applyChanges();

signals:
    void valuesModified();

private slots:
    void onValueChanged();

private:
    void setupUI();
    void createEditors();
    void updateFromData();

    AIValues* m_values;
    QScrollArea* m_scrollArea;
    QWidget* m_contentWidget;

    // Category group boxes
    QMap<QString, QGroupBox*> m_categoryGroups;

    // Editors by name
    QMap<QString, AIValueEditor*> m_editors;
};

} // namespace AI

#endif // AIVALUESWIDGET_H
