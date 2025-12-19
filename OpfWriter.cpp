#include "OpfWriter.h"
#include <QFileInfo>
#include <QDir>
#include <QDateTime>
#include <QDebug>

namespace Opf {

OpfWriter::OpfWriter()
{
}

template<typename T>
void OpfWriter::writeValue(T value)
{
    m_stream << value;
}

void OpfWriter::writeBytes(const char* data, qint64 size)
{
    m_file.write(data, size);
}

void OpfWriter::writeBytes(const QByteArray& data)
{
    m_file.write(data);
}

void OpfWriter::writeOutforceString(const QString& str)
{
    QByteArray encoded = str.toLatin1();
    uint16 length = static_cast<uint16>(encoded.size());
    writeValue<uint16>(length);
    if (length > 0)
    {
        writeBytes(encoded);
    }
}

void OpfWriter::writeOutforceString32(const QString& str)
{
    QByteArray encoded = str.toLatin1();
    uint32 length = static_cast<uint32>(encoded.size());
    writeValue<uint32>(length);
    if (length > 0)
    {
        writeBytes(encoded);
    }
}

void OpfWriter::writeStaticString(const QString& str, int length)
{
    QByteArray encoded = str.toLatin1();
    QByteArray padded(length, '\0');
    int copyLen = qMin(encoded.size(), length);
    memcpy(padded.data(), encoded.data(), copyLen);
    writeBytes(padded);
}

void OpfWriter::writeVec3(const Vector3D& v)
{
    writeValue<fp32>(v.x);
    writeValue<fp32>(v.y);
    writeValue<fp32>(v.z);
}

void OpfWriter::writeVec2(const Vector2D& v)
{
    writeValue<fp32>(v.x);
    writeValue<fp32>(v.y);
}

void OpfWriter::writeColorValue(const ColorValue& c)
{
    writeValue<fp32>(c.red);
    writeValue<fp32>(c.green);
    writeValue<fp32>(c.blue);
    writeValue<fp32>(c.alpha);
}

bool OpfWriter::write(const QString& filename, const PackedProject& project)
{
    m_file.setFileName(filename);

    if (!m_file.open(QIODevice::WriteOnly))
    {
        m_lastError = QString("Cannot open file for writing: %1").arg(filename);
        return false;
    }

    m_stream.setDevice(&m_file);
    m_stream.setByteOrder(QDataStream::LittleEndian);
    m_stream.setFloatingPointPrecision(QDataStream::SinglePrecision);

    qDebug() << "Writing OPF file:" << filename;

    if (!writeProjectHeader(project))
    {
        m_file.close();
        return false;
    }

    if (!writeDependencies(project.dependencies))
    {
        m_file.close();
        return false;
    }

    if (!writeEventDescs(project.eventDescs))
    {
        m_file.close();
        return false;
    }

    if (!writeTextures(project.textures))
    {
        m_file.close();
        return false;
    }

    if (!writeMaterials(project.materials))
    {
        m_file.close();
        return false;
    }

    if (!writeObjects(project.objects))
    {
        m_file.close();
        return false;
    }

    m_file.close();

    qDebug() << "Successfully wrote OPF file:" << m_file.size() << "bytes";
    return true;
}

bool OpfWriter::writeBackup(const QString& originalFile, const PackedProject& project)
{
    QFileInfo fileInfo(originalFile);
    QString backupDir = fileInfo.absolutePath();
    QString baseName = fileInfo.completeBaseName();
    QString extension = fileInfo.suffix();

    QString timestamp = QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss");
    QString backupFile = QString("%1/%2_backup_%3.%4")
                             .arg(backupDir, baseName, timestamp, extension);

    if (QFile::exists(originalFile))
    {
        if (!QFile::copy(originalFile, backupFile))
        {
            m_lastError = QString("Failed to create backup: %1").arg(backupFile);
            return false;
        }
        qDebug() << "Created backup:" << backupFile;
    }

    return write(originalFile, project);
}

bool OpfWriter::writeProjectHeader(const PackedProject& project)
{
    writeStaticString(project.header, 23);
    writeValue<uint32>(project.version);
    writeOutforceString(project.projectName);
    writeOutforceString(project.author);
    writeOutforceString(project.email);
    writeOutforceString32(project.description);
    writeValue<uint16>(project.projectID);

    qDebug() << "Wrote header - Project:" << project.projectName << "Version:" << project.version;

    return m_stream.status() == QDataStream::Ok;
}

bool OpfWriter::writeDependencies(const QVector<QString>& dependencies)
{
    writeValue<uint32>(dependencies.size());

    for (const QString& dep : dependencies)
    {
        writeStaticString(dep, 260);
    }

    qDebug() << "Wrote" << dependencies.size() << "dependencies";
    return m_stream.status() == QDataStream::Ok;
}

bool OpfWriter::writeEventDescs(const QVector<EventDesc>& events)
{
    writeValue<uint32>(events.size());

    for (const EventDesc& evt : events)
    {
        writeValue<int32>(evt.trigger);
        writeOutforceString(evt.name);
    }

    qDebug() << "Wrote" << events.size() << "event descriptors";
    return m_stream.status() == QDataStream::Ok;
}

bool OpfWriter::writeTextures(const QVector<Texture>& textures)
{
    writeValue<uint32>(textures.size());

    for (const Texture& tex : textures)
    {
        writeTexture(tex);
    }

    qDebug() << "Wrote" << textures.size() << "textures";
    return m_stream.status() == QDataStream::Ok;
}

void OpfWriter::writeTexture(const Texture& texture)
{
    writeOutforceString(texture.name);
    writeOutforceString(texture.colorFile);
    writeOutforceString(texture.alphaFile);

    writeValue<uint32>(texture.size.x);
    writeValue<uint32>(texture.size.y);

    writeValue<int32>(texture.id);

    writeValue<int32>(texture.colorBitDepthInFile);
    writeValue<int32>(texture.colorBitDepthInMemory);
    writeValue<int32>(texture.alphaBitDepthInFile);
    writeValue<int32>(texture.alphaBitDepthInMemory);

    writeValue<uint8>(texture.colorDither ? 1 : 0);
    writeValue<uint8>(texture.alphaDither ? 1 : 0);
    writeValue<uint8>(texture.inverseAlpha ? 1 : 0);
    writeValue<uint8>(texture.betterQuality ? 1 : 0);
    writeValue<uint8>(texture.hasColorChannel ? 1 : 0);
    writeValue<uint8>(texture.hasAlphaChannel ? 1 : 0);
    writeValue<uint8>(texture.grayScale ? 1 : 0);
    writeValue<uint8>(texture.multiTexture ? 1 : 0);
    writeValue<int32>(texture.maxCap);

    writeValue<uint16>(texture.projectID);
    writeValue<uint32>(texture.version);

    writeValue<uint8>(texture.mipMap ? 1 : 0);
    writeValue<uint8>(static_cast<uint8>(texture.override));
    writeValue<uint8>(texture.excludeFromExport ? 1 : 0);

    writeValue<uint8>(static_cast<uint8>(texture.exportColorQuality));
    writeValue<uint8>(static_cast<uint8>(texture.exportAlphaQuality));
    writeValue<uint8>(texture.exportColorJPEGQuality);
    writeValue<uint8>(texture.exportAlphaJPEGQuality);

    if (texture.hasColorChannel)
    {
        writeBitmap(texture.colorBitmap);
    }

    if (texture.hasAlphaChannel)
    {
        writeBitmap(texture.alphaBitmap);
    }
}

void OpfWriter::writeBitmap(const Bitmap& bitmap)
{
    writeValue<int32>(bitmap.bitsPerPixel);
    writeValue<int32>(bitmap.width);
    writeValue<int32>(bitmap.height);
    writeValue<int32>(bitmap.lineSize);
    writeValue<uint8>(static_cast<uint8>(bitmap.bitmapType));
    writeValue<uint32>(bitmap.bitmapInfoHeaderSize);

    if (bitmap.bitmapInfoHeaderSize > 0)
    {
        writeValue<uint32>(bitmap.bitmapInfoHeader.size);
        writeValue<int32>(bitmap.bitmapInfoHeader.width);
        writeValue<int32>(bitmap.bitmapInfoHeader.height);
        writeValue<uint16>(bitmap.bitmapInfoHeader.planes);
        writeValue<uint16>(bitmap.bitmapInfoHeader.bitCount);
        writeValue<uint32>(bitmap.bitmapInfoHeader.compression);
        writeValue<uint32>(bitmap.bitmapInfoHeader.sizeImage);
        writeValue<int32>(bitmap.bitmapInfoHeader.xPelsPerMeter);
        writeValue<int32>(bitmap.bitmapInfoHeader.yPelsPerMeter);
        writeValue<uint32>(bitmap.bitmapInfoHeader.clrUsed);
        writeValue<uint32>(bitmap.bitmapInfoHeader.clrImportant);

        if (!bitmap.extraHeaderData.isEmpty())
        {
            writeBytes(bitmap.extraHeaderData);
        }
    }

    if (bitmap.bitmapType == EBitmapType::BMP)
    {
        writeBytes(bitmap.bitmapData);
    }

    else
    {
        writeValue<uint32>(bitmap.bitmapData.size());
        writeBytes(bitmap.bitmapData);
    }
}

bool OpfWriter::writeMaterials(const QVector<Material>& materials)
{
    writeValue<uint32>(materials.size());

    for (const Material& mat : materials)
    {
        writeMaterial(mat);
    }

    qDebug() << "Wrote" << materials.size() << "materials";
    return m_stream.status() == QDataStream::Ok;
}

void OpfWriter::writeMaterial(const Material& material)
{
    writeOutforceString(material.name);
    writeValue<uint16>(material.projectID);
    writeValue<int32>(material.id);
    writeValue<uint32>(material.version);
    writeValue<uint8>(material.doubleSided ? 1 : 0);
    writeValue<uint8>(material.enabledLighting ? 1 : 0);
    writeValue<uint8>(static_cast<uint8>(material.override));
    writeValue<uint8>(material.excludeFromExport ? 1 : 0);

    writeValue<uint32>(material.renderPasses1Stage.size());
    for (const RenderPass1Stage& pass : material.renderPasses1Stage)
    {
        writeRenderPassSettings(pass.settings);
        writeRenderPassStage(pass.stages[0]);
    }

    writeValue<uint32>(material.renderPasses2Stage.size());
    for (const RenderPass2Stage& pass : material.renderPasses2Stage)
    {
        writeRenderPassSettings(pass.settings);
        writeRenderPassStage(pass.stages[0]);
        writeRenderPassStage(pass.stages[1]);
    }

    writeValue<uint32>(material.renderPasses3Stage.size());
    for (const RenderPass3Stage& pass : material.renderPasses3Stage)
    {
        writeRenderPassSettings(pass.settings);
        writeRenderPassStage(pass.stages[0]);
        writeRenderPassStage(pass.stages[1]);
        writeRenderPassStage(pass.stages[2]);
    }
}

void OpfWriter::writeRenderPassSettings(const RenderPassSettings& s)
{
    writeValue<uint8>(s.antialias ? 1 : 0);
    writeValue<uint8>(s.perspectiveCorrection ? 1 : 0);
    writeValue<uint8>(static_cast<uint8>(s.fillMode));
    writeValue<uint8>(static_cast<uint8>(s.shadeMode));
    writeValue<uint16>(s.linePattern_RepeatFactor);
    writeValue<uint16>(s.linePattern_LinePattern);
    writeValue<uint8>(static_cast<uint8>(s.pixelOperation));
    writeValue<uint8>(s.writeToZBuffer ? 1 : 0);
    writeValue<uint8>(s.testForAlphaBlending ? 1 : 0);
    writeValue<uint8>(s.alphaReference);
    writeValue<uint8>(static_cast<uint8>(s.alphaFunction));
    writeValue<uint8>(s.drawLastPixel ? 1 : 0);
    writeValue<uint8>(static_cast<uint8>(s.sourceBlendMode));
    writeValue<uint8>(static_cast<uint8>(s.destBlendMode));
    writeValue<uint8>(static_cast<uint8>(s.zCompareFunction));
    writeValue<uint8>(s.alphaBlending ? 1 : 0);
    writeValue<uint8>(s.affectedByFog ? 1 : 0);
    writeValue<uint8>(s.specularEnable ? 1 : 0);
    writeValue<uint8>(s.stippled ? 1 : 0);
    writeValue<uint8>(s.edgeAntialias ? 1 : 0);
    writeValue<uint8>(s.colorKeying ? 1 : 0);
    writeValue<uint8>(s.zBias);
    writeValue<uint32>(s.textureFactorColor);

    for (int i = 0; i < 32; ++i)
    {
        writeValue<uint32>(s.stipplePattern[i]);
    }

    writeValue<uint8>(s.useVertexColorWhenLighting ? 1 : 0);

    writeMaterial7(s.materialDescription);
}

void OpfWriter::writeMaterial7(const Material7& mat)
{
    writeColorValue(mat.diffuse);
    writeColorValue(mat.ambient);
    writeColorValue(mat.specular);
    writeColorValue(mat.emissive);
    writeValue<fp32>(mat.power);
}

void OpfWriter::writeRenderPassStage(const RenderPassStage& stage)
{
    writeValue<uint8>(static_cast<uint8>(stage.colorArg1));
    writeValue<uint8>(static_cast<uint8>(stage.colorArg2));
    writeValue<uint8>(static_cast<uint8>(stage.colorOp));
    writeValue<uint8>(static_cast<uint8>(stage.alphaArg1));
    writeValue<uint8>(static_cast<uint8>(stage.alphaArg2));
    writeValue<uint8>(static_cast<uint8>(stage.alphaOp));
    writeValue<uint8>(stage.textureCoordinateIndex);
    writeValue<uint8>(static_cast<uint8>(stage.textureAddressU));
    writeValue<uint8>(static_cast<uint8>(stage.textureAddressV));
    writeValue<uint32>(stage.borderColor);
    writeValue<uint8>(static_cast<uint8>(stage.maxTextureMagnificationFilter));
    writeValue<uint8>(static_cast<uint8>(stage.maxTextureMinificationFilter));
    writeValue<uint8>(static_cast<uint8>(stage.maxTextureMipmapFilter));
    writeValue<fp32>(stage.mipmapLODBias);
    writeValue<uint8>(stage.minMipmapLevel);
    writeValue<uint8>(stage.maxAnisotropy);
    writeValue<uint8>(static_cast<uint8>(stage.wrapping));
    writeValue<uint16>(stage.textureProjectID);
    writeValue<int32>(stage.textureID);
    writeValue<int32>(stage.textureStageInTexture);
}

bool OpfWriter::writeObjects(const QVector<Object*>& objects)
{
    writeValue<uint32>(objects.size());

    for (const Object* obj : objects)
    {
        if (obj)
        {
            writeObject(*obj);
        }
    }

    qDebug() << "Wrote" << objects.size() << "objects";
    return m_stream.status() == QDataStream::Ok;
}

void OpfWriter::writeObject(const Object& obj)
{
    writeOutforceString(obj.className);

    writeValue<uint8>(obj.isUnknown ? 1 : 0);
    writeValue<uint16>(obj.projectID);
    writeValue<int32>(obj.uniqueID);
    writeValue<uint8>(static_cast<uint8>(obj.override));
    writeValue<uint8>(obj.excludeFromExport ? 1 : 0);

    writeOutforceString(obj.name);

    writeVec3(obj.position);
    writeVec3(obj.rotation);
    writeVec3(obj.scaling);

    writeValue<uint32>(obj.version);
    writeValue<uint8>(obj.isDisabled ? 1 : 0);
    writeValue<uint8>(obj.disableTree ? 1 : 0);
    writeValue<uint8>(obj.isBillboard ? 1 : 0);

    writeObjectTemplate(obj.objectTemplate);

    writeValue<uint8>(obj.hasLight ? 1 : 0);

    if (obj.hasLight)
    {
        writeLight(obj.light);
    }

    writeValue<uint8>(0);

    writeCustomSettings(obj.customSettings);

    writeValue<uint32>(obj.children.size());
    for (const Object* child : obj.children)
    {
        if (child)
        {
            writeObject(*child);
        }
    }
}

void OpfWriter::writeObjectTemplate(const ObjectTemplate& templ)
{
    writeVec3(templ.physicalScaling);
    writeVec3(templ.physicalPosition);
    writeVec3(templ.physicalRotation);
    writeVec3(templ.lastPhysicalScaling);
    writeVec3(templ.lastPhysicalPosition);
    writeVec3(templ.lastPhysicalRotation);

    writeValue<uint32>(templ.faceBuffers.size());
    for (const Mesh& mesh : templ.faceBuffers)
    {
        writeMesh(mesh);
    }
}

void OpfWriter::writeMesh(const Mesh& mesh)
{
    writeValue<int32>(static_cast<int32>(mesh.vertexFormat));
    writeValue<uint16>(mesh.materialProjectID);
    writeValue<int32>(mesh.materialID);

    writeOutforceString(mesh.name);

    writeValue<uint32>(static_cast<uint32>(mesh.textureWrapType));
    writeVec3(mesh.textureOrigin);
    writeVec3(mesh.textureRotation);
    writeVec2(mesh.textureScale);
    writeVec2(mesh.texturePosition);

    writeVec3(mesh.position);
    writeValue<fp32>(mesh.boundRadius);

    writeValue<int32>(mesh.morphSize);
    writeValue<int32>(mesh.vertexMorphTargets.size());
    writeValue<int32>(mesh.colorMorphTargets.size());
    writeValue<int32>(mesh.textureMorphTargets.size());

    for (const VertexMorphTarget& vmt : mesh.vertexMorphTargets)
    {
        for (const Vertex& v : vmt.vertices)
        {
            writeVec3(v.position);
            writeVec3(v.normal);
        }
        writeOutforceString(vmt.name);
    }

    for (const ColorMorphTarget& cmt : mesh.colorMorphTargets)
    {
        for (Color c : cmt.colors)
        {
            writeValue<uint32>(c);
        }
    }

    for (const TextureMorphTarget& tmt : mesh.textureMorphTargets)
    {
        for (const Vector2D& tc : tmt.textureCoordinates)
        {
            writeVec2(tc);
        }
    }

    writeValue<int32>(mesh.srcVertex);
    writeValue<int32>(mesh.srcColor);
    writeValue<int32>(mesh.srcTexture[0]);
    writeValue<int32>(mesh.srcTexture[1]);
    writeValue<int32>(mesh.srcTexture[2]);
    writeValue<int32>(mesh.dstVertex);
    writeValue<int32>(mesh.dstColor);
    writeValue<int32>(mesh.dstTexture[0]);
    writeValue<int32>(mesh.dstTexture[1]);
    writeValue<int32>(mesh.dstTexture[2]);
    writeValue<fp32>(mesh.amountVertex);
    writeValue<fp32>(mesh.amountColor);
    writeValue<fp32>(mesh.amountTexture[0]);
    writeValue<fp32>(mesh.amountTexture[1]);
    writeValue<fp32>(mesh.amountTexture[2]);
    writeValue<uint8>(mesh.dstEnvironment[0] ? 1 : 0);
    writeValue<uint8>(mesh.dstEnvironment[1] ? 1 : 0);
    writeValue<uint8>(mesh.dstEnvironment[2] ? 1 : 0);

    writeValue<int32>(static_cast<int32>(mesh.bufferType));
    writeValue<int32>(mesh.numFaces);

    for (uint16 idx : mesh.indices)
    {
        writeValue<uint16>(idx);
    }

    writeValue<int32>(mesh.groupMorphTargets.size());
    for (const GroupMorphTarget& gmt : mesh.groupMorphTargets)
    {
        for (uint8 g : gmt.groups)
        {
            writeValue<uint8>(g);
        }
    }
}

void OpfWriter::writeLight(const Light& light)
{
    writeColorValue(light.diffuse);
    writeColorValue(light.ambient);
    writeColorValue(light.specular);

    writeValue<uint32>(static_cast<uint32>(light.lightType));

    writeValue<fp32>(light.attenuation0);
    writeValue<fp32>(light.attenuation1);
    writeValue<fp32>(light.attenuation2);

    writeValue<fp32>(light.falloff);
    writeValue<fp32>(light.phi);
    writeValue<fp32>(light.range);
    writeValue<fp32>(light.theta);

    writeValue<uint8>(light.active ? 1 : 0);
    writeVec3(light.position);
    writeVec3(light.rotation);
}

void OpfWriter::writeCustomSettings(const QVector<CustomSetting>& settings)
{
    writeValue<uint32>(settings.size());

    for (const CustomSetting& setting : settings)
    {
        writeOutforceString(setting.name);
        writeOutforceString(setting.value);
    }
}

} // namespace Opf
