#include "EmitterEditorWidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QFormLayout>
#include <QScrollArea>
#include <QHeaderView>
#include <QPainter>
#include <QLinearGradient>
#include <cmath>

// ============================================================================
// GRADIENT WIDGET
// ============================================================================

GradientWidget::GradientWidget(QWidget* parent) : QWidget(parent) {
    setMinimumHeight(40);
    setMaximumHeight(60);
}

void GradientWidget::setGradientPoints(const QVector<Effects::GradientPoint>& points) {
    m_points = points;
    update();
}

void GradientWidget::paintEvent(QPaintEvent*) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    painter.fillRect(rect(), Qt::darkGray);

    if (m_points.isEmpty()) {
        painter.setPen(Qt::white);
        painter.drawText(rect(), Qt::AlignCenter, "No gradient points");
        return;
    }

    QVector<Effects::GradientPoint> sorted = m_points;
    std::sort(sorted.begin(), sorted.end(), [](const Effects::GradientPoint& a, const Effects::GradientPoint& b) {
        return a.position < b.position;
    });

    QLinearGradient gradient(0, 0, width(), 0);
    for (const Effects::GradientPoint& p : sorted) {
        float pos = p.position / 1000.0f;
        gradient.setColorAt(pos, p.toQColor());
    }

    painter.fillRect(rect(), gradient);

    painter.setPen(QPen(Qt::white, 2));
    for (const Effects::GradientPoint& p : sorted) {
        int x = (p.position * width()) / 1000;
        painter.drawLine(x, 0, x, 8);
        painter.drawLine(x, height() - 8, x, height());
    }
}

// ============================================================================
// EMITTER EDITOR WIDGET
// ============================================================================

EmitterEditorWidget::EmitterEditorWidget(QWidget* parent) : QWidget(parent), m_emitter(nullptr), m_updating(false)
{
    setupUI();
}

void EmitterEditorWidget::setupUI() {
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    m_scrollArea = new QScrollArea(this);
    m_scrollArea->setWidgetResizable(true);
    m_scrollArea->setFrameShape(QFrame::NoFrame);

    QWidget* scrollContent = new QWidget();
    QVBoxLayout* contentLayout = new QVBoxLayout(scrollContent);

    QGroupBox* basic = createBasicGroup();
    QGroupBox* particle = createParticleGroup();
    QGroupBox* size = createSizeGroup();
    QGroupBox* speed = createSpeedGroup();
    QGroupBox* spread = createSpreadGroup();
    QGroupBox* position = createPositionGroup();
    QGroupBox* gradient = createGradientGroup();

    // Prevent groupboxes from shrinking
    basic->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    particle->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    size->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    speed->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    spread->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    position->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);

    contentLayout->addWidget(basic);
    contentLayout->addWidget(particle);
    contentLayout->addWidget(size);
    contentLayout->addWidget(speed);
    contentLayout->addWidget(spread);
    contentLayout->addWidget(position);
    contentLayout->addWidget(gradient);
    contentLayout->addStretch();

    m_scrollArea->setWidget(scrollContent);
    mainLayout->addWidget(m_scrollArea);
}

QGroupBox* EmitterEditorWidget::createBasicGroup() {
    QGroupBox* group = new QGroupBox("Basic Info", this);
    QFormLayout* layout = new QFormLayout(group);

    m_idSpin = new QSpinBox(this);
    m_idSpin->setRange(1, 9999);
    connect(m_idSpin, QOverload<int>::of(&QSpinBox::valueChanged), this, &EmitterEditorWidget::onParameterChanged);
    layout->addRow("ID:", m_idSpin);

    m_nameEdit = new QLineEdit(this);
    connect(m_nameEdit, &QLineEdit::textChanged, this, &EmitterEditorWidget::onParameterChanged);
    layout->addRow("Name:", m_nameEdit);

    m_materialCombo = new QComboBox(this);
    m_materialCombo->setEditable(true);
    connect(m_materialCombo, &QComboBox::currentTextChanged, this, &EmitterEditorWidget::onParameterChanged);
    layout->addRow("Material:", m_materialCombo);

    return group;
}

