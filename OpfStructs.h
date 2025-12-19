#ifndef OPFSTRUCTS_H
#define OPFSTRUCTS_H

#include <QString>
#include <QVector>
#include <QMap>
#include <QByteArray>
#include <cstdint>

namespace Opf {

// ============================================================================
// BASIC TYPES (matching Outforce types)
// ============================================================================

using uint8 = uint8_t;
using uint16 = uint16_t;
using uint32 = uint32_t;
using int32 = int32_t;
using fp32 = float;

// ============================================================================
// ENUMS (from Outforce_D3D.h, Outforce_Object.h, Outforce_Texture.h, Outforce_Misc.h)
// ============================================================================

// EOverride (Outforce_Misc.h)
enum class EOverride : uint8
{
    None = 0,
    Delete = 0x01,
    Merge = 0x02,
    Overridden = 0x04,
    Visual = 0x08,
    UnitWeapon = 0x10
};

// EBitmapType (Outforce_Texture.h)
enum class EBitmapType : uint8
{
    BMP = 0,
    JPG_Subsampling_None = 1,
    JPG_Subsampling_411 = 2,
    JPG_Subsampling_422 = 3
};

// ETextureWrapType (Outforce_Object.h)
enum class ETextureWrapType : uint32
{
    Flat = 0,
    Cylinder = 1,
    Sphere = 2,
    SequenceSquare = 3,
    SequenceTriangle = 4
};

// EBufferType (Outforce_Object.h)
enum class EBufferType : int32
{
    Points = 1,
    Lines = 2,
    Triangles = 3
};

// EVertexFormat (Outforce_D3D.h)
enum class EVertexFormat : int32
{
    Reserved0 = 0x001,
    Position_Mask = 0x00E,
    XYZ = 0x002,
    XYZRHW = 0x004,
    XYZB1 = 0x006,
    XYZB2 = 0x008,
    XYZB3 = 0x00a,
    XYZB4 = 0x00c,
    XYZB5 = 0x00e,
    Normal = 0x010,
    Reserved1 = 0x020,
    Diffuse = 0x040,
    Specular = 0x080,
    TextCount_MASK = 0xf00,
    TexCount_SHIFT = 8,
    Tex0 = 0x000,
    Tex1 = 0x100,
    Tex2 = 0x200,
    Tex3 = 0x300,
    Tex4 = 0x400,
    Tex5 = 0x500,
    Tex6 = 0x600,
    Tex7 = 0x700,
    Tex8 = 0x800,
    Reserved2 = 0xf000
};

// ELightType (Outforce_D3D.h)
enum class ELightType : uint32
{
    Point = 1,
    Spot = 2,
    Directional = 3
};

// EFillMode (Outforce_D3D.h)
enum class EFillMode : uint8
{
    Point = 1,
    Wireframe = 2,
    Solid = 3
};

// EShadeMode (Outforce_D3D.h)
enum class EShadeMode : uint8
{
    Flat = 1,
    Gouraud = 2,
    Phong = 3
};

// EPixelOperation (Outforce_D3D.h)
enum class EPixelOperation : uint8
{
    Black = 1,
    NotMergePen = 2,
    MaskNotPen = 3,
    NotCopyPen = 4,
    MaskPenNot = 5,
    Not = 6,
    XorPen = 7,
    NotMaskPen = 8,
    MaskPen = 9,
    NotXorPen = 10,
    Nop = 11,
    MergeNotPen = 12,
    CopyPen = 13,
    MergePenNot = 14,
    MergePen = 15,
    White = 16
};

// ECmpFunction (Outforce_D3D.h)
enum class ECmpFunction : uint8
{
    Never = 1,
    Less = 2,
    Equal = 3,
    LessEqual = 4,
    Greater = 5,
    NotEqual = 6,
    GreaterEqual = 7,
    Always = 8
};

// EBlend (Outforce_D3D.h)
enum class EBlend : uint8
{
    Zero = 1,
    One = 2,
    SrcColor = 3,
    InvSrcColor = 4,
    SrcAlpha = 5,
    InvSrcAlpha = 6,
    DestAlpha = 7,
    InvDestAlpha = 8,
    DestColor = 9,
    InvDestColor = 10,
    SrcAlphaSat = 11,
    BothSrcAlpha = 12,
    BothInvSrcAlpha = 13
};

// ETextureStageArg (Outforce_D3D.h)
enum class ETextureStageArg : uint8
{
    SelectMask = 0x0f,
    Diffuse = 0x00,
    Current = 0x01,
    Texture = 0x02,
    TFactor = 0x03,
    Specular = 0x04,
    Complement = 0x10,
    AlphaReplicate = 0x20
};

// ETextureStageOperation (Outforce_D3D.h)
enum class ETextureStageOperation : uint8
{
    Disable = 1,
    SelectArg1 = 2,
    SelectArg2 = 3,
    Modulate = 4,
    Modulate2X = 5,
    Modulate4X = 6,
    Add = 7,
    AddSigned = 8,
    AddSigned2X = 9,
    Subtract = 10,
    AddSmooth = 11,
    BlendDiffuseAlpha = 12,
    BlendTextureAlpha = 13,
    BlendFactorAlpha = 14,
    BlendTextureAlphaPM = 15,
    BlendCurrentAlpha = 16,
    PreModulate = 17,
    ModulateAlpha_AddColor = 18,
    ModulateColor_AddAlpha = 19,
    ModulateInvAlpha_AddColor = 20,
    ModulateInvColor_AddAlpha = 21,
    BumpEnvMap = 22,
    BumpEnvMapLuminance = 23,
    DotProduct3 = 24
};

// ETextureAddress (Outforce_D3D.h)
enum class ETextureAddress : uint8
{
    Wrap = 1,
    Mirror = 2,
    Clamp = 3,
    Border = 4
};

// ETextureMagnificationFilter (Outforce_D3D.h)
enum class ETextureMagnificationFilter : uint8
{
    Point = 1,
    Linear = 2,
    FlatCubic = 3,
    GaussianCubic = 4,
    Anisotropic = 5
};

// ETextureMinificationFilter (Outforce_D3D.h)
enum class ETextureMinificationFilter : uint8
{
    Point = 1,
    Linear = 2,
    Anisotropic = 3
};

// ETextureMipmapFilter (Outforce_D3D.h)
enum class ETextureMipmapFilter : uint8
{
    None = 1,
    Point = 2,
    Linear = 3
};

// EWrapFlag (Outforce_D3D.h)
enum class EWrapFlag : uint8
{
    None = 0,
    WrapU = 1,
    WrapV = 2
};

// ============================================================================
// VECTOR TYPES (from Outforce_Misc.h)
// ============================================================================

struct Vector2D
{
    fp32 x = 0.0f;
    fp32 y = 0.0f;

