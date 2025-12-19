#include "ColorsEditorWidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QFormLayout>
#include <QScrollArea>
#include <QPainter>
#include <QColorDialog>

// ============================================================================
// COLOR PREVIEW WIDGET
// ============================================================================

ColorPreviewWidget::ColorPreviewWidget(QWidget* parent)
    : QWidget(parent), m_color(Qt::black)
{
    setMinimumSize(40, 25);
    setMaximumSize(60, 25);
}

void ColorPreviewWidget::setColor(int r, int g, int b) {
    m_color = QColor(qBound(0, r, 255), qBound(0, g, 255), qBound(0, b, 255));
    update();
}

void ColorPreviewWidget::paintEvent(QPaintEvent*) {
    QPainter painter(this);
    painter.fillRect(rect(), m_color);
    painter.setPen(Qt::black);
    painter.drawRect(rect().adjusted(0, 0, -1, -1));
}

// ============================================================================
// COLORS EDITOR WIDGET
// ============================================================================

ColorsEditorWidget::ColorsEditorWidget(QWidget* parent)
    : QWidget(parent), m_colors(nullptr), m_updating(false)
{
    setupUI();
}

void ColorsEditorWidget::setupUI() {
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    QScrollArea* scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);

    QWidget* scrollContent = new QWidget();
    QVBoxLayout* contentLayout = new QVBoxLayout(scrollContent);

    contentLayout->addWidget(createAmbientGroup());
    contentLayout->addWidget(createBackgroundGroup());
    contentLayout->addWidget(createSunGroup());
    contentLayout->addWidget(createFogGroup());
    contentLayout->addWidget(createLightOfGodGroup());
    contentLayout->addStretch();

    scrollArea->setWidget(scrollContent);
    mainLayout->addWidget(scrollArea);
}

QGroupBox* ColorsEditorWidget::createAmbientGroup() {
    QGroupBox* group = new QGroupBox("Ambient Light", this);
    QHBoxLayout* layout = new QHBoxLayout(group);

    layout->addWidget(new QLabel("R:", this));
    m_ambientR = new QSpinBox(this);
    m_ambientR->setRange(0, 500);
    connect(m_ambientR, QOverload<int>::of(&QSpinBox::valueChanged), this, &ColorsEditorWidget::onParameterChanged);
    layout->addWidget(m_ambientR);

    layout->addWidget(new QLabel("G:", this));
    m_ambientG = new QSpinBox(this);
    m_ambientG->setRange(0, 500);
    connect(m_ambientG, QOverload<int>::of(&QSpinBox::valueChanged), this, &ColorsEditorWidget::onParameterChanged);
    layout->addWidget(m_ambientG);

    layout->addWidget(new QLabel("B:", this));
    m_ambientB = new QSpinBox(this);
    m_ambientB->setRange(0, 500);
    connect(m_ambientB, QOverload<int>::of(&QSpinBox::valueChanged), this, &ColorsEditorWidget::onParameterChanged);
    layout->addWidget(m_ambientB);

    m_ambientPreview = new ColorPreviewWidget(this);
    layout->addWidget(m_ambientPreview);

    QPushButton* pickBtn = new QPushButton("Pick...", this);
    connect(pickBtn, &QPushButton::clicked, this, &ColorsEditorWidget::onPickAmbient);
    layout->addWidget(pickBtn);

    layout->addStretch();

    return group;
}

QGroupBox* ColorsEditorWidget::createBackgroundGroup() {
    QGroupBox* group = new QGroupBox("Background Color", this);
    QHBoxLayout* layout = new QHBoxLayout(group);

    layout->addWidget(new QLabel("R:", this));
    m_bgR = new QSpinBox(this);
    m_bgR->setRange(0, 500);
    connect(m_bgR, QOverload<int>::of(&QSpinBox::valueChanged), this, &ColorsEditorWidget::onParameterChanged);
    layout->addWidget(m_bgR);

    layout->addWidget(new QLabel("G:", this));
    m_bgG = new QSpinBox(this);
    m_bgG->setRange(0, 500);
    connect(m_bgG, QOverload<int>::of(&QSpinBox::valueChanged), this, &ColorsEditorWidget::onParameterChanged);
    layout->addWidget(m_bgG);

    layout->addWidget(new QLabel("B:", this));
    m_bgB = new QSpinBox(this);
    m_bgB->setRange(0, 500);
    connect(m_bgB, QOverload<int>::of(&QSpinBox::valueChanged), this, &ColorsEditorWidget::onParameterChanged);
    layout->addWidget(m_bgB);

    m_bgPreview = new ColorPreviewWidget(this);
    layout->addWidget(m_bgPreview);

    QPushButton* pickBtn = new QPushButton("Pick...", this);
    connect(pickBtn, &QPushButton::clicked, this, &ColorsEditorWidget::onPickBackground);
    layout->addWidget(pickBtn);

    layout->addStretch();

    return group;
}

