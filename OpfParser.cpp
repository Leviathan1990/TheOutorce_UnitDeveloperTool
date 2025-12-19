#include "OpfParser.h"
#include <QDebug>

namespace Opf {

OpfParser::OpfParser() : m_stream(nullptr)
{
}

bool OpfParser::parse(const QString& filename, PackedProject& project)
{
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly))
    {
        m_lastError = QString("Cannot open file: %1").arg(filename);
        return false;
    }

    qint64 fileSize = file.size();
    if (fileSize < 100)
    {
        m_lastError = "File too small to be a valid OPF file";
        file.close();
        return false;
    }

    if (fileSize > 500 * 1024 * 1024)
    {
        m_lastError = "File too large (>500MB). Possibly corrupted.";
        file.close();
        return false;
    }

    QDataStream stream(&file);
    stream.setByteOrder(QDataStream::LittleEndian);
    stream.setFloatingPointPrecision(QDataStream::SinglePrecision);
    m_stream = &stream;

    try {
        // 1. Parse header
        if (!parseHeader(project))
        {
            file.close();
            return false;
        }

        qDebug() << "Project:" << project.projectName;
        qDebug() << "Version:" << project.version;
        qDebug() << "Author:" << project.author;

        // 2. Parse dependencies
        if (!parseDependencies(project))
        {
            file.close();
            return false;
        }

        // 3. Parse event descriptors
        if (!parseEventDescs(project))
        {
            file.close();
            return false;
        }

        // 4. Parse textures
        if (!parseTextures(project))
        {
            file.close();
            return false;
        }

        // 5. Parse materials
        if (!parseMaterials(project))
        {
            file.close();
            return false;
        }

        // 6. Parse objects
        if (!parseObjects(project))
        {
            file.close();
            return false;
        }

        file.close();

        qDebug() << "Successfully parsed OPF file:" << filename;
        qDebug() << "  Dependencies:" << project.dependencies.size();
        qDebug() << "  Events:" << project.eventDescs.size();
        qDebug() << "  Textures:" << project.textures.size();
        qDebug() << "  Materials:" << project.materials.size();
        qDebug() << "  Objects:" << project.objects.size();

        return true;

    }

    catch (const std::exception& e)
    {
        m_lastError = QString("Exception during parsing: %1").arg(e.what());
        file.close();
        return false;
    }
    catch (...)
    {
        m_lastError = "Unknown exception during parsing";
        file.close();
        return false;
    }
}

// ============================================================================
// HEADER PARSING
// ============================================================================

bool OpfParser::parseHeader(PackedProject& project)
{
    // Read header string (23 bytes)
    project.header = readStaticString(23);
    if (project.header != "Outforce Packed Content")
    {
        m_lastError = QString("Invalid file header: '%1'").arg(project.header);
        return false;
    }

    // Version
    project.version = read<uint32>();
    if (project.version != 29)
    {
        m_lastError = QString("Unsupported version: %1 (expected 29)").arg(project.version);
        return false;
    }

    // Project info
    project.projectName = readOutforceString();
    project.author = readOutforceString();
    project.email = readOutforceString();

    // Description uses uint32 length
    project.description = readOutforceString32();

    // Project ID
    project.projectID = read<uint16>();

    return true;
}

// ============================================================================
// DEPENDENCIES PARSING
// ============================================================================

bool OpfParser::parseDependencies(PackedProject& project)
{
    uint32 count = read<uint32>();
    if (count > 1000)
    {
        m_lastError = QString("Invalid dependency count: %1").arg(count);
        return false;
    }

    project.dependencies.reserve(count);
    for (uint32 i = 0; i < count; i++)
    {
        QString dep = readStaticString(260);
        project.dependencies.append(dep);
    }

    qDebug() << "  Loaded" << count << "dependencies";
    return true;
}

// ============================================================================
// EVENT DESCRIPTORS PARSING
// ============================================================================

bool OpfParser::parseEventDescs(PackedProject& project)
{
    uint32 count = read<uint32>();
    if (count > 10000)
    {
        m_lastError = QString("Invalid event count: %1").arg(count);
        return false;
    }

    project.eventDescs.reserve(count);
    for (uint32 i = 0; i < count; i++)
    {
        EventDesc event;
        event.trigger = read<int32>();
        event.name = readOutforceString();
        project.eventDescs.append(event);
    }

    qDebug() << "  Loaded" << count << "event descriptors";
    return true;
}

