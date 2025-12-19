#ifndef AIPARSER_H
#define AIPARSER_H

#include <QString>
#include <QStringList>
#include "AIStructs.h"

namespace AI {

class AIParser {
public:
    AIParser();

    // Parse InitParameters from .ai file
    // Format: Name\tValue\tVariance (TAB separated, // comments)
    bool parseInitParameters(const QString& filePath, InitParameters& params);

    // Parse Build Fitness from .ai file
    // Format: UnitName\tFitness (TAB separated)
    bool parseBuildFitness(const QString& filePath, BuildFitness& fitness);

    // Parse Max Units from .ai file
    // Format: UnitName\tMaxCount (TAB separated)
    bool parseMaxUnits(const QString& filePath, MaxUnits& maxUnits);

    // Scan AIVALUES directory for available races
    bool scanDirectory(const QString& aiValuesDir, AIProject& project);

    // Build file path for specific file type
    static QString buildFilePath(const QString& baseDir, const QString& race,
                                 const QString& fileType, int difficulty);

    // Get last error message
    QString lastError() const { return m_lastError; }

private:
    QString m_lastError;

    // Generic TAB-separated parser for unit-value pairs
    bool parseUnitValueFile(const QString& filePath, QVector<QPair<QString, int>>& entries);
};

} // namespace AI

#endif // AIPARSER_H