    Vector2D() = default;
    Vector2D(fp32 _x, fp32 _y) : x(_x), y(_y) {}
};

struct Vector3D
{
    fp32 x = 0.0f;
    fp32 y = 0.0f;
    fp32 z = 0.0f;

    Vector3D() = default;
    Vector3D(fp32 _x, fp32 _y, fp32 _z) : x(_x), y(_y), z(_z) {}
};

struct Vector2D_uint32
{
    uint32 x = 0;
    uint32 y = 0;
};

// ============================================================================
// COLOR TYPES (from Outforce_D3D.h)
// ============================================================================

using Color = uint32;  // D3DCOLOR

struct ColorValue
{
    fp32 red = 0.0f;
    fp32 green = 0.0f;
    fp32 blue = 0.0f;
    fp32 alpha = 0.0f;

    ColorValue() = default;
    ColorValue(fp32 r, fp32 g, fp32 b, fp32 a) : red(r), green(g), blue(b), alpha(a) {}
};

// ============================================================================
// EVENT DESCRIPTOR (from Outforce_Misc.h)
// ============================================================================

struct EventDesc
{
    int32 trigger = 0;
    QString name;
};

// ============================================================================
// CUSTOM SETTINGS (from Outforce_Object.h) - KEY FOR GAME LOGIC!
// ============================================================================

struct CustomSetting
{
    QString name;   // e.g., "CanBuildUnit", "MaxHealth", "BuildTime", "Cost"
    QString value;  // The value

