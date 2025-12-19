#ifndef SETTINGSMANAGER_H
#define SETTINGSMANAGER_H

#include <QSettings>
#include <QString>
#include <QStringList>

class SettingsManager
{
public:
    static SettingsManager& instance();

    // Export settings
    bool exportOBJ() const;
    void setExportOBJ(bool value);

    bool exportPNG() const;
    void setExportPNG(bool value);

    bool exportJSON() const;
    void setExportJSON(bool value);

    bool exportMTL() const;
    void setExportMTL(bool value);

    bool exportBlenderScript() const;
    void setExportBlenderScript(bool value);

    enum TextureFormat
    {
        PNG = 0,
        JPEG = 1
    };

    TextureFormat textureFormat() const;
    void setTextureFormat(TextureFormat format);

    enum TextureScale
    {
        Scale100 = 100,
        Scale50 = 50,
        Scale25 = 25
    };

    TextureScale textureScale() const;
    void setTextureScale(TextureScale scale);

    // Recent files
    QStringList recentFiles() const;
    void addRecentFile(const QString& filepath);
    void clearRecentFiles();

private:
    SettingsManager();
    ~SettingsManager();

    SettingsManager(const SettingsManager&) = delete;
    SettingsManager& operator=(const SettingsManager&) = delete;

    QSettings m_settings;
    static const int MAX_RECENT_FILES = 10;
};

#endif // SETTINGSMANAGER_H
