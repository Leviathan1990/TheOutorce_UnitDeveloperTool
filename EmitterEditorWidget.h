#ifndef EMITTEREDITORWIDGET_H
#define EMITTEREDITORWIDGET_H

#include <QWidget>
#include <QTableWidget>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QLineEdit>
#include <QComboBox>
#include <QCheckBox>
#include <QLabel>
#include <QGroupBox>
#include <QPushButton>
#include <QScrollArea>

#include "EffectsStructs.h"

class GradientWidget;

class EmitterEditorWidget : public QWidget {
    Q_OBJECT

public:
    explicit EmitterEditorWidget(QWidget* parent = nullptr);

    void setEmitter(Effects::Emitter* emitter);
    Effects::Emitter* currentEmitter() const { return m_emitter; }

    void setMaterialsList(const QStringList& materials);

signals:
    void emitterModified();

private slots:
    void onParameterChanged();
    void onGradientChanged();
    void onAddGradientPoint();
    void onRemoveGradientPoint();

private:
    Effects::Emitter* m_emitter;
    bool m_updating;

    // Basic info
    QSpinBox* m_idSpin;
    QLineEdit* m_nameEdit;
    QComboBox* m_materialCombo;

    // Particle settings
    QDoubleSpinBox* m_birthRateSpin;
    QSpinBox* m_numParticlesSpin;
    QDoubleSpinBox* m_lifeTimeSpin;
    QDoubleSpinBox* m_variableLifeTimeSpin;
    QCheckBox* m_reuseParticlesCheck;
    QCheckBox* m_deathWishCheck;

    // Size settings
    QDoubleSpinBox* m_sizeSpin;
    QDoubleSpinBox* m_sizeVariationSpin;
    QDoubleSpinBox* m_shrinkSpin;

    // Speed settings
    QDoubleSpinBox* m_speedSpin;
    QDoubleSpinBox* m_speedModifierSpin;
    QDoubleSpinBox* m_speedVariationSpin;
    QDoubleSpinBox* m_speedVariationCoeffSpin;
    QDoubleSpinBox* m_accelerationSpin;

    // Spread settings
    QDoubleSpinBox* m_spreadSpin;
    QDoubleSpinBox* m_spreadAroundSpin;
    QDoubleSpinBox* m_spreadAroundStartSpin;
    QDoubleSpinBox* m_spreadCoefficientSpin;

    // Position variation
    QDoubleSpinBox* m_posVarXSpin;
    QDoubleSpinBox* m_posVarYSpin;
    QDoubleSpinBox* m_posVarZSpin;

    // Gradient
    QTableWidget* m_gradientTable;
    GradientWidget* m_gradientPreview;
    QPushButton* m_addGradientBtn;
    QPushButton* m_removeGradientBtn;

    void setupUI();
    QGroupBox* createBasicGroup();
    QGroupBox* createParticleGroup();
    QGroupBox* createSizeGroup();
    QGroupBox* createSpeedGroup();
    QGroupBox* createSpreadGroup();
    QGroupBox* createPositionGroup();
    QGroupBox* createGradientGroup();

    void updateFromEmitter();
    void updateGradientTable();

    QScrollArea* m_scrollArea;
};

// Gradient preview widget
class GradientWidget : public QWidget {
    Q_OBJECT

public:
    explicit GradientWidget(QWidget* parent = nullptr);
    void setGradientPoints(const QVector<Effects::GradientPoint>& points);

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    QVector<Effects::GradientPoint> m_points;
};

#endif // EMITTEREDITORWIDGET_H