// ============================================================================
// TEXTURES PARSING
// ============================================================================

bool OpfParser::parseTextures(PackedProject& project)
{
    uint32 count = read<uint32>();
    if (count > 10000)
    {
        m_lastError = QString("Invalid texture count: %1").arg(count);
        return false;
    }

    qDebug() << "Parsing" << count << "textures...";
    project.textures.reserve(count);

    for (uint32 i = 0; i < count; i++)
    {
        Texture texture;
        if (!parseTexture(texture))
        {
            qWarning() << "Failed to parse texture" << (i + 1) << "/" << count;
            continue;
        }
        project.textures.append(texture);
    }

    return true;
}

bool OpfParser::parseTexture(Texture& texture)
{
    // Basic info
    texture.name = readOutforceString();
    texture.colorFile = readOutforceString();
    texture.alphaFile = readOutforceString();

    // Size
    texture.size = readVector2D_uint32();
    texture.width = texture.size.x;
    texture.height = texture.size.y;

    // IDs
    texture.id = read<int32>();

    // Bit depths
    texture.colorBitDepthInFile = read<int32>();
    texture.colorBitDepthInMemory = read<int32>();
    texture.alphaBitDepthInFile = read<int32>();
    texture.alphaBitDepthInMemory = read<int32>();

    // Flags
    texture.colorDither = read<uint8>();
    texture.alphaDither = read<uint8>();
    texture.inverseAlpha = read<uint8>();
    texture.betterQuality = read<uint8>();
    texture.hasColorChannel = read<uint8>();
    texture.hasAlphaChannel = read<uint8>();
    texture.grayScale = read<uint8>();
    texture.multiTexture = read<uint8>();
    texture.maxCap = read<int32>();

    // Project ID and version
    texture.projectID = read<uint16>();
    texture.version = read<uint32>();

    // More flags
    texture.mipMap = read<uint8>();
    texture.override = static_cast<EOverride>(read<uint8>());
    texture.excludeFromExport = read<uint8>();

    // Export quality settings
    texture.exportColorQuality = static_cast<EBitmapType>(read<uint8>());
    texture.exportAlphaQuality = static_cast<EBitmapType>(read<uint8>());
    texture.exportColorJPEGQuality = read<uint8>();
    texture.exportAlphaJPEGQuality = read<uint8>();

    // Color bitmap
    if (texture.hasColorChannel)
    {
        if (!parseBitmap(texture.colorBitmap))
        {
            return false;
        }
        // Copy for easy access
        texture.colorBitsPerPixel = texture.colorBitmap.bitsPerPixel;
        texture.colorBitmapType = static_cast<uint8>(texture.colorBitmap.bitmapType);
        texture.colorData = texture.colorBitmap.bitmapData;
    }

    // Alpha bitmap
    if (texture.hasAlphaChannel)
    {
        if (!parseBitmap(texture.alphaBitmap))
        {
            return false;
        }
        // Copy for easy access
        texture.alphaBitsPerPixel = texture.alphaBitmap.bitsPerPixel;
        texture.alphaBitmapType = static_cast<uint8>(texture.alphaBitmap.bitmapType);
        texture.alphaData = texture.alphaBitmap.bitmapData;
    }

    return true;
}

