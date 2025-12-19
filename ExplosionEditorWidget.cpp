#include "ExplosionEditorWidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QFormLayout>
#include <QScrollArea>
#include <QHeaderView>
#include <QCheckBox>

ExplosionEditorWidget::ExplosionEditorWidget(QWidget* parent)
    : QWidget(parent), m_explosion(nullptr), m_updating(false)
{
    setupUI();
}

void ExplosionEditorWidget::setupUI() {
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    QScrollArea* scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);

    QWidget* scrollContent = new QWidget();
    QVBoxLayout* contentLayout = new QVBoxLayout(scrollContent);

    contentLayout->addWidget(createBasicGroup());
    contentLayout->addWidget(createSettingsGroup());
    contentLayout->addWidget(createObjectsGroup());
    contentLayout->addWidget(createThrowGroup());
    contentLayout->addStretch();

    scrollArea->setWidget(scrollContent);
    mainLayout->addWidget(scrollArea);
}

QGroupBox* ExplosionEditorWidget::createBasicGroup() {
    QGroupBox* group = new QGroupBox("Basic Info", this);
    QFormLayout* layout = new QFormLayout(group);

    m_nameEdit = new QLineEdit(this);
    m_nameEdit->setReadOnly(true);
    layout->addRow("Name:", m_nameEdit);

    m_soundEdit = new QLineEdit(this);
    connect(m_soundEdit, &QLineEdit::textChanged, this, &ExplosionEditorWidget::onParameterChanged);
    layout->addRow("Sound File:", m_soundEdit);

    m_debrisEdit = new QLineEdit(this);
    connect(m_debrisEdit, &QLineEdit::textChanged, this, &ExplosionEditorWidget::onParameterChanged);
    layout->addRow("Debris Explosion:", m_debrisEdit);

    return group;
}

QGroupBox* ExplosionEditorWidget::createSettingsGroup() {
    QGroupBox* group = new QGroupBox("Settings", this);
    QGridLayout* layout = new QGridLayout(group);

    // Scale
    layout->addWidget(new QLabel("Scale X:", this), 0, 0);
    m_scaleXSpin = new QDoubleSpinBox(this);
    m_scaleXSpin->setRange(0.01, 100);
    m_scaleXSpin->setDecimals(2);
    connect(m_scaleXSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &ExplosionEditorWidget::onParameterChanged);
    layout->addWidget(m_scaleXSpin, 0, 1);

    layout->addWidget(new QLabel("Y:", this), 0, 2);
    m_scaleYSpin = new QDoubleSpinBox(this);
    m_scaleYSpin->setRange(0.01, 100);
    m_scaleYSpin->setDecimals(2);
    connect(m_scaleYSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &ExplosionEditorWidget::onParameterChanged);
    layout->addWidget(m_scaleYSpin, 0, 3);

    layout->addWidget(new QLabel("Z:", this), 0, 4);
    m_scaleZSpin = new QDoubleSpinBox(this);
    m_scaleZSpin->setRange(0.01, 100);
    m_scaleZSpin->setDecimals(2);
    connect(m_scaleZSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &ExplosionEditorWidget::onParameterChanged);
    layout->addWidget(m_scaleZSpin, 0, 5);

    // Shake
    layout->addWidget(new QLabel("Camera Shake:", this), 1, 0);
    m_shakeSpin = new QDoubleSpinBox(this);
    m_shakeSpin->setRange(0, 10);
    m_shakeSpin->setDecimals(2);
    connect(m_shakeSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &ExplosionEditorWidget::onParameterChanged);
    layout->addWidget(m_shakeSpin, 1, 1);

    // Poly blow speed
    layout->addWidget(new QLabel("Poly Blow Speed:", this), 2, 0);
    m_polyBlowSpeedSpin = new QDoubleSpinBox(this);
    m_polyBlowSpeedSpin->setRange(-100, 100);
    m_polyBlowSpeedSpin->setDecimals(2);
    connect(m_polyBlowSpeedSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &ExplosionEditorWidget::onParameterChanged);
    layout->addWidget(m_polyBlowSpeedSpin, 2, 1);

    // Pressure Wave section
    layout->addWidget(new QLabel("Pressure Wave:", this), 3, 0);
    m_pressureWaveCheck = new QCheckBox("Enable", this);
    connect(m_pressureWaveCheck, &QCheckBox::toggled, this, &ExplosionEditorWidget::onParameterChanged);
    layout->addWidget(m_pressureWaveCheck, 3, 1);

    layout->addWidget(new QLabel("Force:", this), 4, 0);
    m_pressureWaveForceSpin = new QDoubleSpinBox(this);
    m_pressureWaveForceSpin->setRange(0, 1000);
    m_pressureWaveForceSpin->setDecimals(1);
    connect(m_pressureWaveForceSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &ExplosionEditorWidget::onParameterChanged);
    layout->addWidget(m_pressureWaveForceSpin, 4, 1);

    layout->addWidget(new QLabel("Inner Radius:", this), 4, 2);
    m_pressureWaveInnerSpin = new QDoubleSpinBox(this);
    m_pressureWaveInnerSpin->setRange(0, 1000);
    m_pressureWaveInnerSpin->setDecimals(1);
    connect(m_pressureWaveInnerSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &ExplosionEditorWidget::onParameterChanged);
    layout->addWidget(m_pressureWaveInnerSpin, 4, 3);

    layout->addWidget(new QLabel("Outer Radius:", this), 4, 4);
    m_pressureWaveOuterSpin = new QDoubleSpinBox(this);
    m_pressureWaveOuterSpin->setRange(0, 1000);
    m_pressureWaveOuterSpin->setDecimals(1);
    connect(m_pressureWaveOuterSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &ExplosionEditorWidget::onParameterChanged);
    layout->addWidget(m_pressureWaveOuterSpin, 4, 5);

    return group;
}