QGroupBox* ColorsEditorWidget::createSunGroup() {
    QGroupBox* group = new QGroupBox("Sun Light", this);
    QGridLayout* layout = new QGridLayout(group);

    // Colors
    layout->addWidget(new QLabel("Color R:", this), 0, 0);
    m_sunR = new QSpinBox(this);
    m_sunR->setRange(0, 1000);
    connect(m_sunR, QOverload<int>::of(&QSpinBox::valueChanged), this, &ColorsEditorWidget::onParameterChanged);
    layout->addWidget(m_sunR, 0, 1);

    layout->addWidget(new QLabel("G:", this), 0, 2);
    m_sunG = new QSpinBox(this);
    m_sunG->setRange(0, 1000);
    connect(m_sunG, QOverload<int>::of(&QSpinBox::valueChanged), this, &ColorsEditorWidget::onParameterChanged);
    layout->addWidget(m_sunG, 0, 3);

    layout->addWidget(new QLabel("B:", this), 0, 4);
    m_sunB = new QSpinBox(this);
    m_sunB->setRange(0, 1000);
    connect(m_sunB, QOverload<int>::of(&QSpinBox::valueChanged), this, &ColorsEditorWidget::onParameterChanged);
    layout->addWidget(m_sunB, 0, 5);

    // Direction
    layout->addWidget(new QLabel("Pitch:", this), 1, 0);
    m_sunPitch = new QDoubleSpinBox(this);
    m_sunPitch->setRange(-3.15, 3.15);
    m_sunPitch->setDecimals(2);
    connect(m_sunPitch, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &ColorsEditorWidget::onParameterChanged);
    layout->addWidget(m_sunPitch, 1, 1);

    layout->addWidget(new QLabel("Heading:", this), 1, 2);
    m_sunHeading = new QDoubleSpinBox(this);
    m_sunHeading->setRange(-6.3, 6.3);
    m_sunHeading->setDecimals(2);
    connect(m_sunHeading, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &ColorsEditorWidget::onParameterChanged);
    layout->addWidget(m_sunHeading, 1, 3);

    return group;
}

QGroupBox* ColorsEditorWidget::createFogGroup() {
    QGroupBox* group = new QGroupBox("Fog", this);
    QGridLayout* layout = new QGridLayout(group);

    // Main fog
    layout->addWidget(new QLabel("Fog Color R:", this), 0, 0);
    m_fogR = new QSpinBox(this);
    m_fogR->setRange(0, 500);
    connect(m_fogR, QOverload<int>::of(&QSpinBox::valueChanged), this, &ColorsEditorWidget::onParameterChanged);
    layout->addWidget(m_fogR, 0, 1);

    layout->addWidget(new QLabel("G:", this), 0, 2);
    m_fogG = new QSpinBox(this);
    m_fogG->setRange(0, 500);
    connect(m_fogG, QOverload<int>::of(&QSpinBox::valueChanged), this, &ColorsEditorWidget::onParameterChanged);
    layout->addWidget(m_fogG, 0, 3);

    layout->addWidget(new QLabel("B:", this), 0, 4);
    m_fogB = new QSpinBox(this);
    m_fogB->setRange(0, 500);
    connect(m_fogB, QOverload<int>::of(&QSpinBox::valueChanged), this, &ColorsEditorWidget::onParameterChanged);
    layout->addWidget(m_fogB, 0, 5);

    m_fogPreview = new ColorPreviewWidget(this);
    layout->addWidget(m_fogPreview, 0, 6);

    QPushButton* pickBtn = new QPushButton("Pick...", this);
    connect(pickBtn, &QPushButton::clicked, this, &ColorsEditorWidget::onPickFog);
    layout->addWidget(pickBtn, 0, 7);

    // Half fog
    layout->addWidget(new QLabel("Fog Half R:", this), 1, 0);
    m_fogHalfR = new QSpinBox(this);
    m_fogHalfR->setRange(0, 500);
    connect(m_fogHalfR, QOverload<int>::of(&QSpinBox::valueChanged), this, &ColorsEditorWidget::onParameterChanged);
    layout->addWidget(m_fogHalfR, 1, 1);

    layout->addWidget(new QLabel("G:", this), 1, 2);
    m_fogHalfG = new QSpinBox(this);
    m_fogHalfG->setRange(0, 500);
    connect(m_fogHalfG, QOverload<int>::of(&QSpinBox::valueChanged), this, &ColorsEditorWidget::onParameterChanged);
    layout->addWidget(m_fogHalfG, 1, 3);

    layout->addWidget(new QLabel("B:", this), 1, 4);
    m_fogHalfB = new QSpinBox(this);
    m_fogHalfB->setRange(0, 500);
    connect(m_fogHalfB, QOverload<int>::of(&QSpinBox::valueChanged), this, &ColorsEditorWidget::onParameterChanged);
    layout->addWidget(m_fogHalfB, 1, 5);

    return group;
}

