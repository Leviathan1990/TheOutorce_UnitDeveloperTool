#include "AIWriter.h"
#include <QFile>
#include <QTextStream>
#include <QFileInfo>
#include <QDir>

namespace AI {

AIWriter::AIWriter() {}

bool AIWriter::writeInitParameters(const QString& filePath, const InitParameters& params) {
    QFileInfo fileInfo(filePath);
    QDir dir = fileInfo.dir();
    if (!dir.exists()) {
        if (!dir.mkpath(".")) {
            m_lastError = QString("Cannot create directory: %1").arg(dir.path());
            return false;
        }
    }

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        m_lastError = QString("Cannot open file for writing: %1").arg(filePath);
        return false;
    }

    QTextStream out(&file);

    // Write header comment
    out << "// " << fileInfo.fileName() << "\n";
    out << "\n";

    // Get known parameter names for ordering
    QStringList knownNames = getKnownInitParamNames();
    QStringList writtenNames;

    // Write known parameters first (preserves expected order)
    for (const QString& name : knownNames) {
        if (params.params.contains(name)) {
            const InitParameter& p = params.params[name];
            if (p.isFloat) {
                out << p.name << "\t" << QString::number(p.value, 'f', 2)
                << "\t" << QString::number(p.variance, 'f', 2) << "\n";
            } else {
                out << p.name << "\t" << static_cast<int>(p.value)
                << "\t" << static_cast<int>(p.variance) << "\n";
            }
            writtenNames.append(name);
        }
    }

    // Write any custom parameters
    for (auto it = params.params.begin(); it != params.params.end(); ++it) {
        if (!writtenNames.contains(it.key())) {
            const InitParameter& p = it.value();
            if (p.isFloat) {
                out << p.name << "\t" << QString::number(p.value, 'f', 2)
                << "\t" << QString::number(p.variance, 'f', 2) << "\n";
            } else {
                out << p.name << "\t" << static_cast<int>(p.value)
                << "\t" << static_cast<int>(p.variance) << "\n";
            }
        }
    }

    file.close();
    return true;
}

bool AIWriter::writeBuildFitness(const QString& filePath, const BuildFitness& fitness) {
    QFileInfo fileInfo(filePath);
    QDir dir = fileInfo.dir();
    if (!dir.exists()) {
        if (!dir.mkpath(".")) {
            m_lastError = QString("Cannot create directory: %1").arg(dir.path());
            return false;
        }
    }

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        m_lastError = QString("Cannot open file for writing: %1").arg(filePath);
        return false;
    }

    QTextStream out(&file);

    // Write entries in TAB-separated format: UnitName\tFitnessValue
    for (const BuildFitnessEntry& entry : fitness.entries) {
        out << entry.unitName << "\t" << entry.fitness << "\n";
    }

    file.close();
    return true;
}

bool AIWriter::writeMaxUnits(const QString& filePath, const MaxUnits& maxUnits) {
    QFileInfo fileInfo(filePath);
    QDir dir = fileInfo.dir();
    if (!dir.exists()) {
        if (!dir.mkpath(".")) {
            m_lastError = QString("Cannot create directory: %1").arg(dir.path());
            return false;
        }
    }

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        m_lastError = QString("Cannot open file for writing: %1").arg(filePath);
        return false;
    }

    QTextStream out(&file);

    // Write entries in TAB-separated format: UnitName\tMaxCount
    for (const MaxUnitsEntry& entry : maxUnits.entries) {
        out << entry.unitName << "\t" << entry.maxCount << "\n";
    }

    file.close();
    return true;
}

} // namespace AI
