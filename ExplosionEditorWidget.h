#ifndef EXPLOSIONEDITORWIDGET_H
#define EXPLOSIONEDITORWIDGET_H

#include <QWidget>
#include <QTableWidget>
#include <QDoubleSpinBox>
#include <QLineEdit>
#include <QGroupBox>
#include <QPushButton>
#include <QLabel>
#include <QCheckBox>

#include "EffectsStructs.h"

class ExplosionEditorWidget : public QWidget {
    Q_OBJECT

public:
    explicit ExplosionEditorWidget(QWidget* parent = nullptr);

    void setExplosion(Effects::Explosion* explosion);
    Effects::Explosion* currentExplosion() const { return m_explosion; }

signals:
    void explosionModified();

private slots:
    void onParameterChanged();
    void onObjectsTableChanged();
    void onThrowTableChanged();
    void onAddObject();
    void onRemoveObject();
    void onAddThrowObject();
    void onRemoveThrowObject();

private:
    Effects::Explosion* m_explosion;
    bool m_updating;

    // Basic info
    QLineEdit* m_nameEdit;
    QLineEdit* m_soundEdit;
    QLineEdit* m_debrisEdit;

    // Settings
    QDoubleSpinBox* m_scaleXSpin;
    QDoubleSpinBox* m_scaleYSpin;
    QDoubleSpinBox* m_scaleZSpin;
    QDoubleSpinBox* m_shakeSpin;
    QDoubleSpinBox* m_polyBlowSpeedSpin;

    // Pressure Wave
    QCheckBox* m_pressureWaveCheck;
    QDoubleSpinBox* m_pressureWaveForceSpin;
    QDoubleSpinBox* m_pressureWaveInnerSpin;
    QDoubleSpinBox* m_pressureWaveOuterSpin;

    // Objects table
    QTableWidget* m_objectsTable;
    QPushButton* m_addObjectBtn;
    QPushButton* m_removeObjectBtn;

    // Throw objects table
    QTableWidget* m_throwTable;
    QPushButton* m_addThrowBtn;
    QPushButton* m_removeThrowBtn;

    void setupUI();
    QGroupBox* createBasicGroup();
    QGroupBox* createSettingsGroup();
    QGroupBox* createObjectsGroup();
    QGroupBox* createThrowGroup();

    void updateFromExplosion();
    void updateObjectsTable();
    void updateThrowTable();
};

#endif // EXPLOSIONEDITORWIDGET_H