QGroupBox* ExplosionEditorWidget::createObjectsGroup() {
    QGroupBox* group = new QGroupBox("Explosion Objects", this);
    QVBoxLayout* layout = new QVBoxLayout(group);

    m_objectsTable = new QTableWidget(this);
    m_objectsTable->setColumnCount(8);
    m_objectsTable->setHorizontalHeaderLabels({"Material", "Scale", "Delay", "Duration", "Variation", "Additive", "Loop", "Billboard"});
    m_objectsTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_objectsTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_objectsTable->setMinimumHeight(150);
    connect(m_objectsTable, &QTableWidget::cellChanged, this, &ExplosionEditorWidget::onObjectsTableChanged);
    layout->addWidget(m_objectsTable);

    QHBoxLayout* btnLayout = new QHBoxLayout();
    m_addObjectBtn = new QPushButton("Add Object", this);
    connect(m_addObjectBtn, &QPushButton::clicked, this, &ExplosionEditorWidget::onAddObject);
    btnLayout->addWidget(m_addObjectBtn);

    m_removeObjectBtn = new QPushButton("Remove Object", this);
    connect(m_removeObjectBtn, &QPushButton::clicked, this, &ExplosionEditorWidget::onRemoveObject);
    btnLayout->addWidget(m_removeObjectBtn);

    btnLayout->addStretch();
    layout->addLayout(btnLayout);

    return group;
}

QGroupBox* ExplosionEditorWidget::createThrowGroup() {
    QGroupBox* group = new QGroupBox("Throw Objects (Debris)", this);
    QVBoxLayout* layout = new QVBoxLayout(group);

    m_throwTable = new QTableWidget(this);
    m_throwTable->setColumnCount(7);
    m_throwTable->setHorizontalHeaderLabels({"Material", "Scale", "Delay", "Duration", "Variation", "Speed", "Count"});
    m_throwTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_throwTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_throwTable->setMinimumHeight(150);
    connect(m_throwTable, &QTableWidget::cellChanged, this, &ExplosionEditorWidget::onThrowTableChanged);
    layout->addWidget(m_throwTable);

    QHBoxLayout* btnLayout = new QHBoxLayout();
    m_addThrowBtn = new QPushButton("Add Throw Object", this);
    connect(m_addThrowBtn, &QPushButton::clicked, this, &ExplosionEditorWidget::onAddThrowObject);
    btnLayout->addWidget(m_addThrowBtn);

    m_removeThrowBtn = new QPushButton("Remove Throw Object", this);
    connect(m_removeThrowBtn, &QPushButton::clicked, this, &ExplosionEditorWidget::onRemoveThrowObject);
    btnLayout->addWidget(m_removeThrowBtn);

    btnLayout->addStretch();
    layout->addLayout(btnLayout);

    return group;
}

