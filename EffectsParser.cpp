#include "EffectsParser.h"
#include <QFile>
#include <QTextStream>
#include <QRegularExpression>
#include <QDir>
#include <QDebug>

namespace Effects {

EffectsParser::EffectsParser() {}

// ============================================================================
// HELPER FUNCTIONS
// ============================================================================

QStringList EffectsParser::extractParameters(const QString& line) {
    // Extract content between parentheses
    int start = line.indexOf('(');
    int end = line.lastIndexOf(')');
    if (start == -1 || end == -1 || end <= start) {
        return QStringList();
    }

    QString params = line.mid(start + 1, end - start - 1);

    // Split by comma, but handle quoted strings
    QStringList result;
    QString current;
    bool inQuotes = false;

    for (int i = 0; i < params.length(); i++) {
        QChar c = params[i];
        if (c == '"') {
            inQuotes = !inQuotes;
            current += c;
        } else if (c == ',' && !inQuotes) {
            result.append(current.trimmed());
            current.clear();
        } else {
            current += c;
        }
    }
    if (!current.trimmed().isEmpty()) {
        result.append(current.trimmed());
    }

    return result;
}

QString EffectsParser::extractStringParam(const QString& param) {
    QString s = param.trimmed();
    if (s.startsWith('"') && s.endsWith('"')) {
        return s.mid(1, s.length() - 2);
    }
    return s;
}

float EffectsParser::extractFloat(const QString& param) {
    return param.trimmed().toFloat();
}

int EffectsParser::extractInt(const QString& param) {
    return param.trimmed().toInt();
}

// ============================================================================
// EMITTERS.CFG PARSER
// From IDA: sub_4B7F00 - parameter order verified, offsets in comments
// ============================================================================

bool EffectsParser::parseEmitterCreate(const QString& line, Emitter& emitter) {
    QStringList params = extractParameters(line);
    if (params.size() < 22) {
        m_lastError = QString("Emitter_Create needs 22 parameters, got %1").arg(params.size());
        return false;
    }

    // Parameter order from cfg file, IDA offsets in comments
    emitter.id = extractInt(params[0]);                  // +32
    emitter.birthRate = extractFloat(params[1]);         // +48
    emitter.reuseParticles = extractInt(params[2]);      // +96
    emitter.deathWish = extractInt(params[3]);           // +56
    emitter.speed = extractFloat(params[4]);             // +88 (Size in IDA)
    emitter.lifeTime = extractFloat(params[5]);          // +80
    emitter.variableLifeTime = extractFloat(params[6]);  // +84
    emitter.numParticles = extractInt(params[7]);        // +44
    emitter.size = extractFloat(params[8]);              // +60 (SizeVariation in IDA)
    emitter.spread = extractFloat(params[9]);            // +64
    emitter.speedModifier = extractFloat(params[10]);    // +68 (Speed in IDA)
    emitter.spreadAround = extractFloat(params[11]);     // +72
    emitter.spreadAroundStart = extractFloat(params[12]);// +76
    emitter.acceleration = extractFloat(params[13]);     // +92
    emitter.posVariationX = extractFloat(params[14]);    // +100
    emitter.posVariationY = extractFloat(params[15]);    // +104
    emitter.posVariationZ = extractFloat(params[16]);    // +108
    emitter.shrink = extractFloat(params[17]);           // +112
    emitter.sizeVariation = extractFloat(params[18]);    // +116
    emitter.speedVariation = extractFloat(params[19]);   // +120
    emitter.speedVariationCoeff = extractFloat(params[20]); // +124
    emitter.spreadCoefficient = extractFloat(params[21]); // +128

    return true;
}

bool EffectsParser::parseGradientPoint(const QString& line, GradientPoint& point) {
    QStringList params = extractParameters(line);
    if (params.size() < 5) {
        m_lastError = QString("Emitter_AddGradientPoint needs 5 parameters, got %1").arg(params.size());
        return false;
    }

    point.position = extractInt(params[0]);
    point.r = extractFloat(params[1]);
    point.g = extractFloat(params[2]);
    point.b = extractFloat(params[3]);
    point.alpha = extractFloat(params[4]);

    return true;
}

bool EffectsParser::parseEmitterSetName(const QString& line, QString& name) {
    QStringList params = extractParameters(line);
    if (params.isEmpty()) {
        return false;
    }
    name = extractStringParam(params[0]);
    return true;
}

bool EffectsParser::parseEmittersCfg(const QString& filepath, EffectsProject& project) {
    QFile file(filepath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        m_lastError = QString("Cannot open file: %1").arg(filepath);
        return false;
    }

    project.emittersCfgPath = filepath;

    QTextStream in(&file);
    Emitter currentEmitter;
    bool hasEmitter = false;

    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();

        // Skip empty lines and comments
        if (line.isEmpty() || line.startsWith("//")) {
            continue;
        }

        if (line.startsWith("Emitter_Create")) {
            // Save previous emitter if exists
            if (hasEmitter && currentEmitter.id > 0) {
                project.emitters[currentEmitter.id] = currentEmitter;
            }

            currentEmitter = Emitter();
            hasEmitter = parseEmitterCreate(line, currentEmitter);
        }
        else if (line.startsWith("Emitter_AddGradientPoint") && hasEmitter) {
            GradientPoint point;
            if (parseGradientPoint(line, point)) {
                currentEmitter.gradientPoints.append(point);
            }
        }
        else if (line.startsWith("Emitter_SetName") && hasEmitter) {
            parseEmitterSetName(line, currentEmitter.name);
        }
    }