    CustomSetting() = default;
    CustomSetting(const QString& n, const QString& v) : name(n), value(v) {}
};

// ============================================================================
// BITMAP STRUCTURES (from Outforce_Texture.h)
// ============================================================================

struct BitmapInfoHeader
{
    uint32 size = 0;
    int32 width = 0;
    int32 height = 0;
    uint16 planes = 0;
    uint16 bitCount = 0;
    uint32 compression = 0;
    uint32 sizeImage = 0;
    int32 xPelsPerMeter = 0;
    int32 yPelsPerMeter = 0;
    uint32 clrUsed = 0;
    uint32 clrImportant = 0;
};

struct Bitmap
{
    int32 bitsPerPixel = 0;
    int32 width = 0;
    int32 height = 0;
    int32 lineSize = 0;
    EBitmapType bitmapType = EBitmapType::BMP;
    uint32 bitmapInfoHeaderSize = 0;
    BitmapInfoHeader bitmapInfoHeader;
    QByteArray extraHeaderData;
    QByteArray bitmapData;
};

// ============================================================================
// TEXTURE (from Outforce_Texture.h) - 100% COMPLETE
// ============================================================================

struct Texture
{
    // Basic info
    QString name;
    QString colorFile;      // Original color file path
    QString alphaFile;      // Original alpha file path
    Vector2D_uint32 size;   // Using proper struct
    int32 id = 0;
    uint16 projectID = 0;
    uint32 version = 0;

    // Convenience accessors (for compatibility)
    uint32 width = 0;
    uint32 height = 0;

    // Bit depths
    int32 colorBitDepthInFile = 0;
    int32 colorBitDepthInMemory = 0;
    int32 alphaBitDepthInFile = 0;
    int32 alphaBitDepthInMemory = 0;

    // Flags
    bool colorDither = true;
    bool alphaDither = true;
    bool inverseAlpha = false;
    bool betterQuality = false;
    bool hasColorChannel = true;
    bool hasAlphaChannel = false;
    bool grayScale = false;
    bool multiTexture = false;
    int32 maxCap = 16;
    bool mipMap = true;

    // Override/Export
    EOverride override = EOverride::None;
    bool excludeFromExport = false;
    EBitmapType exportColorQuality = EBitmapType::BMP;
    EBitmapType exportAlphaQuality = EBitmapType::BMP;
    uint8 exportColorJPEGQuality = 88;
    uint8 exportAlphaJPEGQuality = 88;

    // Bitmap data (for export - simplified access)
    int colorBitsPerPixel = 0;
    int alphaBitsPerPixel = 0;
    uint8 colorBitmapType = 0;
    uint8 alphaBitmapType = 0;
    QByteArray colorData;
    QByteArray alphaData;

    // Full bitmap structures (for complete data preservation)
    Bitmap colorBitmap;
    Bitmap alphaBitmap;

    // Utility methods
    void clearBitmapData()
    {
        colorData.clear();
        alphaData.clear();
        colorBitmap.bitmapData.clear();
        alphaBitmap.bitmapData.clear();
    }

    size_t memoryUsage() const
    {
        return colorData.size() + alphaData.size() + colorBitmap.bitmapData.size() + alphaBitmap.bitmapData.size();
    }
};

// ============================================================================
// MATERIAL STRUCTURES (from Outforce_Material.h) - 100% COMPLETE
// ============================================================================

struct Material7
{
    ColorValue diffuse{1.0f, 1.0f, 1.0f, 0.0f};
    ColorValue ambient{1.0f, 1.0f, 1.0f, 0.0f};
    ColorValue specular{1.0f, 1.0f, 1.0f, 0.0f};
    ColorValue emissive{0.0f, 0.0f, 0.0f, 0.0f};
    fp32 power = 0.0f;
};

struct RenderPassSettings
{
    bool antialias = true;
    bool perspectiveCorrection = true;
    EFillMode fillMode = EFillMode::Solid;
    EShadeMode shadeMode = EShadeMode::Gouraud;
    uint16 linePattern_RepeatFactor = 1;
    uint16 linePattern_LinePattern = 0xFFFF;
    EPixelOperation pixelOperation = EPixelOperation::CopyPen;
    bool writeToZBuffer = true;
    bool testForAlphaBlending = false;
    uint8 alphaReference = 0;
    ECmpFunction alphaFunction = ECmpFunction::Always;
    bool drawLastPixel = false;
    EBlend sourceBlendMode = EBlend::One;
    EBlend destBlendMode = EBlend::Zero;
    ECmpFunction zCompareFunction = ECmpFunction::LessEqual;
    bool alphaBlending = false;
    bool affectedByFog = true;
    bool specularEnable = false;
    bool stippled = false;
    bool edgeAntialias = false;
    bool colorKeying = false;
    uint8 zBias = 0;
    Color textureFactorColor = 0;

