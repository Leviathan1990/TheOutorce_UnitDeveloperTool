#ifndef AIWRITER_H
#define AIWRITER_H

#include <QString>
#include "AIStructs.h"

namespace AI {

class AIWriter {
public:
    AIWriter();

    // Write InitParameters to .ai file
    bool writeInitParameters(const QString& filePath, const InitParameters& params);

    // Write Build Fitness to .ai file
    bool writeBuildFitness(const QString& filePath, const BuildFitness& fitness);

    // Write Max Units to .ai file
    bool writeMaxUnits(const QString& filePath, const MaxUnits& maxUnits);

    // Get last error message
    QString lastError() const { return m_lastError; }

private:
    QString m_lastError;
};

} // namespace AI

#endif // AIWRITER_H
