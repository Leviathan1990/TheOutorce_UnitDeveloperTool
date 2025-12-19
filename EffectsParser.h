#ifndef EFFECTSPARSER_H
#define EFFECTSPARSER_H

#include "EffectsStructs.h"
#include <QString>
#include <QStringList>

namespace Effects {

class EffectsParser {
public:
    EffectsParser();

    // Parse individual files
    bool parseEmittersCfg(const QString& filepath, EffectsProject& project);
    bool parseEmitterMaterialsCfg(const QString& filepath, EffectsProject& project);
    bool parseExplosionsCfg(const QString& filepath, EffectsProject& project);
    bool parseExplosionsSettingsCfg(const QString& filepath, EffectsProject& project);
    bool parseColorsCfg(const QString& filepath, EffectsProject& project);

    // Parse all from a directory
    bool parseDirectory(const QString& dirPath, EffectsProject& project);

    // Get last error
    QString lastError() const { return m_lastError; }

private:
    QString m_lastError;

    // Helper functions
    QStringList extractParameters(const QString& line);
    QString extractStringParam(const QString& param);
    float extractFloat(const QString& param);
    int extractInt(const QString& param);

    // Line parsers
    bool parseEmitterCreate(const QString& line, Emitter& emitter);
    bool parseGradientPoint(const QString& line, GradientPoint& point);
    bool parseEmitterSetName(const QString& line, QString& name);
    bool parseEmitterSetMaterial(const QString& line, int& id, QString& material);

    bool parseExplosionCreate(const QString& line, QString& name);
    bool parseExplosionAddObject(const QString& line, QString& exploName, ExplosionObject& obj);
    bool parseExplosionAddThrowObject(const QString& line, QString& exploName, ThrowObject& obj);
    bool parseExplosionSetScale(const QString& line, QString& exploName, float& x, float& y, float& z);
    bool parseExplosionSetShake(const QString& line, QString& exploName, float& value);
    bool parseExplosionSetSound(const QString& line, QString& exploName, QString& sound);
    bool parseExplosionPolyBlowSpeed(const QString& line, QString& exploName, float& speed);
    bool parseExplosionDebrisExplosion(const QString& line, QString& exploName, QString& debris);
    bool parseExplosionLock(const QString& line, QString& exploName);
    bool parseExplosionSetPressureWave(const QString& line, QString& exploName,
                                       float& force, float& innerRadius, float& outerRadius);

    bool parseColorSetting(const QString& line, SceneColors& colors);
};

} // namespace Effects

#endif // EFFECTSPARSER_H
