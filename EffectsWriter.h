#ifndef EFFECTSWRITER_H
#define EFFECTSWRITER_H

#include "EffectsStructs.h"
#include <QString>

namespace Effects {

class EffectsWriter {
public:
    EffectsWriter();

    // Write individual files
    bool writeEmittersCfg(const QString& filepath, const EffectsProject& project);
    bool writeEmitterMaterialsCfg(const QString& filepath, const EffectsProject& project);
    bool writeExplosionsCfg(const QString& filepath, const EffectsProject& project);
    bool writeExplosionsSettingsCfg(const QString& filepath, const EffectsProject& project);
    bool writeColorsCfg(const QString& filepath, const EffectsProject& project);

    // Write all to original paths
    bool writeAll(const EffectsProject& project);

    // Get last error
    QString lastError() const { return m_lastError; }

private:
    QString m_lastError;

    // Formatting helpers
    QString formatFloat(float value, int decimals = 6) const;
    QString formatEmitterCreate(const Emitter& emitter) const;
    QString formatGradientPoint(const GradientPoint& point) const;
    QString formatExplosionObject(const QString& exploName, const ExplosionObject& obj) const;
    QString formatThrowObject(const QString& exploName, const ThrowObject& obj) const;
};

} // namespace Effects

#endif // EFFECTSWRITER_H