bool OpfParser::parseBitmap(Bitmap& bitmap)
{
    bitmap.bitsPerPixel = read<int32>();
    bitmap.width = read<int32>();
    bitmap.height = read<int32>();
    bitmap.lineSize = read<int32>();
    bitmap.bitmapType = static_cast<EBitmapType>(read<uint8>());
    bitmap.bitmapInfoHeaderSize = read<uint32>();

    // Parse bitmap info header
    qint64 startPos = m_stream->device()->pos();
    if (!parseBitmapInfoHeader(bitmap.bitmapInfoHeader, bitmap.bitmapInfoHeaderSize))
    {
        return false;
    }

    // Read any extra header data
    qint64 currentPos = m_stream->device()->pos();
    qint64 expectedEndPos = startPos + bitmap.bitmapInfoHeaderSize;
    if (currentPos < expectedEndPos)
    {
        int extraBytes = expectedEndPos - currentPos;
        bitmap.extraHeaderData = readBytes(extraBytes);
    }

    // Read bitmap data
    uint32 dataSize = 0;
    if (bitmap.bitmapType == EBitmapType::BMP)
    {
        dataSize = bitmap.bitmapInfoHeader.sizeImage;
        if (dataSize == 0)
        {
            dataSize = bitmap.lineSize * bitmap.height;
        }
    }

    else
    {
        // JPEG - size is stored separately
        dataSize = read<uint32>();
    }

    bitmap.bitmapData = readBytes(dataSize);

    return true;
}

bool OpfParser::parseBitmapInfoHeader(BitmapInfoHeader& header, uint32 headerSize)
{
    qint64 startPos = m_stream->device()->pos();
    qint64 endPos = startPos + headerSize;

    if (m_stream->device()->pos() < endPos) header.size = read<uint32>();
    if (m_stream->device()->pos() < endPos) header.width = read<int32>();
    if (m_stream->device()->pos() < endPos) header.height = read<int32>();
    if (m_stream->device()->pos() < endPos) header.planes = read<uint16>();
    if (m_stream->device()->pos() < endPos) header.bitCount = read<uint16>();
    if (m_stream->device()->pos() < endPos) header.compression = read<uint32>();
    if (m_stream->device()->pos() < endPos) header.sizeImage = read<uint32>();
    if (m_stream->device()->pos() < endPos) header.xPelsPerMeter = read<int32>();
    if (m_stream->device()->pos() < endPos) header.yPelsPerMeter = read<int32>();
    if (m_stream->device()->pos() < endPos) header.clrUsed = read<uint32>();
    if (m_stream->device()->pos() < endPos) header.clrImportant = read<uint32>();

    return true;
}

// ============================================================================
// MATERIALS PARSING
// ============================================================================

bool OpfParser::parseMaterials(PackedProject& project)
{
    uint32 count = read<uint32>();
    if (count > 10000)
    {
        m_lastError = QString("Invalid material count: %1").arg(count);
        return false;
    }

    qDebug() << "Parsing" << count << "materials...";
    project.materials.reserve(count);

    for (uint32 i = 0; i < count; i++)
    {
        Material material;
        if (!parseMaterial(material))
        {
            qWarning() << "Failed to parse material" << (i + 1) << "/" << count;
            continue;
        }
        project.materials.append(material);
    }

    return true;
}

bool OpfParser::parseMaterial(Material& material)
{
    material.name = readOutforceString();
    material.projectID = read<uint16>();
    material.id = read<int32>();
    material.version = read<uint32>();
    material.doubleSided = read<uint8>();
    material.enabledLighting = read<uint8>();
    material.override = static_cast<EOverride>(read<uint8>());
    material.excludeFromExport = read<uint8>();

    material.textureID = -1;  // Default

    // Parse 1-stage render passes
    uint32 count1 = read<uint32>();
    material.renderPasses1Stage.reserve(count1);
    for (uint32 i = 0; i < count1; i++)
    {
        RenderPass1Stage pass;
        if (!parseRenderPassSettings(pass.settings)) return false;
        if (!parseRenderPassStage(pass.stages[0])) return false;

        // Capture first texture ID
        if (material.textureID == -1 && pass.stages[0].textureID != 0)
        {
            material.textureID = pass.stages[0].textureID;
        }

        material.renderPasses1Stage.append(pass);
    }

    // Parse 2-stage render passes
    uint32 count2 = read<uint32>();
    material.renderPasses2Stage.reserve(count2);
    for (uint32 i = 0; i < count2; i++)
    {
        RenderPass2Stage pass;
        if (!parseRenderPassSettings(pass.settings)) return false;
        if (!parseRenderPassStage(pass.stages[0])) return false;
        if (!parseRenderPassStage(pass.stages[1])) return false;

        if (material.textureID == -1 && pass.stages[0].textureID != 0)
        {
            material.textureID = pass.stages[0].textureID;
        }

        material.renderPasses2Stage.append(pass);
    }

    // Parse 3-stage render passes
    uint32 count3 = read<uint32>();
    material.renderPasses3Stage.reserve(count3);
    for (uint32 i = 0; i < count3; i++)
    {
        RenderPass3Stage pass;
        if (!parseRenderPassSettings(pass.settings)) return false;
        if (!parseRenderPassStage(pass.stages[0])) return false;
        if (!parseRenderPassStage(pass.stages[1])) return false;
        if (!parseRenderPassStage(pass.stages[2])) return false;

        if (material.textureID == -1 && pass.stages[0].textureID != 0)
        {
            material.textureID = pass.stages[0].textureID;
        }

        material.renderPasses3Stage.append(pass);
    }

    return true;
}