    // Save last emitter
    if (hasEmitter && currentEmitter.id > 0) {
        project.emitters[currentEmitter.id] = currentEmitter;
    }

    file.close();
    return true;
}

// ============================================================================
// EMITTERMATERIALS.CFG PARSER
// ============================================================================

bool EffectsParser::parseEmitterSetMaterial(const QString& line, int& id, QString& material) {
    QStringList params = extractParameters(line);
    if (params.size() < 2) {
        return false;
    }

    id = extractInt(params[0]);
    material = extractStringParam(params[1]);
    return true;
}

bool EffectsParser::parseEmitterMaterialsCfg(const QString& filepath, EffectsProject& project) {
    QFile file(filepath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        m_lastError = QString("Cannot open file: %1").arg(filepath);
        return false;
    }

    project.emitterMaterialsCfgPath = filepath;

    QTextStream in(&file);

    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();

        // Remove inline comments
        int commentPos = line.indexOf("//");
        if (commentPos > 0) {
            line = line.left(commentPos).trimmed();
        }

        if (line.isEmpty() || line.startsWith("//")) {
            continue;
        }

        if (line.startsWith("Emitter_SetMaterial")) {
            int id;
            QString material;
            if (parseEmitterSetMaterial(line, id, material)) {
                project.emitterMaterials[id] = material;
            }
        }
    }

    file.close();

    // Apply materials to emitters
    project.applyMaterials();

    return true;
}

// ============================================================================
// EXPLOSIONS.CFG PARSER
// ============================================================================

bool EffectsParser::parseExplosionCreate(const QString& line, QString& name) {
    QStringList params = extractParameters(line);
    if (params.isEmpty()) {
        return false;
    }
    name = extractStringParam(params[0]);
    return true;
}

bool EffectsParser::parseExplosionsCfg(const QString& filepath, EffectsProject& project) {
    QFile file(filepath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        m_lastError = QString("Cannot open file: %1").arg(filepath);
        return false;
    }

    project.explosionsCfgPath = filepath;

    QTextStream in(&file);

    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();

        if (line.isEmpty() || line.startsWith("//")) {
            continue;
        }

        if (line.startsWith("Effect_Explo_Create")) {
            QString name;
            if (parseExplosionCreate(line, name)) {
                if (!project.explosions.contains(name)) {
                    Explosion explo;
                    explo.name = name;
                    project.explosions[name] = explo;
                }
            }
        }
    }

    file.close();
    return true;
}

// ============================================================================
// EXPLOSIONSSETTINGS.CFG PARSER
// ============================================================================