    uint32 stipplePattern[32] =
    {
        0xAAAAAAAA, 0x55555555, 0xAAAAAAAA, 0x55555555,
        0xAAAAAAAA, 0x55555555, 0xAAAAAAAA, 0x55555555,
        0xAAAAAAAA, 0x55555555, 0xAAAAAAAA, 0x55555555,
        0xAAAAAAAA, 0x55555555, 0xAAAAAAAA, 0x55555555,
        0xAAAAAAAA, 0x55555555, 0xAAAAAAAA, 0x55555555,
        0xAAAAAAAA, 0x55555555, 0xAAAAAAAA, 0x55555555,
        0xAAAAAAAA, 0x55555555, 0xAAAAAAAA, 0x55555555,
        0xAAAAAAAA, 0x55555555, 0xAAAAAAAA, 0x55555555
    };
    bool useVertexColorWhenLighting = true;
    Material7 materialDescription;
};

struct RenderPassStage
{
    ETextureStageArg colorArg1 = ETextureStageArg::Texture;
    ETextureStageArg colorArg2 = ETextureStageArg::Diffuse;
    ETextureStageOperation colorOp = ETextureStageOperation::Modulate;
    ETextureStageArg alphaArg1 = ETextureStageArg::Texture;
    ETextureStageArg alphaArg2 = ETextureStageArg::Diffuse;
    ETextureStageOperation alphaOp = ETextureStageOperation::Modulate;
    uint8 textureCoordinateIndex = 0;
    ETextureAddress textureAddressU = ETextureAddress::Wrap;
    ETextureAddress textureAddressV = ETextureAddress::Wrap;
    Color borderColor = 0xFFFFFFFF;
    ETextureMagnificationFilter maxTextureMagnificationFilter = ETextureMagnificationFilter::Anisotropic;
    ETextureMinificationFilter maxTextureMinificationFilter = ETextureMinificationFilter::Anisotropic;
    ETextureMipmapFilter maxTextureMipmapFilter = ETextureMipmapFilter::Linear;
    fp32 mipmapLODBias = 0.0f;
    uint8 minMipmapLevel = 0;
    uint8 maxAnisotropy = 1;
    EWrapFlag wrapping = EWrapFlag::None;
    uint16 textureProjectID = 0;
    int32 textureID = 0;
    int32 textureStageInTexture = 1;
};

struct RenderPass1Stage
{
    RenderPassSettings settings;
    RenderPassStage stages[1];
};

struct RenderPass2Stage
{
    RenderPassSettings settings;
    RenderPassStage stages[2];
};

struct RenderPass3Stage
{
    RenderPassSettings settings;
    RenderPassStage stages[3];
};

struct Material
{
    QString name;
    uint16 projectID = 0;
    int32 id = 0;
    uint32 version = 0;
    bool doubleSided = true;
    bool enabledLighting = true;
    EOverride override = EOverride::None;
    bool excludeFromExport = false;

    // Simple access (backward compatibility)
    int32 textureID = -1;

    // Full render pass data
    QVector<RenderPass1Stage> renderPasses1Stage;
    QVector<RenderPass2Stage> renderPasses2Stage;
    QVector<RenderPass3Stage> renderPasses3Stage;
};

// ============================================================================
// MESH/FACEBUFFER STRUCTURES (from Outforce_Object.h) - 100% COMPLETE
// ============================================================================

struct Vertex
{
    Vector3D position;
    Vector3D normal;
    Vector2D texCoord;

    Vertex() = default;
};

struct VertexMorphTarget
{
    QVector<Vertex> vertices;
    QString name;
};

struct ColorMorphTarget
{
    QVector<Color> colors;
};

struct TextureMorphTarget
{
    QVector<Vector2D> textureCoordinates;
};

struct GroupMorphTarget
{
    QVector<uint8> groups;
};

struct Mesh
{
    // Basic info
    QString name;
    EVertexFormat vertexFormat = EVertexFormat::XYZ;
    uint16 materialProjectID = 0;
    int32 materialID = 0;

    // Texture wrapping
    ETextureWrapType textureWrapType = ETextureWrapType::Flat;
    Vector3D textureOrigin;
    Vector3D textureRotation;
    Vector2D textureScale;
    Vector2D texturePosition;

    // Spatial
    Vector3D position;
    fp32 boundRadius = 0.0f;