QGroupBox* EmitterEditorWidget::createParticleGroup() {
    QGroupBox* group = new QGroupBox("Particle Settings", this);
    QGridLayout* layout = new QGridLayout(group);

    layout->addWidget(new QLabel("Birth Rate:", this), 0, 0);
    m_birthRateSpin = new QDoubleSpinBox(this);
    m_birthRateSpin->setRange(0, 10000);
    m_birthRateSpin->setDecimals(2);
    connect(m_birthRateSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &EmitterEditorWidget::onParameterChanged);
    layout->addWidget(m_birthRateSpin, 0, 1);
    layout->addWidget(new QLabel("particles/sec", this), 0, 2);

    layout->addWidget(new QLabel("Max Particles:", this), 1, 0);
    m_numParticlesSpin = new QSpinBox(this);
    m_numParticlesSpin->setRange(1, 10000);
    connect(m_numParticlesSpin, QOverload<int>::of(&QSpinBox::valueChanged), this, &EmitterEditorWidget::onParameterChanged);
    layout->addWidget(m_numParticlesSpin, 1, 1);

    layout->addWidget(new QLabel("Lifetime:", this), 2, 0);
    m_lifeTimeSpin = new QDoubleSpinBox(this);
    m_lifeTimeSpin->setRange(0.001, 100);
    m_lifeTimeSpin->setDecimals(3);
    connect(m_lifeTimeSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &EmitterEditorWidget::onParameterChanged);
    layout->addWidget(m_lifeTimeSpin, 2, 1);
    layout->addWidget(new QLabel("seconds", this), 2, 2);

    layout->addWidget(new QLabel("Lifetime Var:", this), 3, 0);
    m_variableLifeTimeSpin = new QDoubleSpinBox(this);
    m_variableLifeTimeSpin->setRange(0, 100);
    m_variableLifeTimeSpin->setDecimals(3);
    connect(m_variableLifeTimeSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &EmitterEditorWidget::onParameterChanged);
    layout->addWidget(m_variableLifeTimeSpin, 3, 1);

    m_reuseParticlesCheck = new QCheckBox("Reuse Particles", this);
    connect(m_reuseParticlesCheck, &QCheckBox::toggled, this, &EmitterEditorWidget::onParameterChanged);
    layout->addWidget(m_reuseParticlesCheck, 4, 0, 1, 2);

    m_deathWishCheck = new QCheckBox("Death Wish (destroy when done)", this);
    connect(m_deathWishCheck, &QCheckBox::toggled, this, &EmitterEditorWidget::onParameterChanged);
    layout->addWidget(m_deathWishCheck, 5, 0, 1, 2);

    return group;
}

QGroupBox* EmitterEditorWidget::createSizeGroup() {
    QGroupBox* group = new QGroupBox("Size Settings", this);
    QGridLayout* layout = new QGridLayout(group);

    layout->addWidget(new QLabel("Size:", this), 0, 0);
    m_sizeSpin = new QDoubleSpinBox(this);
    m_sizeSpin->setRange(0.001, 100);
    m_sizeSpin->setDecimals(3);
    connect(m_sizeSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &EmitterEditorWidget::onParameterChanged);
    layout->addWidget(m_sizeSpin, 0, 1);

    layout->addWidget(new QLabel("Size Variation:", this), 1, 0);
    m_sizeVariationSpin = new QDoubleSpinBox(this);
    m_sizeVariationSpin->setRange(0, 10);
    m_sizeVariationSpin->setDecimals(3);
    connect(m_sizeVariationSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &EmitterEditorWidget::onParameterChanged);
    layout->addWidget(m_sizeVariationSpin, 1, 1);

    layout->addWidget(new QLabel("Shrink:", this), 2, 0);
    m_shrinkSpin = new QDoubleSpinBox(this);
    m_shrinkSpin->setRange(-100, 100);
    m_shrinkSpin->setDecimals(3);
    connect(m_shrinkSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &EmitterEditorWidget::onParameterChanged);
    layout->addWidget(m_shrinkSpin, 2, 1);

    return group;
}