bool EffectsParser::parseExplosionAddObject(const QString& line, QString& exploName, ExplosionObject& obj) {
    QStringList params = extractParameters(line);
    if (params.size() < 9) {
        return false;
    }

    exploName = extractStringParam(params[0]);
    obj.materialName = extractStringParam(params[1]);
    obj.scale = extractFloat(params[2]);        // +36
    obj.delay = extractFloat(params[3]);        // +40
    obj.duration = extractFloat(params[4]);     // +44
    obj.variation = extractFloat(params[5]);    // +48
    obj.additive = extractInt(params[6]);       // +52 (bool)
    obj.loop = extractInt(params[7]);           // +53 (bool)
    obj.billboard = extractInt(params[8]);      // +25 (bool)

    return true;
}

bool EffectsParser::parseExplosionAddThrowObject(const QString& line, QString& exploName, ThrowObject& obj) {
    QStringList params = extractParameters(line);
    if (params.size() < 8) {
        return false;
    }

    exploName = extractStringParam(params[0]);
    obj.materialName = extractStringParam(params[1]);
    obj.scale = extractFloat(params[2]);        // +36
    obj.delay = extractFloat(params[3]);        // +40
    obj.duration = extractFloat(params[4]);     // +44
    obj.variation = extractFloat(params[5]);    // +48
    obj.speed = extractFloat(params[6]);        // +28
    obj.count = extractFloat(params[7]);        // +32

    return true;
}

bool EffectsParser::parseExplosionSetScale(const QString& line, QString& exploName, float& x, float& y, float& z) {
    QStringList params = extractParameters(line);
    if (params.size() < 4) {
        return false;
    }

    exploName = extractStringParam(params[0]);
    x = extractFloat(params[1]);
    y = extractFloat(params[2]);
    z = extractFloat(params[3]);

    return true;
}

bool EffectsParser::parseExplosionSetShake(const QString& line, QString& exploName, float& value) {
    QStringList params = extractParameters(line);
    if (params.size() < 2) {
        return false;
    }

    exploName = extractStringParam(params[0]);
    value = extractFloat(params[1]);

    return true;
}

bool EffectsParser::parseExplosionSetSound(const QString& line, QString& exploName, QString& sound) {
    QStringList params = extractParameters(line);
    if (params.size() < 2) {
        return false;
    }

    exploName = extractStringParam(params[0]);
    sound = extractStringParam(params[1]);

    return true;
}

bool EffectsParser::parseExplosionPolyBlowSpeed(const QString& line, QString& exploName, float& speed) {
    QStringList params = extractParameters(line);
    if (params.size() < 2) {
        return false;
    }

    exploName = extractStringParam(params[0]);
    speed = extractFloat(params[1]);

    return true;
}

bool EffectsParser::parseExplosionDebrisExplosion(const QString& line, QString& exploName, QString& debris) {
    QStringList params = extractParameters(line);
    if (params.size() < 2) {
        return false;
    }

    exploName = extractStringParam(params[0]);
    debris = extractStringParam(params[1]);

    return true;
}

bool EffectsParser::parseExplosionLock(const QString& line, QString& exploName) {
    QStringList params = extractParameters(line);
    if (params.isEmpty()) {
        return false;
    }

    exploName = extractStringParam(params[0]);
    return true;
}

bool EffectsParser::parseExplosionSetPressureWave(const QString& line, QString& exploName,
                                                  float& force, float& innerRadius, float& outerRadius) {
    QStringList params = extractParameters(line);
    if (params.size() < 4) {
        return false;
    }

    exploName = extractStringParam(params[0]);
    force = extractFloat(params[1]);
    innerRadius = extractFloat(params[2]);
    outerRadius = extractFloat(params[3]);

    return true;
}

