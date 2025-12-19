#ifndef AISTRUCTS_H
#define AISTRUCTS_H

#include <QString>
#include <QVector>
#include <QMap>
#include <QVariant>

namespace AI {

// ============================================================================
// Difficulty levels - used for all AI files
// ============================================================================
enum Difficulty {
    Easy = 0,
    Medium = 1,
    Hard = 2
};

inline QString difficultyToString(int diff) {
    switch (diff) {
    case 0: return "Easy";
    case 1: return "Medium";
    case 2: return "Hard";
    default: return QString::number(diff);
    }
}

// ============================================================================
// INIT PARAMETER - single AI behavior parameter with variance
// From: InitParameters{0,1,2}.ai files
// Format: Name\tValue\tVariance (TAB separated)
// ============================================================================
struct InitParameter {
    QString name;
    float value;
    float variance;
    bool isFloat;

    InitParameter() : value(0), variance(0), isFloat(false) {}
    InitParameter(const QString& n, float v, float var, bool isF = false)
        : name(n), value(v), variance(var), isFloat(isF) {}
};

// ============================================================================
// INIT PARAMETERS - complete set from one InitParameters file
// ============================================================================
struct InitParameters {
    QMap<QString, InitParameter> params;
    QString filePath;
    QString raceName;
    int difficulty;  // 0=Easy, 1=Medium, 2=Hard

    void clear() {
        params.clear();
        filePath.clear();
        raceName.clear();
        difficulty = 0;
    }

    float getValue(const QString& name, float defaultVal = 0) const {
        if (params.contains(name)) return params[name].value;
        return defaultVal;
    }

    float getVariance(const QString& name, float defaultVar = 0) const {
        if (params.contains(name)) return params[name].variance;
        return defaultVar;
    }

    void setParam(const QString& name, float val, float var, bool isFloat = false) {
        params[name] = InitParameter(name, val, var, isFloat);
    }
};

// ============================================================================
// BUILD FITNESS ENTRY - unit build priority for AI
// From: BuildFitness{0,1,2}.ai files
// Format: UnitName\tFitness (TAB separated)
// Stored at unit template offset 724, default 50
// ============================================================================
struct BuildFitnessEntry {
    QString unitName;
    int fitness;

    BuildFitnessEntry() : fitness(50) {}
    BuildFitnessEntry(const QString& name, int fit)
        : unitName(name), fitness(fit) {}
};

// ============================================================================
// BUILD FITNESS - complete build fitness data
// ============================================================================
struct BuildFitness {
    QVector<BuildFitnessEntry> entries;
    QString filePath;
    QString raceName;
    int difficulty;  // 0=Easy, 1=Medium, 2=Hard

    void clear() {
        entries.clear();
        filePath.clear();
        raceName.clear();
        difficulty = 0;
    }

    int findEntry(const QString& unitName) const {
        for (int i = 0; i < entries.size(); ++i) {
            if (entries[i].unitName.compare(unitName, Qt::CaseInsensitive) == 0) {
                return i;
            }
        }
        return -1;
    }

    int getFitness(const QString& unitName) const {
        int idx = findEntry(unitName);
        return (idx >= 0) ? entries[idx].fitness : 50;
    }

    void setFitness(const QString& unitName, int fitness) {
        int idx = findEntry(unitName);
        if (idx >= 0) {
            entries[idx].fitness = fitness;
        } else {
            entries.append(BuildFitnessEntry(unitName, fitness));
        }
    }
};

// ============================================================================
// MAX UNITS ENTRY - maximum number of units AI can build
// From: MaxNumberOfUnits{0,1,2}.ai files
// Format: UnitName\tMaxCount (TAB separated)
// Stored at unit template offset 732/736, default 10000
// ============================================================================
struct MaxUnitsEntry {
    QString unitName;
    int maxCount;

    MaxUnitsEntry() : maxCount(10000) {}
    MaxUnitsEntry(const QString& name, int max)
        : unitName(name), maxCount(max) {}
};

// ============================================================================
// MAX UNITS - complete max units data
// ============================================================================
struct MaxUnits {
    QVector<MaxUnitsEntry> entries;
    QString filePath;
    QString raceName;
    int difficulty;  // 0=Easy, 1=Medium, 2=Hard

    void clear() {
        entries.clear();
        filePath.clear();
        raceName.clear();
        difficulty = 0;
    }

    int findEntry(const QString& unitName) const {
        for (int i = 0; i < entries.size(); ++i) {
            if (entries[i].unitName.compare(unitName, Qt::CaseInsensitive) == 0) {
                return i;
            }
        }
        return -1;
    }

    int getMaxCount(const QString& unitName) const {
        int idx = findEntry(unitName);
        return (idx >= 0) ? entries[idx].maxCount : 10000;
    }

    void setMaxCount(const QString& unitName, int maxCount) {
        int idx = findEntry(unitName);
        if (idx >= 0) {
            entries[idx].maxCount = maxCount;
        } else {
            entries.append(MaxUnitsEntry(unitName, maxCount));
        }
    }
};

// ============================================================================
// AI PROJECT - contains all AI data for editing
// ============================================================================
struct AIProject {
    InitParameters initParams;
    BuildFitness buildFitness;
    MaxUnits maxUnits;

    QStringList availableRaces;     // Race folders (Humans, Gobins, etc.)
    QString baseDir;                 // AIVALUES directory path

    void clear() {
        initParams.clear();
        buildFitness.clear();
        maxUnits.clear();
        availableRaces.clear();
        baseDir.clear();
    }
};

// ============================================================================
// KNOWN INIT PARAMETERS - from IDA analysis of InitParameters files
// ============================================================================
inline QStringList getKnownInitParamNames() {
    return {
        // Timing
        "AttackDelay",
        "FirstSpyDelay",
        "FirstAttackDelay",
        "buildDelay",
        "InstantAttack",

        // Distance
        "attackDistanceIncrement",
        "DefendRadius",

        // Group sizes
        "numberInSpyGroups",
        "numberInDefendGroups",
        "numberInAttackGroups",
        "wantNumberOfSpyingGroups",
        "wantNumberOfDefendingGroups",
        "wantNumberOfAttackingGroups",
        "NrOfGroupsAttackingTogether",

        // Building distances
        "minDistanceForGroup",
        "minDistanceBetweenBuildings",

        // Warp
        "timeBetweenWarpTargetUpdates",
        "minDistanceToOwnUnitsForWarpNukeToLaunch",

        // Misc
        "forgettingForCollisionGrid",

        // Economy
        "lowEnergy",
        "lowResources",

        // Cheating
        "cheating"
    };
}

} // namespace AI

#endif // AISTRUCTS_H