QGroupBox* EmitterEditorWidget::createSpeedGroup() {
    QGroupBox* group = new QGroupBox("Speed Settings", this);
    QGridLayout* layout = new QGridLayout(group);

    layout->addWidget(new QLabel("Speed:", this), 0, 0);
    m_speedSpin = new QDoubleSpinBox(this);
    m_speedSpin->setRange(-100, 100);
    m_speedSpin->setDecimals(3);
    connect(m_speedSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &EmitterEditorWidget::onParameterChanged);
    layout->addWidget(m_speedSpin, 0, 1);

    layout->addWidget(new QLabel("Speed Modifier:", this), 1, 0);
    m_speedModifierSpin = new QDoubleSpinBox(this);
    m_speedModifierSpin->setRange(-100, 100);
    m_speedModifierSpin->setDecimals(3);
    connect(m_speedModifierSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &EmitterEditorWidget::onParameterChanged);
    layout->addWidget(m_speedModifierSpin, 1, 1);

    layout->addWidget(new QLabel("Speed Variation:", this), 2, 0);
    m_speedVariationSpin = new QDoubleSpinBox(this);
    m_speedVariationSpin->setRange(-100, 100);
    m_speedVariationSpin->setDecimals(3);
    connect(m_speedVariationSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &EmitterEditorWidget::onParameterChanged);
    layout->addWidget(m_speedVariationSpin, 2, 1);

    layout->addWidget(new QLabel("Speed Var Coeff:", this), 3, 0);
    m_speedVariationCoeffSpin = new QDoubleSpinBox(this);
    m_speedVariationCoeffSpin->setRange(-100, 100);
    m_speedVariationCoeffSpin->setDecimals(3);
    connect(m_speedVariationCoeffSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &EmitterEditorWidget::onParameterChanged);
    layout->addWidget(m_speedVariationCoeffSpin, 3, 1);

    layout->addWidget(new QLabel("Acceleration:", this), 4, 0);
    m_accelerationSpin = new QDoubleSpinBox(this);
    m_accelerationSpin->setRange(-100, 100);
    m_accelerationSpin->setDecimals(3);
    connect(m_accelerationSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &EmitterEditorWidget::onParameterChanged);
    layout->addWidget(m_accelerationSpin, 4, 1);

    return group;
}

QGroupBox* EmitterEditorWidget::createSpreadGroup() {
    QGroupBox* group = new QGroupBox("Spread Settings (radians)", this);
    QGridLayout* layout = new QGridLayout(group);

    layout->addWidget(new QLabel("Spread:", this), 0, 0);
    m_spreadSpin = new QDoubleSpinBox(this);
    m_spreadSpin->setRange(0, 12.57);
    m_spreadSpin->setDecimals(6);
    connect(m_spreadSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &EmitterEditorWidget::onParameterChanged);
    layout->addWidget(m_spreadSpin, 0, 1);
    layout->addWidget(new QLabel("(6.28 = 360°)", this), 0, 2);

    layout->addWidget(new QLabel("Spread Around:", this), 1, 0);
    m_spreadAroundSpin = new QDoubleSpinBox(this);
    m_spreadAroundSpin->setRange(-12.57, 12.57);
    m_spreadAroundSpin->setDecimals(6);
    connect(m_spreadAroundSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &EmitterEditorWidget::onParameterChanged);
    layout->addWidget(m_spreadAroundSpin, 1, 1);

    layout->addWidget(new QLabel("Spread Around Start:", this), 2, 0);
    m_spreadAroundStartSpin = new QDoubleSpinBox(this);
    m_spreadAroundStartSpin->setRange(-12.57, 12.57);
    m_spreadAroundStartSpin->setDecimals(6);
    connect(m_spreadAroundStartSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &EmitterEditorWidget::onParameterChanged);
    layout->addWidget(m_spreadAroundStartSpin, 2, 1);

    layout->addWidget(new QLabel("Spread Coefficient:", this), 3, 0);
    m_spreadCoefficientSpin = new QDoubleSpinBox(this);
    m_spreadCoefficientSpin->setRange(-100, 100);
    m_spreadCoefficientSpin->setDecimals(3);
    connect(m_spreadCoefficientSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &EmitterEditorWidget::onParameterChanged);
    layout->addWidget(m_spreadCoefficientSpin, 3, 1);

    return group;
}

QGroupBox* EmitterEditorWidget::createPositionGroup() {
    QGroupBox* group = new QGroupBox("Position Variation", this);
    QGridLayout* layout = new QGridLayout(group);

    layout->addWidget(new QLabel("X:", this), 0, 0);
    m_posVarXSpin = new QDoubleSpinBox(this);
    m_posVarXSpin->setRange(0, 100);
    m_posVarXSpin->setDecimals(3);
    connect(m_posVarXSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &EmitterEditorWidget::onParameterChanged);
    layout->addWidget(m_posVarXSpin, 0, 1);

    layout->addWidget(new QLabel("Y:", this), 0, 2);
    m_posVarYSpin = new QDoubleSpinBox(this);
    m_posVarYSpin->setRange(0, 100);
    m_posVarYSpin->setDecimals(3);
    connect(m_posVarYSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &EmitterEditorWidget::onParameterChanged);
    layout->addWidget(m_posVarYSpin, 0, 3);

    layout->addWidget(new QLabel("Z:", this), 0, 4);
    m_posVarZSpin = new QDoubleSpinBox(this);
    m_posVarZSpin->setRange(0, 100);
    m_posVarZSpin->setDecimals(3);
    connect(m_posVarZSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &EmitterEditorWidget::onParameterChanged);
    layout->addWidget(m_posVarZSpin, 0, 5);

    return group;
}