bool EffectsParser::parseExplosionsSettingsCfg(const QString& filepath, EffectsProject& project) {
    QFile file(filepath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        m_lastError = QString("Cannot open file: %1").arg(filepath);
        return false;
    }

    project.explosionsSettingsCfgPath = filepath;

    QTextStream in(&file);

    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();

        if (line.isEmpty() || line.startsWith("//")) {
            continue;
        }

        QString exploName;

        if (line.startsWith("Effect_Explo_AddObject")) {
            ExplosionObject obj;
            if (parseExplosionAddObject(line, exploName, obj)) {
                if (project.explosions.contains(exploName)) {
                    project.explosions[exploName].objects.append(obj);
                } else {
                    // Create explosion if not exists
                    Explosion explo;
                    explo.name = exploName;
                    explo.objects.append(obj);
                    project.explosions[exploName] = explo;
                }
            }
        }
        else if (line.startsWith("Effect_Explo_AddThrowObject")) {
            ThrowObject obj;
            if (parseExplosionAddThrowObject(line, exploName, obj)) {
                if (project.explosions.contains(exploName)) {
                    project.explosions[exploName].throwObjects.append(obj);
                }
            }
        }
        else if (line.startsWith("Effect_Explo_SetScale")) {
            float x, y, z;
            if (parseExplosionSetScale(line, exploName, x, y, z)) {
                if (project.explosions.contains(exploName)) {
                    project.explosions[exploName].scaleX = x;
                    project.explosions[exploName].scaleY = y;
                    project.explosions[exploName].scaleZ = z;
                }
            }
        }
        else if (line.startsWith("Effect_Explo_SetShake")) {
            float value;
            if (parseExplosionSetShake(line, exploName, value)) {
                if (project.explosions.contains(exploName)) {
                    project.explosions[exploName].shake = value;
                }
            }
        }
        else if (line.startsWith("Effect_Explo_SetSound")) {
            QString sound;
            if (parseExplosionSetSound(line, exploName, sound)) {
                if (project.explosions.contains(exploName)) {
                    project.explosions[exploName].soundFile = sound;
                }
            }
        }
        else if (line.startsWith("Effect_Explo_PolyBlowSpeed")) {
            float speed;
            if (parseExplosionPolyBlowSpeed(line, exploName, speed)) {
                if (project.explosions.contains(exploName)) {
                    project.explosions[exploName].polyBlowSpeed = speed;
                }
            }
        }
        else if (line.startsWith("Effect_Explo_DebrisExplosion")) {
            QString debris;
            if (parseExplosionDebrisExplosion(line, exploName, debris)) {
                if (project.explosions.contains(exploName)) {
                    project.explosions[exploName].debrisExplosion = debris;
                }
            }
        }
        else if (line.startsWith("Effect_Explo_Lock")) {
            if (parseExplosionLock(line, exploName)) {
                if (project.explosions.contains(exploName)) {
                    project.explosions[exploName].isLocked = true;
                }
            }
        }
        else if (line.startsWith("Effect_Explo_SetPressureWave")) {
            float force, innerRadius, outerRadius;
            if (parseExplosionSetPressureWave(line, exploName, force, innerRadius, outerRadius)) {
                if (project.explosions.contains(exploName)) {
                    project.explosions[exploName].hasPressureWave = true;
                    project.explosions[exploName].pressureWaveForce = force;
                    project.explosions[exploName].pressureWaveInnerRadius = innerRadius;
                    project.explosions[exploName].pressureWaveOuterRadius = outerRadius;
                }
            }
        }
    }

    file.close();
    return true;
}

// ============================================================================
// COLORS.CFG PARSER
// ============================================================================

