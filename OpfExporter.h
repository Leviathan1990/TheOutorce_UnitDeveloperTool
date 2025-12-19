#ifndef OPFEXPORTER_H
#define OPFEXPORTER_H

#include "OpfStructs.h"
#include <QString>
#include <QObject>
#include <QDir>

class QProgressDialog;

namespace Opf {

class OpfExporter : public QObject
{
    Q_OBJECT

public:
    explicit OpfExporter(QObject* parent = nullptr);

    // Export templates to JSON for Map Editor
    bool exportTemplatesToJson(const PackedProject& project, const QString& filename);

    // Export single object to file
    bool exportObject(const Object& object, const QString& filename);

    // Export texture metadata
    bool exportTexture(const Texture& texture, const QString& filename);

    // Export mesh to OBJ format
    bool exportMeshToObj(const Mesh& mesh, const QString& filename, const PackedProject& project);

    // Export all meshes of an object to OBJ files
    bool exportObjectMeshes(const Object& object, const QString& directory, const PackedProject& project);

    // Export texture to PNG
    bool exportTextureToPng(const Texture& texture, const QString& filename);

    // Export all textures
    bool exportAllTextures(const QVector<Texture>& textures, const QString& directory);

    // Export material to JSON
    bool exportMaterialToJson(const Material& material, const QString& filename);

    // Export mesh MTL file
    bool exportMeshMtl(const Mesh& mesh, const QString& mtlFilename, const PackedProject& project);

    // Find texture by material ID
    const Texture* findTextureForMaterial(int materialID, const PackedProject& project);

    // Export Blender import script
    bool exportBlenderImportScript(const PackedProject& project, const QString& filename);

    // Export all assets with progress tracking
    bool exportAll(const PackedProject& project, const QString& directory, QProgressDialog* progress = nullptr);

    QString lastError() const { return m_lastError; }

    // Export asset list to text file for verification
    bool exportAssetListToTxt(const PackedProject& project, const QString& filename);

signals:
    void progressUpdated(int value);
    void statusChanged(const QString& status);

private:
    QString m_lastError;
    int m_currentProgress;

    // Cross-platform filename sanitization
    QString sanitizeFilename(const QString& filename);

    // Helper function
    void updateProgress(QProgressDialog* progress, const QString& status = QString());

    // NEW: Recursive helper for deeply nested children
    void exportObjectMeshesRecursiveHelper(const Object& object, const QDir& dir, const PackedProject& project, const QString& parentPrefix);

    // Helper for writing object hierarchy to text
    void writeObjectToList(QTextStream& out, const Object* obj, int depth);

    // Helper for counting children recursively
    int countChildrenRecursive(const Object* obj);


};

} // namespace Opf

#endif // OPFEXPORTER_H