QGroupBox* ColorsEditorWidget::createLightOfGodGroup() {
    QGroupBox* group = new QGroupBox("Light of God (Volumetric)", this);
    QGridLayout* layout = new QGridLayout(group);

    // Settings
    layout->addWidget(new QLabel("Height:", this), 0, 0);
    m_logHeight = new QDoubleSpinBox(this);
    m_logHeight->setRange(0, 100);
    m_logHeight->setDecimals(2);
    connect(m_logHeight, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &ColorsEditorWidget::onParameterChanged);
    layout->addWidget(m_logHeight, 0, 1);

    layout->addWidget(new QLabel("Range:", this), 0, 2);
    m_logRange = new QDoubleSpinBox(this);
    m_logRange->setRange(0, 100);
    m_logRange->setDecimals(2);
    connect(m_logRange, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &ColorsEditorWidget::onParameterChanged);
    layout->addWidget(m_logRange, 0, 3);

    layout->addWidget(new QLabel("Strength:", this), 1, 0);
    m_logStrength = new QDoubleSpinBox(this);
    m_logStrength->setRange(0, 10);
    m_logStrength->setDecimals(2);
    connect(m_logStrength, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &ColorsEditorWidget::onParameterChanged);
    layout->addWidget(m_logStrength, 1, 1);

    layout->addWidget(new QLabel("Variation:", this), 1, 2);
    m_logVariation = new QSpinBox(this);
    m_logVariation->setRange(0, 100);
    connect(m_logVariation, QOverload<int>::of(&QSpinBox::valueChanged), this, &ColorsEditorWidget::onParameterChanged);
    layout->addWidget(m_logVariation, 1, 3);

    // Color
    layout->addWidget(new QLabel("Color R:", this), 2, 0);
    m_logColorR = new QDoubleSpinBox(this);
    m_logColorR->setRange(-10, 10);
    m_logColorR->setDecimals(2);
    connect(m_logColorR, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &ColorsEditorWidget::onParameterChanged);
    layout->addWidget(m_logColorR, 2, 1);

    layout->addWidget(new QLabel("G:", this), 2, 2);
    m_logColorG = new QDoubleSpinBox(this);
    m_logColorG->setRange(-10, 10);
    m_logColorG->setDecimals(2);
    connect(m_logColorG, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &ColorsEditorWidget::onParameterChanged);
    layout->addWidget(m_logColorG, 2, 3);

    layout->addWidget(new QLabel("B:", this), 2, 4);
    m_logColorB = new QDoubleSpinBox(this);
    m_logColorB->setRange(-10, 10);
    m_logColorB->setDecimals(2);
    connect(m_logColorB, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &ColorsEditorWidget::onParameterChanged);
    layout->addWidget(m_logColorB, 2, 5);

    return group;
}

void ColorsEditorWidget::setSceneColors(Effects::SceneColors* colors) {
    m_colors = colors;
    updateFromColors();
}