bool EffectsParser::parseColorSetting(const QString& line, SceneColors& colors) {
    // Extract function name and value
    int parenStart = line.indexOf('(');
    int parenEnd = line.indexOf(')');
    if (parenStart == -1 || parenEnd == -1) {
        return false;
    }

    QString funcName = line.left(parenStart).trimmed();
    QString valueStr = line.mid(parenStart + 1, parenEnd - parenStart - 1).trimmed();

    // Scene ambient
    if (funcName == "scene_ambinet_red") colors.ambientR = valueStr.toInt();
    else if (funcName == "scene_ambinet_green") colors.ambientG = valueStr.toInt();
    else if (funcName == "scene_ambinet_blue") colors.ambientB = valueStr.toInt();

    // Background
    else if (funcName == "scene_bgcolor_red") colors.bgColorR = valueStr.toInt();
    else if (funcName == "scene_bgcolor_green") colors.bgColorG = valueStr.toInt();
    else if (funcName == "scene_bgcolor_blue") colors.bgColorB = valueStr.toInt();

    // Sun
    else if (funcName == "scene_sun_red") colors.sunR = valueStr.toInt();
    else if (funcName == "scene_sun_green") colors.sunG = valueStr.toInt();
    else if (funcName == "scene_sun_blue") colors.sunB = valueStr.toInt();
    else if (funcName == "scene_sun_pitch") colors.sunPitch = valueStr.toFloat();
    else if (funcName == "scene_sun_heading") colors.sunHeading = valueStr.toFloat();

    // Fog
    else if (funcName == "Effect_FogColorR") colors.fogR = valueStr.toInt();
    else if (funcName == "Effect_FogColorG") colors.fogG = valueStr.toInt();
    else if (funcName == "Effect_FogColorB") colors.fogB = valueStr.toInt();
    else if (funcName == "Effect_FogHalfColorR") colors.fogHalfR = valueStr.toInt();
    else if (funcName == "Effect_FogHalfColorG") colors.fogHalfG = valueStr.toInt();
    else if (funcName == "Effect_FogHalfColorB") colors.fogHalfB = valueStr.toInt();

    // Light of God
    else if (funcName == "Effect_LightOfGod_Height") colors.logHeight = valueStr.toFloat();
    else if (funcName == "Effect_LightOfGod_Range") colors.logRange = valueStr.toFloat();
    else if (funcName == "Effect_LightOfGod_Strength") colors.logStrength = valueStr.toFloat();
    else if (funcName == "Effect_LightOfGod_Variation") colors.logVariation = valueStr.toInt();
    else if (funcName == "Effect_LightOfGod_ColorR") colors.logColorR = valueStr.toFloat();
    else if (funcName == "Effect_LightOfGod_ColorG") colors.logColorG = valueStr.toFloat();
    else if (funcName == "Effect_LightOfGod_ColorB") colors.logColorB = valueStr.toFloat();

    return true;
}

bool EffectsParser::parseColorsCfg(const QString& filepath, EffectsProject& project) {
    QFile file(filepath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        m_lastError = QString("Cannot open file: %1").arg(filepath);
        return false;
    }

    project.colorsCfgPath = filepath;

    QTextStream in(&file);

    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();

        if (line.isEmpty() || line.startsWith("//")) {
            continue;
        }

        parseColorSetting(line, project.sceneColors);
    }

    file.close();
    return true;
}

// ============================================================================
// DIRECTORY PARSER
// ============================================================================

bool EffectsParser::parseDirectory(const QString& dirPath, EffectsProject& project) {
    QDir dir(dirPath);
    if (!dir.exists()) {
        m_lastError = QString("Directory does not exist: %1").arg(dirPath);
        return false;
    }

    project.clear();
    bool anyParsed = false;

    // Look for config files
    QString emittersCfg = dir.filePath("Emitters.cfg");
    QString emitterMaterialsCfg = dir.filePath("EmitterMaterials.cfg");
    QString explosionsCfg = dir.filePath("Explosions.cfg");
    QString explosionsSettingsCfg = dir.filePath("ExplosionsSettings.cfg");
    QString colorsCfg = dir.filePath("Colors.cfg");

    if (QFile::exists(emittersCfg)) {
        parseEmittersCfg(emittersCfg, project);
        anyParsed = true;
    }

    if (QFile::exists(emitterMaterialsCfg)) {
        parseEmitterMaterialsCfg(emitterMaterialsCfg, project);
        anyParsed = true;
    }

    if (QFile::exists(explosionsCfg)) {
        parseExplosionsCfg(explosionsCfg, project);
        anyParsed = true;
    }

    if (QFile::exists(explosionsSettingsCfg)) {
        parseExplosionsSettingsCfg(explosionsSettingsCfg, project);
        anyParsed = true;
    }

    if (QFile::exists(colorsCfg)) {
        parseColorsCfg(colorsCfg, project);
        anyParsed = true;
    }

    if (!anyParsed) {
        m_lastError = "No effects configuration files found in directory";
        return false;
    }

    return true;
}

} // namespace Effects