bool OpfParser::parseRenderPassSettings(RenderPassSettings& settings)
{
    settings.antialias = read<uint8>();
    settings.perspectiveCorrection = read<uint8>();
    settings.fillMode = static_cast<EFillMode>(read<uint8>());
    settings.shadeMode = static_cast<EShadeMode>(read<uint8>());
    settings.linePattern_RepeatFactor = read<uint16>();
    settings.linePattern_LinePattern = read<uint16>();
    settings.pixelOperation = static_cast<EPixelOperation>(read<uint8>());
    settings.writeToZBuffer = read<uint8>();
    settings.testForAlphaBlending = read<uint8>();
    settings.alphaReference = read<uint8>();
    settings.alphaFunction = static_cast<ECmpFunction>(read<uint8>());
    settings.drawLastPixel = read<uint8>();
    settings.sourceBlendMode = static_cast<EBlend>(read<uint8>());
    settings.destBlendMode = static_cast<EBlend>(read<uint8>());
    settings.zCompareFunction = static_cast<ECmpFunction>(read<uint8>());
    settings.alphaBlending = read<uint8>();
    settings.affectedByFog = read<uint8>();
    settings.specularEnable = read<uint8>();
    settings.stippled = read<uint8>();
    settings.edgeAntialias = read<uint8>();
    settings.colorKeying = read<uint8>();
    settings.zBias = read<uint8>();
    settings.textureFactorColor = read<uint32>();

    // Stipple pattern (32 x uint32)
    for (int i = 0; i < 32; i++)
    {
        settings.stipplePattern[i] = read<uint32>();
    }

    settings.useVertexColorWhenLighting = read<uint8>();

    // Material7
    if (!parseMaterial7(settings.materialDescription))
    {
        return false;
    }

    return true;
}

bool OpfParser::parseMaterial7(Material7& mat)
{
    if (!parseColorValue(mat.diffuse)) return false;
    if (!parseColorValue(mat.ambient)) return false;
    if (!parseColorValue(mat.specular)) return false;
    if (!parseColorValue(mat.emissive)) return false;
    mat.power = read<fp32>();
    return true;
}

bool OpfParser::parseColorValue(ColorValue& color)
{
    color.red = read<fp32>();
    color.green = read<fp32>();
    color.blue = read<fp32>();
    color.alpha = read<fp32>();
    return true;
}

bool OpfParser::parseRenderPassStage(RenderPassStage& stage)
{
    stage.colorArg1 = static_cast<ETextureStageArg>(read<uint8>());
    stage.colorArg2 = static_cast<ETextureStageArg>(read<uint8>());
    stage.colorOp = static_cast<ETextureStageOperation>(read<uint8>());
    stage.alphaArg1 = static_cast<ETextureStageArg>(read<uint8>());
    stage.alphaArg2 = static_cast<ETextureStageArg>(read<uint8>());
    stage.alphaOp = static_cast<ETextureStageOperation>(read<uint8>());
    stage.textureCoordinateIndex = read<uint8>();
    stage.textureAddressU = static_cast<ETextureAddress>(read<uint8>());
    stage.textureAddressV = static_cast<ETextureAddress>(read<uint8>());
    stage.borderColor = read<uint32>();
    stage.maxTextureMagnificationFilter = static_cast<ETextureMagnificationFilter>(read<uint8>());
    stage.maxTextureMinificationFilter = static_cast<ETextureMinificationFilter>(read<uint8>());
    stage.maxTextureMipmapFilter = static_cast<ETextureMipmapFilter>(read<uint8>());
    stage.mipmapLODBias = read<fp32>();
    stage.minMipmapLevel = read<uint8>();
    stage.maxAnisotropy = read<uint8>();
    stage.wrapping = static_cast<EWrapFlag>(read<uint8>());
    stage.textureProjectID = read<uint16>();
    stage.textureID = read<int32>();
    stage.textureStageInTexture = read<int32>();

    return true;
}