    // Morph data
    int32 morphSize = 0;
    QVector<VertexMorphTarget> vertexMorphTargets;
    QVector<ColorMorphTarget> colorMorphTargets;
    QVector<TextureMorphTarget> textureMorphTargets;
    QVector<GroupMorphTarget> groupMorphTargets;

    // Morph blend parameters
    int32 srcVertex = 0;
    int32 srcColor = 0;
    int32 srcTexture[3] = {0, 0, 0};
    int32 dstVertex = 0;
    int32 dstColor = 0;
    int32 dstTexture[3] = {0, 0, 0};
    fp32 amountVertex = 0.0f;
    fp32 amountColor = 0.0f;
    fp32 amountTexture[3] = {0.0f, 0.0f, 0.0f};
    bool dstEnvironment[3] = {false, false, false};

    // Geometry
    EBufferType bufferType = EBufferType::Triangles;
    int32 numFaces = 0;

    // Vertex and index data (simplified for export)
    QVector<Vertex> vertices;
    QVector<uint16> indices;

    // Legacy compatibility
    int primitiveType = 3;

    Mesh() = default;
};

// ============================================================================
// LIGHT STRUCTURE (from Outforce_Object.h) - 100% COMPLETE
// ============================================================================

struct Light
{
    ColorValue diffuse{1.0f, 1.0f, 1.0f, 0.0f};
    ColorValue ambient{0.0f, 0.0f, 0.0f, 0.0f};
    ColorValue specular{1.0f, 1.0f, 1.0f, 0.0f};
    ELightType lightType = ELightType::Point;
    fp32 attenuation0 = 0.0f;
    fp32 attenuation1 = 0.0f;
    fp32 attenuation2 = 1.0f;
    fp32 falloff = 0.0f;
    fp32 phi = 0.0f;
    fp32 range = 1.0f;
    fp32 theta = 0.0f;
    bool active = false;
    Vector3D position;
    Vector3D rotation;
};

// ============================================================================
// OBJECT TEMPLATE (from Outforce_Object.h)
// ============================================================================

struct ObjectTemplate
{
    Vector3D physicalScaling;
    Vector3D physicalPosition;
    Vector3D physicalRotation;
    Vector3D lastPhysicalScaling;
    Vector3D lastPhysicalPosition;
    Vector3D lastPhysicalRotation;
    QVector<Mesh> faceBuffers;
};

// ============================================================================
// OBJECT (from Outforce_Object.h) - 100% COMPLETE
// ============================================================================

struct Object
{
    // Class and identification
    QString className;
    QString name;
    uint16 projectID = 0;
    int32 uniqueID = 0;
    uint32 version = 0;

    // Flags
    bool isUnknown = false;         // m_bUnknown
    bool isSuperParent = false;     // m_bIsSuperParent (not serialized)
    EOverride override = EOverride::None;
    bool excludeFromExport = false;
    bool isDisabled = false;        // m_bDisable
    bool disableTree = false;       // m_bDisableTree
    bool isBillboard = false;       // m_bBillBoard

    // Transform
    Vector3D position;
    Vector3D rotation;
    Vector3D scaling;

    // Template data
    ObjectTemplate objectTemplate;

    // Light
    bool hasLight = false;
    Light light;

    // Custom settings (GAME LOGIC DATA!)
    QVector<CustomSetting> customSettings;

    // Hierarchy
    QVector<Object*> children;

    // Convenience: direct mesh access (reference to objectTemplate.faceBuffers)
    QVector<Mesh>& meshes() { return objectTemplate.faceBuffers; }
    const QVector<Mesh>& meshes() const { return objectTemplate.faceBuffers; }

    // Category helper
    QString getCategory() const
    {
        if (className.contains("Unit")) return "Units";
        if (className.contains("Base") || className.contains("Building")) return "Buildings";
        if (className.contains("Weapon")) return "Weapons";
        if (className.contains("Projectile")) return "Projectiles";
        if (className.contains("Effect")) return "Effects";
        if (className.contains("Environment") || className.contains("Env")) return "Environment";
        return "Other";
    }

    // Custom setting helpers
    QString getCustomSetting(const QString& settingName) const
    {
        for (const auto& setting : customSettings)
        {
            if (setting.name == settingName)
            {
                return setting.value;
            }
        }
        return QString();
    }

    void setCustomSetting(const QString& settingName, const QString& value)
    {
        for (auto& setting : customSettings)
        {
            if (setting.name == settingName)
            {
                setting.value = value;
                return;
            }
        }
        customSettings.append(CustomSetting(settingName, value));
    }

