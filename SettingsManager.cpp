#include "SettingsManager.h"

SettingsManager::SettingsManager() : m_settings(QSettings::IniFormat, QSettings::UserScope,"OutforceModding", "OutforceAssetBrowser")
{
    if (!m_settings.contains("Export/exportOBJ"))
    {
        m_settings.setValue("Export/exportOBJ", true);
    }

    if (!m_settings.contains("Export/exportPNG"))
    {
        m_settings.setValue("Export/exportPNG", true);
    }

    if (!m_settings.contains("Export/exportJSON"))
    {
        m_settings.setValue("Export/exportJSON", true);
    }

    if (!m_settings.contains("Export/exportMTL"))
    {
        m_settings.setValue("Export/exportMTL", true);
    }

    if (!m_settings.contains("Export/exportBlenderScript"))
    {
        m_settings.setValue("Export/exportBlenderScript", true);
    }

    if (!m_settings.contains("Export/textureFormat"))
    {
        m_settings.setValue("Export/textureFormat", PNG);
    }

    if (!m_settings.contains("Export/textureScale"))
    {
        m_settings.setValue("Export/textureScale", Scale100);
    }
    m_settings.sync();
}

SettingsManager::~SettingsManager()
{
    m_settings.sync();
}

SettingsManager& SettingsManager::instance()
{
    static SettingsManager instance;
    return instance;
}

bool SettingsManager::exportOBJ() const
{
    return m_settings.value("Export/exportOBJ", true).toBool();
}

void SettingsManager::setExportOBJ(bool value)
{
    m_settings.setValue("Export/exportOBJ", value);
    m_settings.sync();
}

bool SettingsManager::exportPNG() const
{
    return m_settings.value("Export/exportPNG", true).toBool();
}

void SettingsManager::setExportPNG(bool value)
{
    m_settings.setValue("Export/exportPNG", value);
    m_settings.sync();
}

bool SettingsManager::exportJSON() const
{
    return m_settings.value("Export/exportJSON", true).toBool();
}

void SettingsManager::setExportJSON(bool value)
{
    m_settings.setValue("Export/exportJSON", value);
    m_settings.sync();
}

bool SettingsManager::exportMTL() const
{
    return m_settings.value("Export/exportMTL", true).toBool();
}

void SettingsManager::setExportMTL(bool value)
{
    m_settings.setValue("Export/exportMTL", value);
    m_settings.sync();
}

bool SettingsManager::exportBlenderScript() const
{
    return m_settings.value("Export/exportBlenderScript", true).toBool();
}

void SettingsManager::setExportBlenderScript(bool value)
{
    m_settings.setValue("Export/exportBlenderScript", value);
    m_settings.sync();
}

SettingsManager::TextureFormat SettingsManager::textureFormat() const
{
    return static_cast<TextureFormat>(
        m_settings.value("Export/textureFormat", PNG).toInt());
}

void SettingsManager::setTextureFormat(TextureFormat format)
{
    m_settings.setValue("Export/textureFormat", static_cast<int>(format));
    m_settings.sync();
}

SettingsManager::TextureScale SettingsManager::textureScale() const
{
    return static_cast<TextureScale>(m_settings.value("Export/textureScale", Scale100).toInt());
}

void SettingsManager::setTextureScale(TextureScale scale)
{
    m_settings.setValue("Export/textureScale", static_cast<int>(scale));
    m_settings.sync();
}

QStringList SettingsManager::recentFiles() const
{
    return m_settings.value("Recent/files").toStringList();
}

void SettingsManager::addRecentFile(const QString& filepath)
{
    QStringList files = recentFiles();
    files.removeAll(filepath);
    files.prepend(filepath);

    while (files.size() > MAX_RECENT_FILES)
    {
        files.removeLast();
    }

    m_settings.setValue("Recent/files", files);
    m_settings.sync();
}

void SettingsManager::clearRecentFiles()
{
    m_settings.setValue("Recent/files", QStringList());
    m_settings.sync();
}
