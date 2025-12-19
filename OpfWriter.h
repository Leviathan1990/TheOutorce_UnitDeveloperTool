#ifndef OPFWRITER_H
#define OPFWRITER_H

#include "OpfStructs.h"
#include <QString>
#include <QFile>
#include <QDataStream>

namespace Opf {

class OpfWriter
{
public:
    OpfWriter();

    bool write(const QString& filename, const PackedProject& project);
    bool writeBackup(const QString& originalFile, const PackedProject& project);

    QString lastError() const { return m_lastError; }

private:
    template<typename T>
    void writeValue(T value);

    void writeBytes(const char* data, qint64 size);
    void writeBytes(const QByteArray& data);

    void writeOutforceString(const QString& str);
    void writeOutforceString32(const QString& str);
    void writeStaticString(const QString& str, int length);

    bool writeProjectHeader(const PackedProject& project);
    bool writeDependencies(const QVector<QString>& dependencies);
    bool writeEventDescs(const QVector<EventDesc>& events);
    bool writeTextures(const QVector<Texture>& textures);
    bool writeMaterials(const QVector<Material>& materials);
    bool writeObjects(const QVector<Object*>& objects);

    void writeTexture(const Texture& texture);
    void writeBitmap(const Bitmap& bitmap);
    void writeMaterial(const Material& material);
    void writeRenderPassSettings(const RenderPassSettings& settings);
    void writeRenderPassStage(const RenderPassStage& stage);
    void writeMaterial7(const Material7& mat);
    void writeObject(const Object& object);
    void writeObjectTemplate(const ObjectTemplate& templ);
    void writeMesh(const Mesh& mesh);
    void writeLight(const Light& light);
    void writeCustomSettings(const QVector<CustomSetting>& settings);

    void writeVec3(const Vector3D& v);
    void writeVec2(const Vector2D& v);
    void writeColorValue(const ColorValue& c);

    QFile m_file;
    QDataStream m_stream;
    QString m_lastError;
};

} // namespace Opf

#endif // OPFWRITER_H