    bool hasCustomSetting(const QString& settingName) const
    {
        for (const auto& setting : customSettings)
        {
            if (setting.name == settingName)
            {
                return true;
            }
        }
        return false;
    }

    void removeCustomSetting(const QString& settingName)
    {
        for (int i = 0; i < customSettings.size(); ++i)
        {
            if (customSettings[i].name == settingName)
            {
                customSettings.removeAt(i);
                return;
            }
        }
    }

    // CanBuildUnit specific helpers
    QStringList getCanBuildUnits() const
    {
        QStringList units;
        for (const auto& setting : customSettings)
        {
            if (setting.name == "CanBuildUnit")
            {
                units.append(setting.value);
            }
        }
        return units;
    }

    void addCanBuildUnit(const QString& unitName)
    {
        customSettings.append(CustomSetting("CanBuildUnit", unitName));
    }

    void removeCanBuildUnit(const QString& unitName)
    {
        for (int i = customSettings.size() - 1; i >= 0; --i)
        {
            if (customSettings[i].name == "CanBuildUnit" && customSettings[i].value == unitName)
            {
                customSettings.removeAt(i);
            }
        }
    }

    void clearCanBuildUnits()
    {
        for (int i = customSettings.size() - 1; i >= 0; --i)
        {
            if (customSettings[i].name == "CanBuildUnit")
            {
                customSettings.removeAt(i);
            }
        }
    }

    // Destructor
    ~Object()
    {
        qDeleteAll(children);
    }
};

// ============================================================================
// PACKED PROJECT (from Outforce_Project.h) - 100% COMPLETE
// ============================================================================

struct PackedProject {

    // Header
    QString header = "Outforce Packed Content";
    uint32 version = 29;

    // Project info
    QString projectName;
    QString author;
    QString email;
    QString description;
    uint16 projectID = 0;

    // Dependencies (paths to other .opf files)
    QVector<QString> dependencies;

    // Event descriptors
    QVector<EventDesc> eventDescs;

    // Assets
    QVector<Texture> textures;
    QVector<Material> materials;
    QVector<Object*> objects;

    // Destructor
    ~PackedProject()
    {
        qDeleteAll(objects);
    }

    // Helper methods
    Object* findObjectByName(const QString& name) const
    {
        for (Object* obj : objects)
        {
            if (obj && obj->name == name)
            {
                return obj;
            }
        }
        return nullptr;
    }

    Object* findObjectByID(int32 uniqueID) const
    {
        for (Object* obj : objects)
        {
            if (obj && obj->uniqueID == uniqueID)
            {
                return obj;
            }
        }
        return nullptr;
    }

    const Texture* findTextureByID(int32 id) const
    {
        for (const auto& tex : textures)
        {
            if (tex.id == id)
            {
                return &tex;
            }
        }
        return nullptr;
    }

    Texture* findTextureByID(int32 id)
    {
        for (auto& tex : textures)
        {
            if (tex.id == id)
            {
                return &tex;
            }
        }
        return nullptr;
    }

    const Material* findMaterialByID(int32 id) const
    {
        for (const auto& mat : materials)
        {
            if (mat.id == id)
            {
                return &mat;
            }
        }
        return nullptr;
    }

    Material* findMaterialByID(int32 id)
    {
        for (auto& mat : materials)
        {
            if (mat.id == id)
            {
                return &mat;
            }
        }
        return nullptr;
    }

    // Get all objects that can build units
    QVector<Object*> getBuildableObjects() const
    {
        QVector<Object*> result;
        for (Object* obj : objects)
        {
            if (obj && !obj->getCanBuildUnits().isEmpty())
            {
                result.append(obj);
            }
        }
        return result;
    }

    // Get all unit names (for CanBuildUnit dropdown)
    QStringList getAllUnitNames() const
    {
        QStringList names;
        for (Object* obj : objects)
        {
            if (obj && obj->className.contains("Unit"))
            {
                names.append(obj->name);
            }
        }
        names.sort();
        return names;
    }

    // Get all object names
    QStringList getAllObjectNames() const
    {
        QStringList names;
        for (Object* obj : objects)
        {
            if (obj)
            {
                names.append(obj->name);
            }
        }
        names.sort();
        return names;
    }
};

} // namespace Opf

#endif // OPFSTRUCTS_H