// ============================================================================
// OBJECTS PARSING
// ============================================================================

bool OpfParser::parseObjects(PackedProject& project)
{
    uint32 count = read<uint32>();
    if (count > 10000)
    {
        m_lastError = QString("Invalid object count: %1").arg(count);
        return false;
    }

    qDebug() << "Parsing" << count << "objects...";
    project.objects.reserve(count);

    for (uint32 i = 0; i < count; i++)
    {
        Object* object = new Object();
        if (!parseObject(*object))
        {
            qWarning() << "Failed to parse object" << (i + 1) << "/" << count;
            delete object;
            continue;
        }
        project.objects.append(object);
    }

    return true;
}

bool OpfParser::parseObject(Object& object)
{
    try {
        // Class name
        object.className = readOutforceString();

        // Flags and IDs
        object.isUnknown = read<uint8>();
        object.projectID = read<uint16>();
        object.uniqueID = read<int32>();
        object.override = static_cast<EOverride>(read<uint8>());
        object.excludeFromExport = read<uint8>();

        // Name
        object.name = readOutforceString();

        // Validate name
        if (object.name.contains('\0') && object.name.length() > 50)
        {
            qWarning() << "Invalid object name detected";
            return false;
        }

        // Transform
        object.position = readVector3D();
        object.rotation = readVector3D();
        object.scaling = readVector3D();

        // Version and flags
        object.version = read<uint32>();
        object.isDisabled = read<uint8>();
        object.disableTree = read<uint8>();
        object.isBillboard = read<uint8>();

        // Object template (contains meshes)
        if (!parseObjectTemplate(object.objectTemplate))
        {
            return false;
        }

        // Light
        object.hasLight = read<uint8>();
        if (object.hasLight)
        {
            if (!parseLight(object.light))
            {
                return false;
            }
        }

        // Reserved byte (must be 0)
        uint8 reserved = read<uint8>();
        if (reserved != 0)
        {
            qWarning() << "Reserved byte is not 0:" << reserved;
        }

        // Custom settings (THIS IS THE KEY FOR GAME LOGIC!)
        if (!parseCustomSettings(object.customSettings))
        {
            return false;
        }

        // Debug: print custom settings
        if (!object.customSettings.isEmpty())
        {
            qDebug() << "  Object" << object.name << "has" << object.customSettings.size() << "custom settings:";
            for (const auto& setting : object.customSettings)
            {
                qDebug() << "    " << setting.name << "=" << setting.value;
            }
        }

        // Children
        uint32 childCount = read<uint32>();
        object.children.reserve(childCount);
        for (uint32 i = 0; i < childCount; i++)
        {
            Object* child = new Object();
            if (!parseObject(*child))
            {
                delete child;
                return false;
            }
            object.children.append(child);
        }

        return true;

    }

    catch (...)
    {
        qWarning() << "Exception while parsing object";
        return false;
    }
}

bool OpfParser::parseObjectTemplate(ObjectTemplate& templ)
{
    // Physical transforms
    templ.physicalScaling = readVector3D();
    templ.physicalPosition = readVector3D();
    templ.physicalRotation = readVector3D();
    templ.lastPhysicalScaling = readVector3D();
    templ.lastPhysicalPosition = readVector3D();
    templ.lastPhysicalRotation = readVector3D();

    // Face buffers (meshes)
    uint32 faceBufferCount = read<uint32>();
    if (faceBufferCount > 100)
    {
        qWarning() << "Suspicious face buffer count:" << faceBufferCount;
        return false;
    }

    templ.faceBuffers.reserve(faceBufferCount);
    for (uint32 i = 0; i < faceBufferCount; i++)
    {
        Mesh mesh;
        if (!parseFaceBuffer(mesh))
        {
            return false;
        }
        templ.faceBuffers.append(mesh);
    }

    return true;
}

