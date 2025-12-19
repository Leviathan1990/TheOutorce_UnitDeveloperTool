#ifndef CUSTOMSETTINGSWIDGET_H
#define CUSTOMSETTINGSWIDGET_H

#include <QWidget>
#include <QTableWidget>
#include <QPushButton>
#include <QLineEdit>
#include <QComboBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QLabel>
#include <QListWidget>

#include "OpfStructs.h"

class CustomSettingsWidget : public QWidget
{
    Q_OBJECT

public:
    explicit CustomSettingsWidget(QWidget* parent = nullptr);

    void setObject(Opf::Object* object);
    void clear();

    static QStringList getKnownSettingNames();

signals:
    void settingsModified();

private slots:
    void onAddSetting();
    void onRemoveSetting();
    void onCellChanged(int row, int column);
    void onSelectionChanged();

private:
    void setupUI();
    void refreshTable();
    void updateButtonStates();

    Opf::Object* m_currentObject;

    QTableWidget* m_table;
    QPushButton* m_addButton;
    QPushButton* m_removeButton;
    QComboBox* m_nameCombo;
    QLineEdit* m_valueEdit;

    static const QStringList s_knownSettingNames;
};

class CanBuildUnitWidget : public QWidget
{
    Q_OBJECT

public:
    explicit CanBuildUnitWidget(QWidget* parent = nullptr);

    void setObject(Opf::Object* object);
    void setAvailableUnits(const QStringList& units);
    void clear();

signals:
    void settingsModified();

private slots:
    void onAddUnit();
    void onRemoveUnit();
    void onSelectionChanged();

private:
    void setupUI();
    void refreshList();
    void updateButtonStates();

    Opf::Object* m_currentObject;
    QStringList m_availableUnits;

    QListWidget* m_unitList;
    QComboBox* m_unitCombo;
    QPushButton* m_addButton;
    QPushButton* m_removeButton;
};

#endif // CUSTOMSETTINGSWIDGET_H
