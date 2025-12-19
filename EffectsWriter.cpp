#include "EffectsWriter.h"
#include <QFile>
#include <QTextStream>
#include <QFileInfo>

namespace Effects {

EffectsWriter::EffectsWriter() {}

QString EffectsWriter::formatFloat(float value, int decimals) const {
    return QString::number(value, 'f', decimals);
}

QString EffectsWriter::formatEmitterCreate(const Emitter& emitter) const {
    // Parameter order matches cfg file (IDA offsets in comments)
    return QString("Emitter_Create (%1, %2, %3, %4, %5, %6, %7, %8, %9, %10, %11, %12, %13, %14, %15, %16, %17, %18, %19, %20, %21, %22);")
        .arg(emitter.id)                              // +32
        .arg(formatFloat(emitter.birthRate))          // +48
        .arg(emitter.reuseParticles)                  // +96
        .arg(emitter.deathWish)                       // +56
        .arg(formatFloat(emitter.speed))              // +88
        .arg(formatFloat(emitter.lifeTime))           // +80
        .arg(formatFloat(emitter.variableLifeTime))   // +84
        .arg(emitter.numParticles)                    // +44
        .arg(formatFloat(emitter.size))               // +60
        .arg(formatFloat(emitter.spread))             // +64
        .arg(formatFloat(emitter.speedModifier))      // +68
        .arg(formatFloat(emitter.spreadAround))       // +72
        .arg(formatFloat(emitter.spreadAroundStart))  // +76
        .arg(formatFloat(emitter.acceleration))       // +92
        .arg(formatFloat(emitter.posVariationX))      // +100
        .arg(formatFloat(emitter.posVariationY))      // +104
        .arg(formatFloat(emitter.posVariationZ))      // +108
        .arg(formatFloat(emitter.shrink))             // +112
        .arg(formatFloat(emitter.sizeVariation))      // +116
        .arg(formatFloat(emitter.speedVariation))     // +120
        .arg(formatFloat(emitter.speedVariationCoeff))// +124
        .arg(formatFloat(emitter.spreadCoefficient)); // +128
}

QString EffectsWriter::formatGradientPoint(const GradientPoint& point) const {
    return QString("Emitter_AddGradientPoint (%1, %2, %3, %4, %5);")
    .arg(point.position)
        .arg(formatFloat(point.r))
        .arg(formatFloat(point.g))
        .arg(formatFloat(point.b))
        .arg(formatFloat(point.alpha));
}

QString EffectsWriter::formatExplosionObject(const QString& exploName, const ExplosionObject& obj) const {
    return QString("Effect_Explo_AddObject (\"%1\", \"%2\", %3, %4, %5, %6, %7, %8, %9);")
    .arg(exploName)
        .arg(obj.materialName)
        .arg(formatFloat(obj.scale, 1))
        .arg(formatFloat(obj.delay, 1))
        .arg(formatFloat(obj.duration, 1))
        .arg(formatFloat(obj.variation, 1))
        .arg(obj.additive)
        .arg(obj.loop)
        .arg(obj.billboard);
}

QString EffectsWriter::formatThrowObject(const QString& exploName, const ThrowObject& obj) const {
    return QString("Effect_Explo_AddThrowObject (\"%1\", \"%2\", %3, %4, %5, %6, %7, %8);")
    .arg(exploName)
        .arg(obj.materialName)
        .arg(formatFloat(obj.scale, 1))
        .arg(formatFloat(obj.delay))
        .arg(formatFloat(obj.duration, 1))
        .arg(formatFloat(obj.variation, 1))
        .arg(formatFloat(obj.speed, 1))
        .arg(formatFloat(obj.count, 1));
}

// ============================================================================
// EMITTERS.CFG WRITER
// ============================================================================

bool EffectsWriter::writeEmittersCfg(const QString& filepath, const EffectsProject& project) {
    QFile file(filepath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        m_lastError = QString("Cannot write file: %1").arg(filepath);
        return false;
    }

    QTextStream out(&file);

    // Get sorted list of emitter IDs
    QList<int> ids = project.emitters.keys();
    std::sort(ids.begin(), ids.end());

    for (int id : ids) {
        const Emitter& emitter = project.emitters[id];

        // Comment with name
        out << "// " << emitter.name << "\n";

        // Emitter_Create
        out << formatEmitterCreate(emitter) << "\n";

        // Gradient points
        for (const GradientPoint& point : emitter.gradientPoints) {
            out << formatGradientPoint(point) << "\n";
        }

        // Set name
        out << "Emitter_SetName (\"" << emitter.name << "\");\n";
        out << "\n";
    }

    file.close();
    return true;
}

// ============================================================================
// EMITTERMATERIALS.CFG WRITER
// ============================================================================

bool EffectsWriter::writeEmitterMaterialsCfg(const QString& filepath, const EffectsProject& project) {
    QFile file(filepath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        m_lastError = QString("Cannot write file: %1").arg(filepath);
        return false;
    }

    QTextStream out(&file);

    // Get sorted list of emitter IDs
    QList<int> ids = project.emitters.keys();
    std::sort(ids.begin(), ids.end());

    for (int id : ids) {
        const Emitter& emitter = project.emitters[id];

        out << QString("Emitter_SetMaterial (%1, \"%2\");   // %3\n")
                   .arg(id)
                   .arg(emitter.material)
                   .arg(emitter.name);
    }

    file.close();
    return true;
}

// ============================================================================
// EXPLOSIONS.CFG WRITER
// ============================================================================

bool EffectsWriter::writeExplosionsCfg(const QString& filepath, const EffectsProject& project) {
    QFile file(filepath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        m_lastError = QString("Cannot write file: %1").arg(filepath);
        return false;
    }

    QTextStream out(&file);

    out << "// Define a Explosion\n\n";

    // Group explosions by category (Human, Gobin, Crion, Xenon, etc.)
    QStringList humanExplos, gobinExplos, crionExplos, xenonExplos, otherExplos;

    for (const QString& name : project.explosions.keys()) {
        if (name.contains("Human") || name.contains("UCM") || name.contains("Centa") ||
            name.contains("Shield") || name.contains("MetalWork") || name.contains("Plasma") ||
            name.contains("Warp") || name.contains("Antimatter")) {
            humanExplos.append(name);
        } else if (name.contains("Gob") || name.contains("Gib") || name.contains("Into") ||
                   name.contains("Wontar") || name.contains("Destruct") || name.contains("Build")) {
            gobinExplos.append(name);
        } else if (name.contains("Crion") || name.contains("Cri_")) {
            crionExplos.append(name);
        } else if (name.contains("Xeno") || name.contains("Xenon")) {
            xenonExplos.append(name);
        } else {
            otherExplos.append(name);
        }
    }

    auto writeCategory = [&out](const QString& title, const QStringList& names) {
        if (!names.isEmpty()) {
            out << "\n// " << title << "\n\n";
            for (const QString& name : names) {
                out << "Effect_Explo_Create (\"" << name << "\");\n\n";
            }
        }
    };

    writeCategory("Human Explosions FX", humanExplos);
    writeCategory("Gobin Explosions FX", gobinExplos);
    writeCategory("Crion Explosions FX", crionExplos);
    writeCategory("XENON SPECIAL Explosions", xenonExplos);
    writeCategory("Other Explosions", otherExplos);

    file.close();
    return true;
}

// ============================================================================
// EXPLOSIONSSETTINGS.CFG WRITER
// ============================================================================

bool EffectsWriter::writeExplosionsSettingsCfg(const QString& filepath, const EffectsProject& project) {
    QFile file(filepath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        m_lastError = QString("Cannot write file: %1").arg(filepath);
        return false;
    }

    QTextStream out(&file);

    out << "// Explo_SetShake(\"Explo_type\", Value);\n";
    out << "// Generated by Outforce Effects Editor\n\n";

    for (auto it = project.explosions.begin(); it != project.explosions.end(); ++it) {
        const QString& name = it.key();
        const Explosion& explo = it.value();

        out << "// " << name.toUpper() << "\n\n";

        // Add objects
        for (const ExplosionObject& obj : explo.objects) {
            out << formatExplosionObject(name, obj) << "\n";
        }

        if (!explo.objects.isEmpty()) {
            out << "\n";
        }

        // Add throw objects
        if (!explo.throwObjects.isEmpty()) {
            out << "// Add a throw object to the explosion\n";
            for (const ThrowObject& obj : explo.throwObjects) {
                out << formatThrowObject(name, obj) << "\n";
            }
            out << "\n";
        }

        // PolyBlowSpeed
        if (explo.polyBlowSpeed != 0.0f) {
            out << QString("Effect_Explo_PolyBlowSpeed (\"%1\", %2);\n")
            .arg(name)
                .arg(formatFloat(explo.polyBlowSpeed, 1));
        }

        // DebrisExplosion
        if (!explo.debrisExplosion.isEmpty()) {
            out << QString("Effect_Explo_DebrisExplosion (\"%1\", \"%2\");\n")
            .arg(name)
                .arg(explo.debrisExplosion);
        }

        // Shake
        if (explo.shake != 0.0f) {
            out << QString("Effect_Explo_SetShake(\"%1\", %2);\n")
            .arg(name)
                .arg(formatFloat(explo.shake, 1));
        }

        // Scale
        if (explo.scaleX != 1.0f || explo.scaleY != 1.0f || explo.scaleZ != 1.0f) {
            out << QString("Effect_Explo_SetScale (\"%1\", %2, %3, %4);\n")
            .arg(name)
                .arg(formatFloat(explo.scaleX, 1))
                .arg(formatFloat(explo.scaleY, 1))
                .arg(formatFloat(explo.scaleZ, 1));
        }

        // Sound
        if (!explo.soundFile.isEmpty()) {
            out << QString("Effect_Explo_SetSound (\"%1\", \"%2\");\n")
            .arg(name)
                .arg(explo.soundFile);
        }

        // Pressure Wave
        if (explo.hasPressureWave) {
            out << QString("Effect_Explo_SetPressureWave (\"%1\", %2, %3, %4);\n")
            .arg(name)
                .arg(formatFloat(explo.pressureWaveForce, 1))
                .arg(formatFloat(explo.pressureWaveInnerRadius, 1))
                .arg(formatFloat(explo.pressureWaveOuterRadius, 1));
        }

        // Lock
        out << QString("Effect_Explo_Lock (\"%1\");\n").arg(name);
        out << "\n// " << QString("-").repeated(100) << "\n\n";
    }

    file.close();
    return true;
}

// ============================================================================
// COLORS.CFG WRITER
// ============================================================================

bool EffectsWriter::writeColorsCfg(const QString& filepath, const EffectsProject& project) {
    QFile file(filepath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        m_lastError = QString("Cannot write file: %1").arg(filepath);
        return false;
    }

    QTextStream out(&file);
    const SceneColors& c = project.sceneColors;

    // Ambient
    out << "scene_ambinet_red (" << c.ambientR << ");\n";
    out << "scene_ambinet_green (" << c.ambientG << ");\n";
    out << "scene_ambinet_blue (" << c.ambientB << ");\n\n";

    // Background
    out << "scene_bgcolor_red (" << c.bgColorR << ");\n";
    out << "scene_bgcolor_green (" << c.bgColorG << ");\n";
    out << "scene_bgcolor_blue (" << c.bgColorB << ");\n\n";

    // Sun
    out << "scene_sun_blue (" << c.sunB << ");\n";
    out << "scene_sun_green (" << c.sunG << ");\n";
    out << "scene_sun_red (" << c.sunR << ");\n";
    out << "scene_sun_pitch (" << formatFloat(c.sunPitch, 1) << ");\n";
    out << "scene_sun_heading (" << formatFloat(c.sunHeading, 1) << ");\n\n";

    // Fog
    out << "Effect_FogColorR (" << c.fogR << ");\n";
    out << "Effect_FogColorG (" << c.fogG << ");\n";
    out << "Effect_FogColorB (" << c.fogB << ");\n\n";

    out << "Effect_FogHalfColorR (" << c.fogHalfR << ");\n";
    out << "Effect_FogHalfColorG (" << c.fogHalfG << ");\n";
    out << "Effect_FogHalfColorB (" << c.fogHalfB << ");\n\n\n";

    // Light of God
    out << "Effect_LightOfGod_Height (" << formatFloat(c.logHeight, 1) << ");\n";
    out << "Effect_LightOfGod_Range (" << formatFloat(c.logRange, 1) << ");\n";
    out << "Effect_LightOfGod_Strength (" << formatFloat(c.logStrength, 1) << ");\n";
    out << "Effect_LightOfGod_Variation (" << c.logVariation << ");\n\n";

    out << "Effect_LightOfGod_ColorR (" << formatFloat(c.logColorR, 1) << ");\n";
    out << "Effect_LightOfGod_ColorG (" << formatFloat(c.logColorG, 1) << ");\n";
    out << "Effect_LightOfGod_ColorB (" << formatFloat(c.logColorB, 1) << ");\n";

    file.close();
    return true;
}

// ============================================================================
// WRITE ALL
// ============================================================================

bool EffectsWriter::writeAll(const EffectsProject& project) {
    bool success = true;

    if (!project.emittersCfgPath.isEmpty()) {
        if (!writeEmittersCfg(project.emittersCfgPath, project)) {
            success = false;
        }
    }

    if (!project.emitterMaterialsCfgPath.isEmpty()) {
        if (!writeEmitterMaterialsCfg(project.emitterMaterialsCfgPath, project)) {
            success = false;
        }
    }

    if (!project.explosionsCfgPath.isEmpty()) {
        if (!writeExplosionsCfg(project.explosionsCfgPath, project)) {
            success = false;
        }
    }

    if (!project.explosionsSettingsCfgPath.isEmpty()) {
        if (!writeExplosionsSettingsCfg(project.explosionsSettingsCfgPath, project)) {
            success = false;
        }
    }

    if (!project.colorsCfgPath.isEmpty()) {
        if (!writeColorsCfg(project.colorsCfgPath, project)) {
            success = false;
        }
    }

    return success;
}

} // namespace Effects