bool OpfParser::parseFaceBuffer(Mesh& mesh)
{
    // Vertex format
    mesh.vertexFormat = static_cast<EVertexFormat>(read<int32>());

    // Material
    mesh.materialProjectID = read<uint16>();
    mesh.materialID = read<int32>();

    // Name
    mesh.name = readOutforceString();

    // Texture wrapping
    mesh.textureWrapType = static_cast<ETextureWrapType>(read<uint32>());
    mesh.textureOrigin = readVector3D();
    mesh.textureRotation = readVector3D();
    mesh.textureScale = readVector2D();
    mesh.texturePosition = readVector2D();

    // Position and bounds
    mesh.position = readVector3D();
    mesh.boundRadius = read<fp32>();

    // Morph data
    mesh.morphSize = read<int32>();
    int32 numVertexMorphs = read<int32>();
    int32 numColorMorphs = read<int32>();
    int32 numTextureMorphs = read<int32>();

    // Parse vertex morph targets
    mesh.vertexMorphTargets.reserve(numVertexMorphs);
    for (int32 i = 0; i < numVertexMorphs; i++)
    {
        VertexMorphTarget target;
        if (!parseVertexMorphTarget(target, mesh.morphSize))
        {
            return false;
        }

        // Store first morph as main vertices
        if (i == 0 && !target.vertices.isEmpty())
        {
            mesh.vertices = target.vertices;
        }

        mesh.vertexMorphTargets.append(target);
    }

    // Parse color morph targets
    mesh.colorMorphTargets.reserve(numColorMorphs);
    for (int32 i = 0; i < numColorMorphs; i++)
    {
        ColorMorphTarget target;
        if (!parseColorMorphTarget(target, mesh.morphSize))
        {
            return false;
        }
        mesh.colorMorphTargets.append(target);
    }

    // Parse texture morph targets
    mesh.textureMorphTargets.reserve(numTextureMorphs);
    for (int32 i = 0; i < numTextureMorphs; i++)
    {
        TextureMorphTarget target;
        if (!parseTextureMorphTarget(target, mesh.morphSize))
        {
            return false;
        }

        // Store first morph as main texture coordinates
        if (i == 0 && !target.textureCoordinates.isEmpty())
        {
            for (int j = 0; j < mesh.vertices.size() && j < target.textureCoordinates.size(); j++)
            {
                mesh.vertices[j].texCoord = target.textureCoordinates[j];
            }
        }

        mesh.textureMorphTargets.append(target);
    }

    // Morph blend parameters
    mesh.srcVertex = read<int32>();
    mesh.srcColor = read<int32>();
    for (int i = 0; i < 3; i++) mesh.srcTexture[i] = read<int32>();

    mesh.dstVertex = read<int32>();
    mesh.dstColor = read<int32>();
    for (int i = 0; i < 3; i++) mesh.dstTexture[i] = read<int32>();

    mesh.amountVertex = read<fp32>();
    mesh.amountColor = read<fp32>();
    for (int i = 0; i < 3; i++) mesh.amountTexture[i] = read<fp32>();

    for (int i = 0; i < 3; i++) mesh.dstEnvironment[i] = read<uint8>();

    // Face buffer type and count
    mesh.bufferType = static_cast<EBufferType>(read<int32>());
    mesh.numFaces = read<int32>();
    mesh.primitiveType = static_cast<int>(mesh.bufferType);

    if (mesh.numFaces < 0 || mesh.numFaces > 100000)
    {
        qWarning() << "Invalid face count:" << mesh.numFaces;
        return false;
    }

    // Read indices
    int indexCount = mesh.numFaces * static_cast<int>(mesh.bufferType);
    mesh.indices.reserve(indexCount);
    for (int i = 0; i < indexCount; i++)
    {
        mesh.indices.append(read<uint16>());
    }

    // Group morph targets
    int32 numGroupMorphs = read<int32>();
    mesh.groupMorphTargets.reserve(numGroupMorphs);
    for (int32 i = 0; i < numGroupMorphs; i++)
    {
        GroupMorphTarget target;
        if (!parseGroupMorphTarget(target, mesh.morphSize))
        {
            return false;
        }
        mesh.groupMorphTargets.append(target);
    }

    return true;
}

