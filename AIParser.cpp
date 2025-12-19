#include "AIParser.h"
#include <QFile>
#include <QTextStream>
#include <QDir>
#include <QFileInfo>
#include <QRegularExpression>
#include <QDebug>

namespace AI {

AIParser::AIParser() {}

QString AIParser::buildFilePath(const QString& baseDir, const QString& race,
                                const QString& fileType, int difficulty) {
    // fileType: "InitParameters", "BuildFitness", "MaxNumberOfUnits"
    return QString("%1/%2/%3%4.ai").arg(baseDir, race, fileType).arg(difficulty);
}

bool AIParser::parseInitParameters(const QString& filePath, InitParameters& params) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        m_lastError = QString("Cannot open file: %1").arg(filePath);
        return false;
    }

    params.clear();
    params.filePath = filePath;

    // Extract race name and difficulty from path
    QFileInfo fileInfo(filePath);
    params.raceName = fileInfo.dir().dirName();

    QString baseName = fileInfo.baseName();  // "InitParameters0"
    if (baseName.startsWith("InitParameters", Qt::CaseInsensitive)) {
        params.difficulty = baseName.mid(14).toInt();
    }

    QTextStream in(&file);

    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();

        // Skip empty lines and comments
        if (line.isEmpty() || line.startsWith("//") || line.startsWith(";")) {
            continue;
        }

        // Format: Name\tValue\tVariance (TAB separated)
        QStringList parts;
        if (line.contains('\t')) {
            parts = line.split('\t', Qt::SkipEmptyParts);
        } else {
            parts = line.split(QRegularExpression("\\s+"), Qt::SkipEmptyParts);
        }

        if (parts.size() >= 2) {
            QString name = parts[0].trimmed();
            float value = parts[1].trimmed().toFloat();
            float variance = (parts.size() >= 3) ? parts[2].trimmed().toFloat() : 0.0f;

            // Check if float (has decimal point)
            bool isFloat = parts[1].contains('.') || (parts.size() >= 3 && parts[2].contains('.'));

            params.setParam(name, value, variance, isFloat);
        }
    }

    file.close();
    return true;
}

bool AIParser::parseBuildFitness(const QString& filePath, BuildFitness& fitness) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        m_lastError = QString("Cannot open file: %1").arg(filePath);
        return false;
    }

    fitness.clear();
    fitness.filePath = filePath;

    // Extract race name and difficulty from path
    QFileInfo fileInfo(filePath);
    fitness.raceName = fileInfo.dir().dirName();

    QString baseName = fileInfo.baseName();  // "BuildFitness0"
    if (baseName.startsWith("BuildFitness", Qt::CaseInsensitive)) {
        fitness.difficulty = baseName.mid(12).toInt();
    }

    QTextStream in(&file);

    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();

        // Skip empty lines and comments
        if (line.isEmpty() || line.startsWith("//") || line.startsWith(";")) {
            continue;
        }

        // Format: UnitName\tFitness (TAB separated)
        QStringList parts;
        if (line.contains('\t')) {
            parts = line.split('\t', Qt::SkipEmptyParts);
        } else {
            parts = line.split(QRegularExpression("\\s+"), Qt::SkipEmptyParts);
        }

        if (parts.size() >= 2) {
            QString unitName = parts[0].trimmed();
            bool ok;
            int fitnessValue = parts[1].trimmed().toInt(&ok);

            if (ok && !unitName.isEmpty()) {
                fitness.entries.append(BuildFitnessEntry(unitName, fitnessValue));
            }
        }
    }

    file.close();
    return true;
}

bool AIParser::parseMaxUnits(const QString& filePath, MaxUnits& maxUnits) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        m_lastError = QString("Cannot open file: %1").arg(filePath);
        return false;
    }

    maxUnits.clear();
    maxUnits.filePath = filePath;

    // Extract race name and difficulty from path
    QFileInfo fileInfo(filePath);
    maxUnits.raceName = fileInfo.dir().dirName();

    QString baseName = fileInfo.baseName();  // "MaxNumberOfUnits0"
    if (baseName.startsWith("MaxNumberOfUnits", Qt::CaseInsensitive)) {
        maxUnits.difficulty = baseName.mid(16).toInt();
    }

    QTextStream in(&file);

    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();

        // Skip empty lines and comments
        if (line.isEmpty() || line.startsWith("//") || line.startsWith(";")) {
            continue;
        }

        // Format: UnitName\tMaxCount (TAB separated)
        QStringList parts;
        if (line.contains('\t')) {
            parts = line.split('\t', Qt::SkipEmptyParts);
        } else {
            parts = line.split(QRegularExpression("\\s+"), Qt::SkipEmptyParts);
        }

        if (parts.size() >= 2) {
            QString unitName = parts[0].trimmed();
            bool ok;
            int maxCount = parts[1].trimmed().toInt(&ok);

            if (ok && !unitName.isEmpty()) {
                maxUnits.entries.append(MaxUnitsEntry(unitName, maxCount));
            }
        }
    }

    file.close();
    return true;
}

bool AIParser::scanDirectory(const QString& aiValuesDir, AIProject& project) {
    QDir dir(aiValuesDir);
    if (!dir.exists()) {
        m_lastError = QString("Directory does not exist: %1").arg(aiValuesDir);
        return false;
    }

    project.clear();
    project.baseDir = aiValuesDir;

    // Find ALL subdirectories as race folders
    QStringList subDirs = dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
    for (const QString& subDir : subDirs) {
        // Check if this looks like a race folder (has AI files)
        QDir raceDir(dir.filePath(subDir));
        QStringList aiFiles = raceDir.entryList(QStringList() << "*.ai", QDir::Files);
        if (!aiFiles.isEmpty()) {
            project.availableRaces.append(subDir);
        }
    }

    // Sort for consistent display
    project.availableRaces.sort(Qt::CaseInsensitive);

    return true;
}

} // namespace AI
