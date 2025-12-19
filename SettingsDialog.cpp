#include "SettingsDialog.h"
#include "SettingsManager.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QPushButton>
#include <QLabel>
#include <QDialogButtonBox>

SettingsDialog::SettingsDialog(QWidget *parent) : QDialog(parent)
{
    setWindowTitle("Preferences");
    setMinimumWidth(500);
    setMinimumHeight(400);

    setupUI();
    loadSettings();
}

void SettingsDialog::setupUI()
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    QTabWidget* tabs = new QTabWidget(this);

    // Export tab
    QWidget* exportTab = new QWidget(this);
    QVBoxLayout* exportLayout = new QVBoxLayout(exportTab);

    QGroupBox* exportGroup = new QGroupBox("Export Options", this);
    QVBoxLayout* exportGroupLayout = new QVBoxLayout(exportGroup);

    m_exportOBJCheck = new QCheckBox("Export OBJ meshes", this);
    exportGroupLayout->addWidget(m_exportOBJCheck);

    m_exportPNGCheck = new QCheckBox("Export PNG textures", this);
    exportGroupLayout->addWidget(m_exportPNGCheck);

    m_exportJSONCheck = new QCheckBox("Export JSON metadata", this);
    exportGroupLayout->addWidget(m_exportJSONCheck);

    m_exportMTLCheck = new QCheckBox("Export MTL files", this);
    exportGroupLayout->addWidget(m_exportMTLCheck);

    m_exportBlenderCheck = new QCheckBox("Generate Blender import script", this);
    exportGroupLayout->addWidget(m_exportBlenderCheck);

    exportLayout->addWidget(exportGroup);

    QGroupBox* formatGroup = new QGroupBox("Texture Format", this);
    QVBoxLayout* formatLayout = new QVBoxLayout(formatGroup);

    m_formatGroup = new QButtonGroup(this);

    m_formatPNGRadio = new QRadioButton("PNG (lossless)", this);
    m_formatGroup->addButton(m_formatPNGRadio, SettingsManager::PNG);
    formatLayout->addWidget(m_formatPNGRadio);

    m_formatJPEGRadio = new QRadioButton("JPEG (compressed)", this);
    m_formatGroup->addButton(m_formatJPEGRadio, SettingsManager::JPEG);
    formatLayout->addWidget(m_formatJPEGRadio);

    exportLayout->addWidget(formatGroup);

    QGroupBox* scaleGroup = new QGroupBox("Texture Scale", this);
    QVBoxLayout* scaleLayout = new QVBoxLayout(scaleGroup);

    m_scaleGroup = new QButtonGroup(this);

    m_scale100Radio = new QRadioButton("100% (original)", this);
    m_scaleGroup->addButton(m_scale100Radio, SettingsManager::Scale100);
    scaleLayout->addWidget(m_scale100Radio);

    m_scale50Radio = new QRadioButton("50%", this);
    m_scaleGroup->addButton(m_scale50Radio, SettingsManager::Scale50);
    scaleLayout->addWidget(m_scale50Radio);

    m_scale25Radio = new QRadioButton("25%", this);
    m_scaleGroup->addButton(m_scale25Radio, SettingsManager::Scale25);
    scaleLayout->addWidget(m_scale25Radio);

    exportLayout->addWidget(scaleGroup);
    exportLayout->addStretch();

    tabs->addTab(exportTab, "Export");

    mainLayout->addWidget(tabs);

    QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel | QDialogButtonBox::Apply,this);

    connect(buttonBox, &QDialogButtonBox::accepted, this, &SettingsDialog::onAccept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
    connect(buttonBox->button(QDialogButtonBox::Apply), &QPushButton::clicked, this, &SettingsDialog::onApply);

    mainLayout->addWidget(buttonBox);
}

void SettingsDialog::loadSettings()
{
    SettingsManager& settings = SettingsManager::instance();

    m_exportOBJCheck->setChecked(settings.exportOBJ());
    m_exportPNGCheck->setChecked(settings.exportPNG());
    m_exportJSONCheck->setChecked(settings.exportJSON());
    m_exportMTLCheck->setChecked(settings.exportMTL());
    m_exportBlenderCheck->setChecked(settings.exportBlenderScript());

    if (settings.textureFormat() == SettingsManager::PNG)
    {
        m_formatPNGRadio->setChecked(true);
    }

    else
    {
        m_formatJPEGRadio->setChecked(true);
    }

    switch (settings.textureScale())
    {
    case SettingsManager::Scale100:
        m_scale100Radio->setChecked(true);
        break;
    case SettingsManager::Scale50:
        m_scale50Radio->setChecked(true);
        break;
    case SettingsManager::Scale25:
        m_scale25Radio->setChecked(true);
        break;
    }
}

void SettingsDialog::saveSettings()
{
    SettingsManager& settings = SettingsManager::instance();

    settings.setExportOBJ(m_exportOBJCheck->isChecked());
    settings.setExportPNG(m_exportPNGCheck->isChecked());
    settings.setExportJSON(m_exportJSONCheck->isChecked());
    settings.setExportMTL(m_exportMTLCheck->isChecked());
    settings.setExportBlenderScript(m_exportBlenderCheck->isChecked());

    settings.setTextureFormat(static_cast<SettingsManager::TextureFormat>(m_formatGroup->checkedId()));

    settings.setTextureScale(static_cast<SettingsManager::TextureScale>(m_scaleGroup->checkedId()));
}

void SettingsDialog::onAccept()
{
    saveSettings();
    accept();
}

void SettingsDialog::onApply()
{
    saveSettings();
}