bool OpfParser::parseVertexMorphTarget(VertexMorphTarget& target, int32 morphSize)
{
    target.vertices.reserve(morphSize);
    for (int32 i = 0; i < morphSize; i++)
    {
        Vertex v;
        v.position = readVector3D();
        v.normal = readVector3D();
        target.vertices.append(v);
    }
    target.name = readOutforceString();
    return true;
}

bool OpfParser::parseColorMorphTarget(ColorMorphTarget& target, int32 morphSize)
{
    target.colors.reserve(morphSize);
    for (int32 i = 0; i < morphSize; i++)
    {
        target.colors.append(read<uint32>());
    }
    return true;
}

bool OpfParser::parseTextureMorphTarget(TextureMorphTarget& target, int32 morphSize)
{
    target.textureCoordinates.reserve(morphSize);
    for (int32 i = 0; i < morphSize; i++)
    {
        target.textureCoordinates.append(readVector2D());
    }
    return true;
}

bool OpfParser::parseGroupMorphTarget(GroupMorphTarget& target, int32 morphSize)
{
    target.groups.reserve(morphSize);
    for (int32 i = 0; i < morphSize; i++)
    {
        target.groups.append(read<uint8>());
    }
    return true;
}

bool OpfParser::parseLight(Light& light)
{
    if (!parseColorValue(light.diffuse)) return false;
    if (!parseColorValue(light.ambient)) return false;
    if (!parseColorValue(light.specular)) return false;

    light.lightType = static_cast<ELightType>(read<uint32>());

    light.attenuation0 = read<fp32>();
    light.attenuation1 = read<fp32>();
    light.attenuation2 = read<fp32>();

    light.falloff = read<fp32>();
    light.phi = read<fp32>();
    light.range = read<fp32>();
    light.theta = read<fp32>();

    light.active = read<uint8>();
    light.position = readVector3D();
    light.rotation = readVector3D();

    return true;
}

bool OpfParser::parseCustomSettings(QVector<CustomSetting>& settings)
{
    uint32 count = read<uint32>();
    settings.reserve(count);

    for (uint32 i = 0; i < count; i++)
    {
        CustomSetting setting;
        setting.name = readOutforceString();
        setting.value = readOutforceString();
        settings.append(setting);
    }

    return true;
}

// ============================================================================
// HELPER FUNCTIONS
// ============================================================================

QString OpfParser::readOutforceString()
{
    uint16 length = read<uint16>();
    if (length == 0) return QString();

    QByteArray data(length, 0);
    m_stream->readRawData(data.data(), length);

    // Convert from Windows-1252 to UTF-8
    return QString::fromLatin1(data);
}

QString OpfParser::readOutforceString32()
{
    uint32 length = read<uint32>();
    if (length == 0) return QString();

    QByteArray data(length, 0);
    m_stream->readRawData(data.data(), length);

    return QString::fromLatin1(data);
}

QString OpfParser::readStaticString(int length)
{
    QByteArray data(length, 0);
    m_stream->readRawData(data.data(), length);

    int nullPos = data.indexOf('\0');
    if (nullPos >= 0)
    {
        data = data.left(nullPos);
    }

    return QString::fromLatin1(data);
}

Vector2D OpfParser::readVector2D()
{
    Vector2D vec;
    vec.x = read<fp32>();
    vec.y = read<fp32>();
    return vec;
}

Vector3D OpfParser::readVector3D()
{
    Vector3D vec;
    vec.x = read<fp32>();
    vec.y = read<fp32>();
    vec.z = read<fp32>();
    return vec;
}

Vector2D_uint32 OpfParser::readVector2D_uint32()
{
    Vector2D_uint32 vec;
    vec.x = read<uint32>();
    vec.y = read<uint32>();
    return vec;
}

Vertex OpfParser::readVertex()
{
    Vertex v;
    v.position = readVector3D();
    v.normal = readVector3D();
    return v;
}

} // namespace Opf