void ExplosionEditorWidget::setExplosion(Effects::Explosion* explosion) {
    m_explosion = explosion;
    updateFromExplosion();
}

void ExplosionEditorWidget::updateFromExplosion() {
    if (!m_explosion) {
        setEnabled(false);
        return;
    }

    setEnabled(true);
    m_updating = true;

    m_nameEdit->setText(m_explosion->name);
    m_soundEdit->setText(m_explosion->soundFile);
    m_debrisEdit->setText(m_explosion->debrisExplosion);

    m_scaleXSpin->setValue(m_explosion->scaleX);
    m_scaleYSpin->setValue(m_explosion->scaleY);
    m_scaleZSpin->setValue(m_explosion->scaleZ);
    m_shakeSpin->setValue(m_explosion->shake);
    m_polyBlowSpeedSpin->setValue(m_explosion->polyBlowSpeed);

    // Pressure Wave
    m_pressureWaveCheck->setChecked(m_explosion->hasPressureWave);
    m_pressureWaveForceSpin->setValue(m_explosion->pressureWaveForce);
    m_pressureWaveInnerSpin->setValue(m_explosion->pressureWaveInnerRadius);
    m_pressureWaveOuterSpin->setValue(m_explosion->pressureWaveOuterRadius);

    updateObjectsTable();
    updateThrowTable();

    m_updating = false;
}

void ExplosionEditorWidget::updateObjectsTable() {
    if (!m_explosion) return;

    m_updating = true;
    m_objectsTable->setRowCount(m_explosion->objects.size());

    for (int i = 0; i < m_explosion->objects.size(); i++) {
        const Effects::ExplosionObject& obj = m_explosion->objects[i];

        m_objectsTable->setItem(i, 0, new QTableWidgetItem(obj.materialName));
        m_objectsTable->setItem(i, 1, new QTableWidgetItem(QString::number(obj.scale, 'f', 1)));
        m_objectsTable->setItem(i, 2, new QTableWidgetItem(QString::number(obj.delay, 'f', 1)));
        m_objectsTable->setItem(i, 3, new QTableWidgetItem(QString::number(obj.duration, 'f', 1)));
        m_objectsTable->setItem(i, 4, new QTableWidgetItem(QString::number(obj.variation, 'f', 1)));
        m_objectsTable->setItem(i, 5, new QTableWidgetItem(QString::number(obj.additive)));
        m_objectsTable->setItem(i, 6, new QTableWidgetItem(QString::number(obj.loop)));
        m_objectsTable->setItem(i, 7, new QTableWidgetItem(QString::number(obj.billboard)));
    }

    m_updating = false;
}

void ExplosionEditorWidget::updateThrowTable() {
    if (!m_explosion) return;

    m_updating = true;
    m_throwTable->setRowCount(m_explosion->throwObjects.size());

    for (int i = 0; i < m_explosion->throwObjects.size(); i++) {
        const Effects::ThrowObject& obj = m_explosion->throwObjects[i];

        m_throwTable->setItem(i, 0, new QTableWidgetItem(obj.materialName));
        m_throwTable->setItem(i, 1, new QTableWidgetItem(QString::number(obj.scale, 'f', 1)));
        m_throwTable->setItem(i, 2, new QTableWidgetItem(QString::number(obj.delay, 'f', 1)));
        m_throwTable->setItem(i, 3, new QTableWidgetItem(QString::number(obj.duration, 'f', 1)));
        m_throwTable->setItem(i, 4, new QTableWidgetItem(QString::number(obj.variation, 'f', 1)));
        m_throwTable->setItem(i, 5, new QTableWidgetItem(QString::number(obj.speed, 'f', 1)));
        m_throwTable->setItem(i, 6, new QTableWidgetItem(QString::number(obj.count, 'f', 1)));
    }

    m_updating = false;
}

