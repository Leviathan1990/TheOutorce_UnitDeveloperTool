#ifndef OPFPARSER_H
#define OPFPARSER_H

#include "OpfStructs.h"
#include <QFile>
#include <QDataStream>

namespace Opf {

class OpfParser
{
public:
    OpfParser();

    bool parse(const QString& filename, PackedProject& project);
    QString lastError() const { return m_lastError; }

private:
    QString m_lastError;
    QDataStream* m_stream;

    // ========================================================================
    // MAIN PARSING FUNCTIONS
    // ========================================================================

    bool parseHeader(PackedProject& project);
    bool parseDependencies(PackedProject& project);
    bool parseEventDescs(PackedProject& project);
    bool parseTextures(PackedProject& project);
    bool parseMaterials(PackedProject& project);
    bool parseObjects(PackedProject& project);

    // ========================================================================
    // TEXTURE PARSING
    // ========================================================================

    bool parseTexture(Texture& texture);
    bool parseBitmap(Bitmap& bitmap);
    bool parseBitmapInfoHeader(BitmapInfoHeader& header, uint32 headerSize);

    // ========================================================================
    // MATERIAL PARSING
    // ========================================================================

    bool parseMaterial(Material& material);
    bool parseRenderPassSettings(RenderPassSettings& settings);
    bool parseRenderPassStage(RenderPassStage& stage);
    bool parseMaterial7(Material7& mat);
    bool parseColorValue(ColorValue& color);

    // ========================================================================
    // OBJECT PARSING
    // ========================================================================

    bool parseObject(Object& object);
    bool parseObjectTemplate(ObjectTemplate& templ);
    bool parseFaceBuffer(Mesh& mesh);
    bool parseLight(Light& light);
    bool parseCustomSettings(QVector<CustomSetting>& settings);

    // Morph targets
    bool parseVertexMorphTarget(VertexMorphTarget& target, int32 morphSize);
    bool parseColorMorphTarget(ColorMorphTarget& target, int32 morphSize);
    bool parseTextureMorphTarget(TextureMorphTarget& target, int32 morphSize);
    bool parseGroupMorphTarget(GroupMorphTarget& target, int32 morphSize);

    // ========================================================================
    // HELPER FUNCTIONS
    // ========================================================================

    QString readOutforceString();
    QString readOutforceString32();
    QString readStaticString(int length);
    Vector2D readVector2D();
    Vector3D readVector3D();
    Vector2D_uint32 readVector2D_uint32();
    Vertex readVertex();

    // Safe template read function
    template<typename T>
    T read()
    {
        T value{};
        if (m_stream->atEnd())
        {
            return value;
        }
        m_stream->readRawData(reinterpret_cast<char*>(&value), sizeof(T));
        return value;
    }

    // Skip bytes
    void skip(int bytes)
    {
        m_stream->skipRawData(bytes);
    }

    // Read raw bytes
    QByteArray readBytes(int count)
    {
        QByteArray data(count, 0);
        m_stream->readRawData(data.data(), count);
        return data;
    }
};

} // namespace Opf

#endif // OPFPARSER_H