void ColorsEditorWidget::updateFromColors() {
    if (!m_colors) {
        setEnabled(false);
        return;
    }

    setEnabled(true);
    m_updating = true;

    m_ambientR->setValue(m_colors->ambientR);
    m_ambientG->setValue(m_colors->ambientG);
    m_ambientB->setValue(m_colors->ambientB);

    m_bgR->setValue(m_colors->bgColorR);
    m_bgG->setValue(m_colors->bgColorG);
    m_bgB->setValue(m_colors->bgColorB);

    m_sunR->setValue(m_colors->sunR);
    m_sunG->setValue(m_colors->sunG);
    m_sunB->setValue(m_colors->sunB);
    m_sunPitch->setValue(m_colors->sunPitch);
    m_sunHeading->setValue(m_colors->sunHeading);

    m_fogR->setValue(m_colors->fogR);
    m_fogG->setValue(m_colors->fogG);
    m_fogB->setValue(m_colors->fogB);
    m_fogHalfR->setValue(m_colors->fogHalfR);
    m_fogHalfG->setValue(m_colors->fogHalfG);
    m_fogHalfB->setValue(m_colors->fogHalfB);

    m_logHeight->setValue(m_colors->logHeight);
    m_logRange->setValue(m_colors->logRange);
    m_logStrength->setValue(m_colors->logStrength);
    m_logVariation->setValue(m_colors->logVariation);
    m_logColorR->setValue(m_colors->logColorR);
    m_logColorG->setValue(m_colors->logColorG);
    m_logColorB->setValue(m_colors->logColorB);

    updatePreviews();

    m_updating = false;
}

void ColorsEditorWidget::updatePreviews() {
    if (!m_colors) return;

    m_ambientPreview->setColor(m_colors->ambientR, m_colors->ambientG, m_colors->ambientB);
    m_bgPreview->setColor(m_colors->bgColorR, m_colors->bgColorG, m_colors->bgColorB);
    m_fogPreview->setColor(m_colors->fogR, m_colors->fogG, m_colors->fogB);
}

void ColorsEditorWidget::onParameterChanged() {
    if (m_updating || !m_colors) return;

    m_colors->ambientR = m_ambientR->value();
    m_colors->ambientG = m_ambientG->value();
    m_colors->ambientB = m_ambientB->value();

    m_colors->bgColorR = m_bgR->value();
    m_colors->bgColorG = m_bgG->value();
    m_colors->bgColorB = m_bgB->value();

    m_colors->sunR = m_sunR->value();
    m_colors->sunG = m_sunG->value();
    m_colors->sunB = m_sunB->value();
    m_colors->sunPitch = m_sunPitch->value();
    m_colors->sunHeading = m_sunHeading->value();

    m_colors->fogR = m_fogR->value();
    m_colors->fogG = m_fogG->value();
    m_colors->fogB = m_fogB->value();
    m_colors->fogHalfR = m_fogHalfR->value();
    m_colors->fogHalfG = m_fogHalfG->value();
    m_colors->fogHalfB = m_fogHalfB->value();

    m_colors->logHeight = m_logHeight->value();
    m_colors->logRange = m_logRange->value();
    m_colors->logStrength = m_logStrength->value();
    m_colors->logVariation = m_logVariation->value();
    m_colors->logColorR = m_logColorR->value();
    m_colors->logColorG = m_logColorG->value();
    m_colors->logColorB = m_logColorB->value();

    updatePreviews();
    emit colorsModified();
}

void ColorsEditorWidget::onPickAmbient() {
    if (!m_colors) return;

    QColor initial(qBound(0, m_colors->ambientR, 255),
                   qBound(0, m_colors->ambientG, 255),
                   qBound(0, m_colors->ambientB, 255));
    QColor color = QColorDialog::getColor(initial, this, "Pick Ambient Color");
    if (color.isValid()) {
        m_ambientR->setValue(color.red());
        m_ambientG->setValue(color.green());
        m_ambientB->setValue(color.blue());
    }
}

void ColorsEditorWidget::onPickBackground() {
    if (!m_colors) return;

    QColor initial(qBound(0, m_colors->bgColorR, 255),
                   qBound(0, m_colors->bgColorG, 255),
                   qBound(0, m_colors->bgColorB, 255));
    QColor color = QColorDialog::getColor(initial, this, "Pick Background Color");
    if (color.isValid()) {
        m_bgR->setValue(color.red());
        m_bgG->setValue(color.green());
        m_bgB->setValue(color.blue());
    }
}

void ColorsEditorWidget::onPickFog() {
    if (!m_colors) return;

    QColor initial(qBound(0, m_colors->fogR, 255),
                   qBound(0, m_colors->fogG, 255),
                   qBound(0, m_colors->fogB, 255));
    QColor color = QColorDialog::getColor(initial, this, "Pick Fog Color");
    if (color.isValid()) {
        m_fogR->setValue(color.red());
        m_fogG->setValue(color.green());
        m_fogB->setValue(color.blue());
    }
}