void ExplosionEditorWidget::onParameterChanged() {
    if (m_updating || !m_explosion) return;

    m_explosion->soundFile = m_soundEdit->text();
    m_explosion->debrisExplosion = m_debrisEdit->text();

    m_explosion->scaleX = m_scaleXSpin->value();
    m_explosion->scaleY = m_scaleYSpin->value();
    m_explosion->scaleZ = m_scaleZSpin->value();
    m_explosion->shake = m_shakeSpin->value();
    m_explosion->polyBlowSpeed = m_polyBlowSpeedSpin->value();

    // Pressure Wave
    m_explosion->hasPressureWave = m_pressureWaveCheck->isChecked();
    m_explosion->pressureWaveForce = m_pressureWaveForceSpin->value();
    m_explosion->pressureWaveInnerRadius = m_pressureWaveInnerSpin->value();
    m_explosion->pressureWaveOuterRadius = m_pressureWaveOuterSpin->value();

    emit explosionModified();
}

void ExplosionEditorWidget::onObjectsTableChanged() {
    if (m_updating || !m_explosion) return;

    for (int i = 0; i < m_objectsTable->rowCount() && i < m_explosion->objects.size(); i++) {
        Effects::ExplosionObject& obj = m_explosion->objects[i];

        if (m_objectsTable->item(i, 0)) obj.materialName = m_objectsTable->item(i, 0)->text();
        if (m_objectsTable->item(i, 1)) obj.scale = m_objectsTable->item(i, 1)->text().toFloat();
        if (m_objectsTable->item(i, 2)) obj.delay = m_objectsTable->item(i, 2)->text().toFloat();
        if (m_objectsTable->item(i, 3)) obj.duration = m_objectsTable->item(i, 3)->text().toFloat();
        if (m_objectsTable->item(i, 4)) obj.variation = m_objectsTable->item(i, 4)->text().toFloat();
        if (m_objectsTable->item(i, 5)) obj.additive = m_objectsTable->item(i, 5)->text().toInt();
        if (m_objectsTable->item(i, 6)) obj.loop = m_objectsTable->item(i, 6)->text().toInt();
        if (m_objectsTable->item(i, 7)) obj.billboard = m_objectsTable->item(i, 7)->text().toInt();
    }

    emit explosionModified();
}

void ExplosionEditorWidget::onThrowTableChanged() {
    if (m_updating || !m_explosion) return;

    for (int i = 0; i < m_throwTable->rowCount() && i < m_explosion->throwObjects.size(); i++) {
        Effects::ThrowObject& obj = m_explosion->throwObjects[i];

        if (m_throwTable->item(i, 0)) obj.materialName = m_throwTable->item(i, 0)->text();
        if (m_throwTable->item(i, 1)) obj.scale = m_throwTable->item(i, 1)->text().toFloat();
        if (m_throwTable->item(i, 2)) obj.delay = m_throwTable->item(i, 2)->text().toFloat();
        if (m_throwTable->item(i, 3)) obj.duration = m_throwTable->item(i, 3)->text().toFloat();
        if (m_throwTable->item(i, 4)) obj.variation = m_throwTable->item(i, 4)->text().toFloat();
        if (m_throwTable->item(i, 5)) obj.speed = m_throwTable->item(i, 5)->text().toFloat();
        if (m_throwTable->item(i, 6)) obj.count = m_throwTable->item(i, 6)->text().toFloat();
    }

    emit explosionModified();
}

void ExplosionEditorWidget::onAddObject() {
    if (!m_explosion) return;

    Effects::ExplosionObject obj;
    obj.materialName = "Effect_explosion_1_RE304_256";
    m_explosion->objects.append(obj);
    updateObjectsTable();
    emit explosionModified();
}

void ExplosionEditorWidget::onRemoveObject() {
    if (!m_explosion) return;

    int row = m_objectsTable->currentRow();
    if (row >= 0 && row < m_explosion->objects.size()) {
        m_explosion->objects.remove(row);
        updateObjectsTable();
        emit explosionModified();
    }
}

void ExplosionEditorWidget::onAddThrowObject() {
    if (!m_explosion) return;

    Effects::ThrowObject obj;
    obj.materialName = "Effect_Skrot_Human_Standard1";
    m_explosion->throwObjects.append(obj);
    updateThrowTable();
    emit explosionModified();
}

void ExplosionEditorWidget::onRemoveThrowObject() {
    if (!m_explosion) return;

    int row = m_throwTable->currentRow();
    if (row >= 0 && row < m_explosion->throwObjects.size()) {
        m_explosion->throwObjects.remove(row);
        updateThrowTable();
        emit explosionModified();
    }
}
