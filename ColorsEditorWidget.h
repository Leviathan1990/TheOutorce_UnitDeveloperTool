#ifndef COLORSEDITORWIDGET_H
#define COLORSEDITORWIDGET_H

#include <QWidget>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QGroupBox>
#include <QPushButton>
#include <QLabel>

#include "EffectsStructs.h"

class ColorPreviewWidget;

class ColorsEditorWidget : public QWidget {
    Q_OBJECT

public:
    explicit ColorsEditorWidget(QWidget* parent = nullptr);

    void setSceneColors(Effects::SceneColors* colors);
    Effects::SceneColors* currentColors() const { return m_colors; }

signals:
    void colorsModified();

private slots:
    void onParameterChanged();
    void onPickAmbient();
    void onPickBackground();
    void onPickFog();

private:
    Effects::SceneColors* m_colors;
    bool m_updating;

    // Ambient
    QSpinBox* m_ambientR;
    QSpinBox* m_ambientG;
    QSpinBox* m_ambientB;
    ColorPreviewWidget* m_ambientPreview;

    // Background
    QSpinBox* m_bgR;
    QSpinBox* m_bgG;
    QSpinBox* m_bgB;
    ColorPreviewWidget* m_bgPreview;

    // Sun
    QSpinBox* m_sunR;
    QSpinBox* m_sunG;
    QSpinBox* m_sunB;
    QDoubleSpinBox* m_sunPitch;
    QDoubleSpinBox* m_sunHeading;

    // Fog
    QSpinBox* m_fogR;
    QSpinBox* m_fogG;
    QSpinBox* m_fogB;
    ColorPreviewWidget* m_fogPreview;
    QSpinBox* m_fogHalfR;
    QSpinBox* m_fogHalfG;
    QSpinBox* m_fogHalfB;

    // Light of God
    QDoubleSpinBox* m_logHeight;
    QDoubleSpinBox* m_logRange;
    QDoubleSpinBox* m_logStrength;
    QSpinBox* m_logVariation;
    QDoubleSpinBox* m_logColorR;
    QDoubleSpinBox* m_logColorG;
    QDoubleSpinBox* m_logColorB;

    void setupUI();
    QGroupBox* createAmbientGroup();
    QGroupBox* createBackgroundGroup();
    QGroupBox* createSunGroup();
    QGroupBox* createFogGroup();
    QGroupBox* createLightOfGodGroup();

    void updateFromColors();
    void updatePreviews();
};

// Simple color preview widget
class ColorPreviewWidget : public QWidget {
    Q_OBJECT
public:
    explicit ColorPreviewWidget(QWidget* parent = nullptr);
    void setColor(int r, int g, int b);
protected:
    void paintEvent(QPaintEvent* event) override;
private:
    QColor m_color;
};

#endif // COLORSEDITORWIDGET_H