QGroupBox* EmitterEditorWidget::createGradientGroup() {
    QGroupBox* group = new QGroupBox("Color Gradient", this);
    QVBoxLayout* layout = new QVBoxLayout(group);

    m_gradientPreview = new GradientWidget(this);
    layout->addWidget(m_gradientPreview);

    m_gradientTable = new QTableWidget(this);
    m_gradientTable->setColumnCount(6);
    m_gradientTable->setHorizontalHeaderLabels({"Position", "Red", "Green", "Blue", "Alpha", "Color"});
    m_gradientTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_gradientTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_gradientTable->setMinimumHeight(150);
    connect(m_gradientTable, &QTableWidget::cellChanged, this, &EmitterEditorWidget::onGradientChanged);
    layout->addWidget(m_gradientTable);

    QHBoxLayout* btnLayout = new QHBoxLayout();
    m_addGradientBtn = new QPushButton("Add Point", this);
    connect(m_addGradientBtn, &QPushButton::clicked, this, &EmitterEditorWidget::onAddGradientPoint);
    btnLayout->addWidget(m_addGradientBtn);

    m_removeGradientBtn = new QPushButton("Remove Point", this);
    connect(m_removeGradientBtn, &QPushButton::clicked, this, &EmitterEditorWidget::onRemoveGradientPoint);
    btnLayout->addWidget(m_removeGradientBtn);

    btnLayout->addStretch();
    layout->addLayout(btnLayout);

    return group;
}

void EmitterEditorWidget::setEmitter(Effects::Emitter* emitter) {
    m_emitter = emitter;
    updateFromEmitter();

    // Force scroll area to recalculate
    if (QScrollArea* scrollArea = findChild<QScrollArea*>()) {
        scrollArea->widget()->adjustSize();
    }
}

void EmitterEditorWidget::setMaterialsList(const QStringList& materials) {
    QString current = m_materialCombo->currentText();
    m_materialCombo->clear();
    m_materialCombo->addItems(materials);
    if (!current.isEmpty()) {
        m_materialCombo->setCurrentText(current);
    }
}

void EmitterEditorWidget::updateFromEmitter() {
    if (!m_emitter) {
        if (m_scrollArea && m_scrollArea->widget()) {
            m_scrollArea->widget()->setEnabled(false);
        }
        return;
    }

    m_updating = true;

    if (m_scrollArea && m_scrollArea->widget()) {
        m_scrollArea->widget()->setEnabled(true);
    }

    m_idSpin->setValue(m_emitter->id);
    m_nameEdit->setText(m_emitter->name);
    m_materialCombo->setCurrentText(m_emitter->material);

    m_birthRateSpin->setValue(m_emitter->birthRate);
    m_numParticlesSpin->setValue(m_emitter->numParticles);
    m_lifeTimeSpin->setValue(m_emitter->lifeTime);
    m_variableLifeTimeSpin->setValue(m_emitter->variableLifeTime);
    m_reuseParticlesCheck->setChecked(m_emitter->reuseParticles != 0);
    m_deathWishCheck->setChecked(m_emitter->deathWish != 0);

    m_sizeSpin->setValue(m_emitter->size);
    m_sizeVariationSpin->setValue(m_emitter->sizeVariation);
    m_shrinkSpin->setValue(m_emitter->shrink);

    m_speedSpin->setValue(m_emitter->speed);
    m_speedModifierSpin->setValue(m_emitter->speedModifier);
    m_speedVariationSpin->setValue(m_emitter->speedVariation);
    m_speedVariationCoeffSpin->setValue(m_emitter->speedVariationCoeff);
    m_accelerationSpin->setValue(m_emitter->acceleration);

    m_spreadSpin->setValue(m_emitter->spread);
    m_spreadAroundSpin->setValue(m_emitter->spreadAround);
    m_spreadAroundStartSpin->setValue(m_emitter->spreadAroundStart);
    m_spreadCoefficientSpin->setValue(m_emitter->spreadCoefficient);

    m_posVarXSpin->setValue(m_emitter->posVariationX);
    m_posVarYSpin->setValue(m_emitter->posVariationY);
    m_posVarZSpin->setValue(m_emitter->posVariationZ);

    updateGradientTable();

    m_updating = false;
}

void EmitterEditorWidget::updateGradientTable() {
    if (!m_emitter) return;

    m_updating = true;
    m_gradientTable->setRowCount(m_emitter->gradientPoints.size());

    for (int i = 0; i < m_emitter->gradientPoints.size(); i++) {
        const Effects::GradientPoint& p = m_emitter->gradientPoints[i];

        m_gradientTable->setItem(i, 0, new QTableWidgetItem(QString::number(p.position)));
        m_gradientTable->setItem(i, 1, new QTableWidgetItem(QString::number(p.r, 'f', 3)));
        m_gradientTable->setItem(i, 2, new QTableWidgetItem(QString::number(p.g, 'f', 3)));
        m_gradientTable->setItem(i, 3, new QTableWidgetItem(QString::number(p.b, 'f', 3)));
        m_gradientTable->setItem(i, 4, new QTableWidgetItem(QString::number(p.alpha, 'f', 3)));

        QTableWidgetItem* colorItem = new QTableWidgetItem();
        colorItem->setBackground(p.toQColor());
        colorItem->setFlags(colorItem->flags() & ~Qt::ItemIsEditable);
        m_gradientTable->setItem(i, 5, colorItem);
    }

    m_gradientPreview->setGradientPoints(m_emitter->gradientPoints);
    m_updating = false;
}

void EmitterEditorWidget::onParameterChanged() {
    if (m_updating || !m_emitter) return;

    m_emitter->id = m_idSpin->value();
    m_emitter->name = m_nameEdit->text();
    m_emitter->material = m_materialCombo->currentText();

    m_emitter->birthRate = m_birthRateSpin->value();
    m_emitter->numParticles = m_numParticlesSpin->value();
    m_emitter->lifeTime = m_lifeTimeSpin->value();
    m_emitter->variableLifeTime = m_variableLifeTimeSpin->value();
    m_emitter->reuseParticles = m_reuseParticlesCheck->isChecked() ? 1 : 0;
    m_emitter->deathWish = m_deathWishCheck->isChecked() ? 1 : 0;

    m_emitter->size = m_sizeSpin->value();
    m_emitter->sizeVariation = m_sizeVariationSpin->value();
    m_emitter->shrink = m_shrinkSpin->value();

    m_emitter->speed = m_speedSpin->value();
    m_emitter->speedModifier = m_speedModifierSpin->value();
    m_emitter->speedVariation = m_speedVariationSpin->value();
    m_emitter->speedVariationCoeff = m_speedVariationCoeffSpin->value();
    m_emitter->acceleration = m_accelerationSpin->value();

    m_emitter->spread = m_spreadSpin->value();
    m_emitter->spreadAround = m_spreadAroundSpin->value();
    m_emitter->spreadAroundStart = m_spreadAroundStartSpin->value();
    m_emitter->spreadCoefficient = m_spreadCoefficientSpin->value();

    m_emitter->posVariationX = m_posVarXSpin->value();
    m_emitter->posVariationY = m_posVarYSpin->value();
    m_emitter->posVariationZ = m_posVarZSpin->value();

    emit emitterModified();
}

void EmitterEditorWidget::onGradientChanged() {
    if (m_updating || !m_emitter) return;

    for (int i = 0; i < m_gradientTable->rowCount() && i < m_emitter->gradientPoints.size(); i++) {
        Effects::GradientPoint& p = m_emitter->gradientPoints[i];

        if (m_gradientTable->item(i, 0)) p.position = m_gradientTable->item(i, 0)->text().toInt();
        if (m_gradientTable->item(i, 1)) p.r = m_gradientTable->item(i, 1)->text().toFloat();
        if (m_gradientTable->item(i, 2)) p.g = m_gradientTable->item(i, 2)->text().toFloat();
        if (m_gradientTable->item(i, 3)) p.b = m_gradientTable->item(i, 3)->text().toFloat();
        if (m_gradientTable->item(i, 4)) p.alpha = m_gradientTable->item(i, 4)->text().toFloat();
    }

    updateGradientTable();
    emit emitterModified();
}

void EmitterEditorWidget::onAddGradientPoint() {
    if (!m_emitter) return;

    Effects::GradientPoint newPoint;
    newPoint.position = 1000;

    m_emitter->gradientPoints.append(newPoint);
    updateGradientTable();
    emit emitterModified();
}

void EmitterEditorWidget::onRemoveGradientPoint() {
    if (!m_emitter) return;

    int row = m_gradientTable->currentRow();
    if (row >= 0 && row < m_emitter->gradientPoints.size()) {
        m_emitter->gradientPoints.remove(row);
        updateGradientTable();
        emit emitterModified();
    }
}
