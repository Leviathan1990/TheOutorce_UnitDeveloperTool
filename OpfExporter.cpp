#include "OpfExporter.h"
#include "SettingsManager.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>
#include <QDir>
#include <QDebug>
#include <QTextStream>
#include <QImage>
#include <QProgressDialog>
#include <QApplication>
#include <QPainter>
#include <QRegularExpression>
#include <QFileInfo>
#include <QDateTime>

namespace Opf
{

OpfExporter::OpfExporter(QObject* parent) : QObject(parent), m_currentProgress(0)
{
}

QString OpfExporter::sanitizeFilename(const QString& filename)
{
    QString safe = filename;

    safe.replace(QChar('<'), QChar('_'));
    safe.replace(QChar('>'), QChar('_'));
    safe.replace(QChar(':'), QChar('_'));
    safe.replace(QChar('"'), QChar('_'));
    safe.replace(QChar('/'), QChar('_'));
    safe.replace(QChar('\\'), QChar('_'));
    safe.replace(QChar('|'), QChar('_'));
    safe.replace(QChar('?'), QChar('_'));
    safe.replace(QChar('*'), QChar('_'));

    safe.replace(QChar(0x00E5), QChar('a'));
    safe.replace(QChar(0x00E4), QChar('a'));
    safe.replace(QChar(0x00F6), QChar('o'));
    safe.replace(QChar(0x00C5), QChar('A'));
    safe.replace(QChar(0x00C4), QChar('A'));
    safe.replace(QChar(0x00D6), QChar('O'));
    safe.replace(QChar(0x00E9), QChar('e'));
    safe.replace(QChar(0x00E8), QChar('e'));
    safe.replace(QChar(0x00EA), QChar('e'));
    safe.replace(QChar(0x00EB), QChar('e'));
    safe.replace(QChar(0x00E1), QChar('a'));
    safe.replace(QChar(0x00E0), QChar('a'));
    safe.replace(QChar(0x00E2), QChar('a'));
    safe.replace(QChar(0x00FC), QChar('u'));
    safe.replace(QChar(0x00FA), QChar('u'));
    safe.replace(QChar(0x00F9), QChar('u'));
    safe.replace(QChar(0x00FB), QChar('u'));
    safe.replace(QChar(0x00ED), QChar('i'));
    safe.replace(QChar(0x00EC), QChar('i'));
    safe.replace(QChar(0x00EE), QChar('i'));
    safe.replace(QChar(0x00EF), QChar('i'));
    safe.replace(QChar(0x00F3), QChar('o'));
    safe.replace(QChar(0x00F2), QChar('o'));
    safe.replace(QChar(0x00F4), QChar('o'));

    safe.replace(QChar(' '), QChar('_'));

    QString result;
    for (const QChar& ch : safe)
    {
        ushort unicode = ch.unicode();
        if ((unicode >= 32 && unicode <= 126) || ch == '_' || ch == '-' || ch == '.')
        {
            result += ch;
        }
    }

    while (result.endsWith('.') || result.endsWith(' '))
    {
        result.chop(1);
    }

    if (result.isEmpty())
    {
        result = "unnamed";
    }

    if (result.length() > 100)
    {
        result = result.left(100);
    }

    return result;
}

// ============================================================================
// HELPER: Count all meshes recursively (including children)
// ============================================================================
static int countAllMeshesRecursive(const Object& object)
{
    int count = object.meshes().size();
    for (const Object* child : object.children)
    {
        if (child)
        {
            count += countAllMeshesRecursive(*child);
        }
    }
    return count;
}

// ============================================================================
// HELPER: Check if object or any children have meshes
// ============================================================================
static bool hasAnyMeshesRecursive(const Object& object)
{
    if (!object.meshes().isEmpty())
    {
        return true;
    }
    for (const Object* child : object.children)
    {
        if (child && hasAnyMeshesRecursive(*child))
        {
            return true;
        }
    }
    return false;
}

bool OpfExporter::exportTemplatesToJson(const PackedProject& project, const QString& filename)
{
    QJsonObject root;
    root["version"] = "2.0";
    root["source"] = "PackedProject.opf";
    root["projectName"] = project.projectName;
    root["projectID"] = project.projectID;

    // Templates (objects)
    QJsonArray templates;
    for (const Object* obj : project.objects)
    {
        if (!obj) continue;

        QJsonObject templ;
        templ["id"] = static_cast<qint64>(obj->uniqueID);
        templ["index"] = static_cast<int>(obj->projectID);
        templ["name"] = obj->name;
        templ["class"] = obj->className;
        templ["category"] = obj->getCategory();
        templ["isSpecial"] = obj->hasLight || obj->className.contains("Base");
        templ["hasLight"] = obj->hasLight;
        templ["isBillboard"] = obj->isBillboard;
        templ["isDisabled"] = obj->isDisabled;
        templ["meshCount"] = obj->meshes().size();
        templ["totalMeshCount"] = countAllMeshesRecursive(*obj);  // NEW: includes children
        templ["childCount"] = obj->children.size();

        // Custom settings
        QJsonArray customSettingsArray;
        for (const auto& setting : obj->customSettings)
        {
            QJsonObject settingObj;
            settingObj["name"] = setting.name;
            settingObj["value"] = setting.value;
            customSettingsArray.append(settingObj);
        }
        templ["customSettings"] = customSettingsArray;

        // CanBuildUnit list
        QJsonArray canBuildArray;
        for (const QString& unit : obj->getCanBuildUnits())
        {
            canBuildArray.append(unit);
        }
        templ["canBuildUnits"] = canBuildArray;

        templates.append(templ);

        // Add children recursively
        std::function<void(const Object*, const QString&)> addChildrenRecursive;
        addChildrenRecursive = [&](const Object* parent, const QString& parentName) {
            for (const Object* child : parent->children)
            {
                if (!child) continue;

                QJsonObject childTempl;
                childTempl["id"] = static_cast<qint64>(child->uniqueID);
                childTempl["index"] = static_cast<int>(child->projectID);
                childTempl["name"] = child->name;
                childTempl["class"] = child->className;
                childTempl["category"] = child->getCategory();
                childTempl["isSpecial"] = child->hasLight;
                childTempl["parent"] = parentName;
                childTempl["meshCount"] = child->meshes().size();
                childTempl["totalMeshCount"] = countAllMeshesRecursive(*child);
                childTempl["childCount"] = child->children.size();

                QJsonArray childCustomSettings;
                for (const auto& setting : child->customSettings)
                {
                    QJsonObject settingObj;
                    settingObj["name"] = setting.name;
                    settingObj["value"] = setting.value;
                    childCustomSettings.append(settingObj);
                }
                childTempl["customSettings"] = childCustomSettings;

                templates.append(childTempl);

                // Recurse into grandchildren
                addChildrenRecursive(child, child->name);
            }
        };

        addChildrenRecursive(obj, obj->name);
    }
    root["templates"] = templates;

    // Textures
    QJsonArray textures;
    for (const Texture& tex : project.textures)
    {
        QJsonObject texObj;
        texObj["id"] = tex.id;
        texObj["name"] = tex.name;
        texObj["projectID"] = tex.projectID;
        texObj["width"] = static_cast<int>(tex.width);
        texObj["height"] = static_cast<int>(tex.height);
        texObj["hasAlpha"] = tex.hasAlphaChannel;
        texObj["hasColor"] = tex.hasColorChannel;

        QString safeName = sanitizeFilename(tex.name);
        QString textureFilename = QString("%1_%2.png").arg(safeName).arg(tex.id);
        texObj["filename"] = textureFilename;
        texObj["path"] = QString("textures/%1").arg(textureFilename);

        textures.append(texObj);
    }
    root["textures"] = textures;
    root["textureCount"] = textures.size();

    // Materials
    QJsonArray materials;
    for (const Material& mat : project.materials)
    {
        QJsonObject matObj;
        matObj["id"] = mat.id;
        matObj["name"] = mat.name;
        matObj["projectID"] = mat.projectID;
        matObj["doubleSided"] = mat.doubleSided;
        matObj["lighting"] = mat.enabledLighting;
        matObj["textureID"] = mat.textureID;
        materials.append(matObj);
    }
    root["materials"] = materials;
    root["materialCount"] = materials.size();

    // Dependencies
    QJsonArray deps;
    for (const QString& dep : project.dependencies)
    {
        deps.append(dep);
    }
    root["dependencies"] = deps;

    // Events
    QJsonArray events;
    for (const EventDesc& evt : project.eventDescs)
    {
        QJsonObject evtObj;
        evtObj["trigger"] = evt.trigger;
        evtObj["name"] = evt.name;
        events.append(evtObj);
    }
    root["events"] = events;

    // Save
    QJsonDocument doc(root);
    QFile file(filename);
    if (!file.open(QIODevice::WriteOnly))
    {
        m_lastError = QString("Cannot write file: %1").arg(filename);
        return false;
    }

    file.write(doc.toJson(QJsonDocument::Indented));
    file.close();

    qDebug() << "Exported" << templates.size() << "templates to" << filename;
    return true;
}

bool OpfExporter::exportObject(const Object& object, const QString& filename)
{
    QJsonObject root;
    root["name"] = object.name;
    root["class"] = object.className;
    root["uniqueID"] = static_cast<qint64>(object.uniqueID);
    root["projectID"] = object.projectID;
    root["version"] = static_cast<int>(object.version);

    QJsonObject position;
    position["x"] = object.position.x;
    position["y"] = object.position.y;
    position["z"] = object.position.z;
    root["position"] = position;

    QJsonObject rotation;
    rotation["x"] = object.rotation.x;
    rotation["y"] = object.rotation.y;
    rotation["z"] = object.rotation.z;
    root["rotation"] = rotation;

    QJsonObject scaling;
    scaling["x"] = object.scaling.x;
    scaling["y"] = object.scaling.y;
    scaling["z"] = object.scaling.z;
    root["scaling"] = scaling;

    root["hasLight"] = object.hasLight;
    root["isDisabled"] = object.isDisabled;
    root["isBillboard"] = object.isBillboard;
    root["childCount"] = object.children.size();
    root["meshCount"] = object.meshes().size();
    root["totalMeshCount"] = countAllMeshesRecursive(object);

    // Custom settings
    QJsonArray customSettings;
    for (const auto& setting : object.customSettings)
    {
        QJsonObject settingObj;
        settingObj["name"] = setting.name;
        settingObj["value"] = setting.value;
        customSettings.append(settingObj);
    }
    root["customSettings"] = customSettings;

    // Children info
    if (!object.children.isEmpty())
    {
        QJsonArray childrenArray;
        for (const Object* child : object.children)
        {
            if (child)
            {
                QJsonObject childObj;
                childObj["name"] = child->name;
                childObj["class"] = child->className;
                childObj["uniqueID"] = static_cast<qint64>(child->uniqueID);
                childObj["meshCount"] = child->meshes().size();
                childObj["totalMeshCount"] = countAllMeshesRecursive(*child);
                childObj["childCount"] = child->children.size();
                childrenArray.append(childObj);
            }
        }
        root["children"] = childrenArray;
    }

    QJsonDocument doc(root);
    QFile file(filename);
    if (!file.open(QIODevice::WriteOnly))
    {
        m_lastError = QString("Cannot write file: %1").arg(filename);
        return false;
    }

    file.write(doc.toJson(QJsonDocument::Indented));
    file.close();

    return true;
}

bool OpfExporter::exportTexture(const Texture& texture, const QString& filename)
{
    QJsonObject root;
    root["name"] = texture.name;
    root["id"] = texture.id;
    root["projectID"] = texture.projectID;
    root["width"] = static_cast<int>(texture.width);
    root["height"] = static_cast<int>(texture.height);
    root["hasColorChannel"] = texture.hasColorChannel;
    root["hasAlphaChannel"] = texture.hasAlphaChannel;
    root["colorFile"] = texture.colorFile;
    root["alphaFile"] = texture.alphaFile;
    root["mipMap"] = texture.mipMap;
    root["grayScale"] = texture.grayScale;
    root["multiTexture"] = texture.multiTexture;

    QJsonDocument doc(root);
    QFile file(filename);
    if (!file.open(QIODevice::WriteOnly))
    {
        m_lastError = QString("Cannot write file: %1").arg(filename);
        return false;
    }

    file.write(doc.toJson(QJsonDocument::Indented));
    file.close();

    return true;
}

bool OpfExporter::exportMeshToObj(const Mesh& mesh, const QString& filename, const PackedProject& project)
{
    QFile file(filename);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        m_lastError = QString("Cannot write file: %1").arg(filename);
        return false;
    }

    QTextStream out(&file);

    QFileInfo fileInfo(filename);
    QString mtlFilename = fileInfo.completeBaseName() + ".mtl";

    out << "# Outforce OBJ Export v2.0\n";
    out << "# Mesh: " << mesh.name << "\n";
    out << "# Vertices: " << mesh.vertices.size() << "\n";
    out << "# Faces: " << (mesh.indices.size() / 3) << "\n";
    out << "# Material ID: " << mesh.materialID << "\n\n";

    out << "mtllib " << mtlFilename << "\n\n";

    // Vertices
    for (const Vertex& v : mesh.vertices)
    {
        out << QString("v %1 %2 %3\n")
        .arg(v.position.x, 0, 'f', 6)
            .arg(v.position.y, 0, 'f', 6)
            .arg(v.position.z, 0, 'f', 6);
    }
    out << "\n";

    // Normals
    for (const Vertex& v : mesh.vertices) {
        out << QString("vn %1 %2 %3\n")
        .arg(v.normal.x, 0, 'f', 6)
            .arg(v.normal.y, 0, 'f', 6)
            .arg(v.normal.z, 0, 'f', 6);
    }
    out << "\n";

    // Texture coordinates - FIX: Mirror Y around texture center (0.5)
    for (const Vertex& v : mesh.vertices) {
        out << QString("vt %1 %2\n")
        .arg(v.texCoord.x, 0, 'f', 6)
            .arg(1.0f - v.texCoord.y, 0, 'f', 6);
    }
    out << "\n";

    // Material
    QString mtlName = QString("material_%1").arg(mesh.materialID);
    out << "usemtl " << mtlName << "\n\n";

    // Faces - FIX: Reverse winding order (DirectX CW to OpenGL/Blender CCW)
    out << "# Faces\n";
    for (int i = 0; i < mesh.indices.size(); i += 3) {
        int i1 = mesh.indices[i] + 1;
        int i2 = mesh.indices[i + 1] + 1;
        int i3 = mesh.indices[i + 2] + 1;

        // Swap i2 and i3 to reverse winding order
        out << QString("f %1/%1/%1 %2/%2/%2 %3/%3/%3\n")
                   .arg(i1).arg(i3).arg(i2);
    }

    file.close();

    // Export MTL
    QString mtlPath = fileInfo.dir().filePath(mtlFilename);
    exportMeshMtl(mesh, mtlPath, project);

    return true;
}

// ============================================================================
// RECURSIVE MESH EXPORT - This is the KEY fix!
// ============================================================================

bool OpfExporter::exportObjectMeshes(const Object& object, const QString& directory, const PackedProject& project)
{
    QDir dir(directory);

    if (!dir.exists())
    {
        if (!dir.mkpath("."))
        {
            m_lastError = QString("Cannot create directory: %1").arg(directory);
            return false;
        }
    }

    // Export this object's meshes
    int meshIndex = 0;
    for (const Mesh& mesh : object.meshes())
    {
        QString meshName = mesh.name;
        if (meshName.isEmpty())
        {
            meshName = QString("%1_mesh_%2").arg(sanitizeFilename(object.name)).arg(meshIndex);
        }
        else
        {
            meshName = sanitizeFilename(meshName);
        }

        QString filename = dir.filePath(QString("%1.obj").arg(meshName));

        if (!exportMeshToObj(mesh, filename, project))
        {
            qWarning() << "Failed to export mesh:" << meshName;
        }
        else
        {
            qDebug() << "  Exported mesh:" << meshName << "from object:" << object.name;
        }

        meshIndex++;
    }

    // ========================================================================
    // RECURSIVELY EXPORT CHILDREN'S MESHES - THE KEY FIX!
    // ========================================================================
    for (const Object* child : object.children)
    {
        if (!child) continue;

        // Create subdirectory for child or export in same directory with prefixed names
        // Option 1: Same directory with child name prefix
        for (int i = 0; i < child->meshes().size(); ++i)
        {
            const Mesh& mesh = child->meshes()[i];

            QString meshName = mesh.name;
            if (meshName.isEmpty())
            {
                meshName = QString("%1_mesh_%2").arg(sanitizeFilename(child->name)).arg(i);
            }
            else
            {
                // Prefix with child name to avoid conflicts
                meshName = QString("%1_%2").arg(sanitizeFilename(child->name), sanitizeFilename(meshName));
            }

            QString filename = dir.filePath(QString("%1.obj").arg(meshName));

            if (!exportMeshToObj(mesh, filename, project))
            {
                qWarning() << "Failed to export child mesh:" << meshName;
            }
            else
            {
                qDebug() << "  Exported child mesh:" << meshName << "from child:" << child->name;
            }
        }

        // Recursively handle grandchildren
        if (!child->children.isEmpty())
        {
            exportObjectMeshesRecursiveHelper(*child, dir, project, child->name);
        }
    }

    return true;
}

// ============================================================================
// HELPER: Recursive mesh export for deeply nested children
// ============================================================================
void OpfExporter::exportObjectMeshesRecursiveHelper(const Object& object, const QDir& dir,
                                                    const PackedProject& project,
                                                    const QString& parentPrefix)
{
    for (const Object* child : object.children)
    {
        if (!child) continue;

        QString childPrefix = parentPrefix + "_" + sanitizeFilename(child->name);

        // Export this child's meshes
        for (int i = 0; i < child->meshes().size(); ++i)
        {
            const Mesh& mesh = child->meshes()[i];

            QString meshName = mesh.name;
            if (meshName.isEmpty())
            {
                meshName = QString("%1_mesh_%2").arg(childPrefix).arg(i);
            }
            else
            {
                meshName = QString("%1_%2").arg(childPrefix, sanitizeFilename(meshName));
            }

            QString filename = dir.filePath(QString("%1.obj").arg(meshName));

            if (!exportMeshToObj(mesh, filename, project))
            {
                qWarning() << "Failed to export grandchild mesh:" << meshName;
            }
            else
            {
                qDebug() << "  Exported grandchild mesh:" << meshName;
            }
        }

        // Continue recursion
        if (!child->children.isEmpty())
        {
            exportObjectMeshesRecursiveHelper(*child, dir, project, childPrefix);
        }
    }
}

bool OpfExporter::exportMeshMtl(const Mesh& mesh, const QString& mtlFilename, const PackedProject& project)
{
    QFile file(mtlFilename);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        m_lastError = QString("Cannot write MTL file: %1").arg(mtlFilename);
        return false;
    }

    QTextStream out(&file);

    out << "# Outforce MTL Export\n";
    out << "# Mesh: " << mesh.name << "\n";
    out << "# Material ID: " << mesh.materialID << "\n\n";

    QString mtlName = QString("material_%1").arg(mesh.materialID);
    out << "newmtl " << mtlName << "\n";

    out << "Ka 1.0 1.0 1.0\n";
    out << "Kd 1.0 1.0 1.0\n";
    out << "Ks 0.0 0.0 0.0\n";
    out << "d 1.0\n";
    out << "illum 2\n";

    const Texture* texture = findTextureForMaterial(mesh.materialID, project);
    if (texture)
    {
        QString textureName = sanitizeFilename(texture->name);
        QString texturePath = QString("../../textures/%1_%2.png").arg(textureName).arg(texture->id);
        out << "map_Kd " << texturePath << "\n";
    }

    file.close();
    return true;
}

const Texture* OpfExporter::findTextureForMaterial(int materialID, const PackedProject& project)
{
    const Material* material = nullptr;
    for (const Material& mat : project.materials)
    {
        if (mat.id == materialID)
        {
            material = &mat;
            break;
        }
    }

    if (!material || material->textureID == -1)
    {
        return nullptr;
    }

    return project.findTextureByID(material->textureID);
}

bool OpfExporter::exportTextureToPng(const Texture& texture, const QString& filename)
{
    if (texture.colorData.isEmpty())
    {
        qDebug() << "Texture" << texture.name << "has no color data, skipping";
        return true;
    }

    SettingsManager& settings = SettingsManager::instance();

    QImage image;

    // Decode color channel
    if (texture.colorBitmapType != 0)
    {
        image.loadFromData(texture.colorData, "JPEG");
        if (image.isNull())
        {
            qWarning() << "Failed to decode JPEG:" << texture.name;
            return false;
        }
    }

    else
    {
        if (texture.colorBitsPerPixel == 32)
        {
            image = QImage((const uchar*)texture.colorData.data(), texture.width, texture.height, QImage::Format_RGBA8888).copy();
        }

        else if (texture.colorBitsPerPixel == 24)
        {
            image = QImage((const uchar*)texture.colorData.data(), texture.width, texture.height, QImage::Format_RGB888).copy();
        }

        else if (texture.colorBitsPerPixel == 16)
        {
            image = QImage(texture.width, texture.height, QImage::Format_RGB888);
            const quint16* src = (const quint16*)texture.colorData.data();

            for (quint32 y = 0; y < texture.height; y++)
            {
                for (quint32 x = 0; x < texture.width; x++)
                {
                    quint16 pixel = src[y * texture.width + x];
                    quint8 r = ((pixel >> 11) & 0x1F) << 3;
                    quint8 g = ((pixel >> 5) & 0x3F) << 2;
                    quint8 b = (pixel & 0x1F) << 3;
                    image.setPixel(x, y, qRgb(r, g, b));
                }
            }
        }

        else
        {
            qWarning() << "Unsupported bit depth:" << texture.colorBitsPerPixel;
            return false;
        }
    }

    // Merge alpha channel
    if (texture.hasAlphaChannel && !texture.alphaData.isEmpty())
    {
        QImage alphaImage;

        if (texture.alphaBitmapType != 0)
        {
            alphaImage.loadFromData(texture.alphaData, "JPEG");
        }

        else
        {
            if (texture.alphaBitsPerPixel == 8)
            {
                alphaImage = QImage((const uchar*)texture.alphaData.data(), texture.width, texture.height, QImage::Format_Grayscale8).copy();
            }

            else if (texture.alphaBitsPerPixel == 16)
            {
                alphaImage = QImage(texture.width, texture.height, QImage::Format_Grayscale8);
                const quint16* src = (const quint16*)texture.alphaData.data();
                for (quint32 y = 0; y < texture.height; y++)
                {
                    for (quint32 x = 0; x < texture.width; x++)
                    {
                        quint16 pixel = src[y * texture.width + x];
                        quint8 gray = (pixel >> 8) & 0xFF;
                        alphaImage.setPixel(x, y, qRgb(gray, gray, gray));
                    }
                }
            }
        }

        if (!alphaImage.isNull())
        {
            if (image.format() != QImage::Format_RGBA8888)
            {
                image = image.convertToFormat(QImage::Format_RGBA8888);
            }

            for (quint32 y = 0; y < texture.height && y < (quint32)alphaImage.height(); y++)
            {
                for (quint32 x = 0; x < texture.width && x < (quint32)alphaImage.width(); x++)
                {
                    QRgb rgb = image.pixel(x, y);
                    int alpha = qGray(alphaImage.pixel(x, y));
                    image.setPixel(x, y, qRgba(qRed(rgb), qGreen(rgb), qBlue(rgb), alpha));
                }
            }
        }
    }

    // Apply texture scale
    SettingsManager::TextureScale scale = settings.textureScale();
    if (scale != SettingsManager::Scale100)
    {
        int newWidth = (image.width() * scale) / 100;
        int newHeight = (image.height() * scale) / 100;
        image = image.scaled(newWidth, newHeight, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    }

    // Save
    SettingsManager::TextureFormat format = settings.textureFormat();
    QString finalFilename = filename;
    const char* saveFormat = "PNG";
    int quality = -1;

    if (format == SettingsManager::JPEG)
    {
        finalFilename.replace(QRegularExpression("\\.png$", QRegularExpression::CaseInsensitiveOption), ".jpg");
        saveFormat = "JPEG";
        quality = 90;

        if (image.hasAlphaChannel())
        {
            QImage rgbImage(image.size(), QImage::Format_RGB888);
            rgbImage.fill(Qt::white);
            QPainter painter(&rgbImage);
            painter.drawImage(0, 0, image);
            painter.end();
            image = rgbImage;
        }
    }

    if (!image.save(finalFilename, saveFormat, quality))
    {
        m_lastError = QString("Failed to save image: %1").arg(finalFilename);
        return false;
    }

    return true;
}

bool OpfExporter::exportAllTextures(const QVector<Texture>& textures, const QString& directory)
{
    QDir dir(directory);
    if (!dir.exists())
    {
        if (!dir.mkpath("."))
        {
            m_lastError = QString("Cannot create directory: %1").arg(directory);
            return false;
        }
    }

    int exportedCount = 0;
    for (const Texture& tex : textures)
    {
        if (tex.colorData.isEmpty())
        {
            continue;
        }

        QString safeName = sanitizeFilename(tex.name);
        QString filename = dir.filePath(QString("%1_%2.png").arg(safeName).arg(tex.id));

        if (exportTextureToPng(tex, filename))
        {
            exportedCount++;
        }
    }

    qDebug() << "Exported" << exportedCount << "textures to" << directory;
    return true;
}

bool OpfExporter::exportMaterialToJson(const Material& material, const QString& filename)
{
    QJsonObject root;
    root["name"] = material.name;
    root["id"] = material.id;
    root["projectID"] = material.projectID;
    root["version"] = static_cast<int>(material.version);
    root["doubleSided"] = material.doubleSided;
    root["enabledLighting"] = material.enabledLighting;
    root["textureID"] = material.textureID;

    QJsonDocument doc(root);
    QFile file(filename);
    if (!file.open(QIODevice::WriteOnly))
    {
        m_lastError = QString("Cannot write file: %1").arg(filename);
        return false;
    }

    file.write(doc.toJson(QJsonDocument::Indented));
    file.close();

    return true;
}

bool OpfExporter::exportBlenderImportScript(const PackedProject& project, const QString& filename)
{
    QFile file(filename);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        m_lastError = QString("Cannot write script: %1").arg(filename);
        return false;
    }

    QTextStream out(&file);

    out << "# Outforce Blender Import Script v2.0\n";
    out << "# Generated by OutforceAssetBrowser\n";
    out << "# Project: " << project.projectName << "\n";
    out << "# Compatible with: Windows, Linux, macOS\n";
    out << "# Blender version: 3.0+\n\n";

    out << "import bpy\n";
    out << "import os\n";
    out << "import json\n\n";

    out << "print('='*60)\n";
    out << "print('Outforce Asset Importer v2.0')\n";
    out << "print('='*60)\n\n";

    out << "def get_script_dir():\n";
    out << "    if bpy.data.filepath:\n";
    out << "        script_dir = os.path.dirname(bpy.data.filepath)\n";
    out << "    else:\n";
    out << "        script_dir = bpy.path.abspath('//')\n";
    out << "        if script_dir == '//':\n";
    out << "            script_dir = os.getcwd()\n";
    out << "    return script_dir\n\n";

    out << "script_dir = get_script_dir()\n";
    out << "meshes_dir = os.path.join(script_dir, 'meshes')\n\n";

    out << "if not os.path.exists(meshes_dir):\n";
    out << "    print(f'ERROR: Meshes directory not found: {meshes_dir}')\n";
    out << "else:\n";
    out << "    main_collection = bpy.data.collections.new('Outforce_Assets')\n";
    out << "    bpy.context.scene.collection.children.link(main_collection)\n\n";

    out << "    imported = 0\n";
    out << "    for root, dirs, files in os.walk(meshes_dir):\n";
    out << "        for f in files:\n";
    out << "            if f.endswith('.obj'):\n";
    out << "                filepath = os.path.join(root, f)\n";
    out << "                try:\n";
    out << "                    bpy.ops.wm.obj_import(filepath=filepath)\n";
    out << "                    obj = bpy.context.active_object\n";
    out << "                    if obj:\n";
    out << "                        for coll in obj.users_collection:\n";
    out << "                            coll.objects.unlink(obj)\n";
    out << "                        main_collection.objects.link(obj)\n";
    out << "                    imported += 1\n";
    out << "                except Exception as e:\n";
    out << "                    print(f'Error importing {f}: {e}')\n\n";

    out << "    print(f'Imported {imported} meshes')\n";

    file.close();
    return true;
}

bool OpfExporter::exportAll(const PackedProject& project, const QString& directory, QProgressDialog* progress)
{
    m_currentProgress = 0;

    SettingsManager& settings = SettingsManager::instance();

    QDir dir(directory);
    if (!dir.exists())
    {
        if (!dir.mkpath("."))
        {
            m_lastError = QString("Cannot create directory: %1").arg(directory);
            return false;
        }
    }

    // Export templates.json
    if (settings.exportJSON())
    {
        updateProgress(progress, "Exporting templates.json...");
        QString templatesPath = dir.filePath("templates.json");

        if (!exportTemplatesToJson(project, templatesPath))
        {
            return false;
        }
    }

    // Create subdirectories
    dir.mkdir("objects");
    if (settings.exportOBJ()) dir.mkdir("meshes");
    if (settings.exportPNG()) dir.mkdir("textures");
    if (settings.exportJSON()) dir.mkdir("materials");

    // Export objects
    int objectCount = 0;
    int meshCount = 0;

    for (const Object* obj : project.objects)
    {
        if (progress && progress->wasCanceled()) return false;
        if (!obj) continue;

        updateProgress(progress, QString("Exporting object: %1...").arg(obj->name));

        QString safeName = sanitizeFilename(obj->name);

        if (settings.exportJSON())
        {
            QString jsonFile = dir.filePath(QString("objects/%1_%2.json").arg(safeName).arg(obj->uniqueID));
            if (exportObject(*obj, jsonFile))
            {
                objectCount++;
            }
        }

        // FIXED: Use hasAnyMeshesRecursive to check children too
        if (settings.exportOBJ() && hasAnyMeshesRecursive(*obj))
        {
            QString meshDir = dir.filePath(QString("meshes/%1_%2").arg(safeName).arg(obj->uniqueID));
            if (exportObjectMeshes(*obj, meshDir, project))
            {
                // FIXED: Count all meshes including children
                meshCount += countAllMeshesRecursive(*obj);
            }
        }
    }

    // Export textures
    if (settings.exportPNG())
    {
        for (int i = 0; i < project.textures.size(); i++)
        {
            if (progress && progress->wasCanceled()) return false;

            const Texture& tex = project.textures[i];
            updateProgress(progress, QString("Exporting texture: %1...").arg(tex.name));

            if (!tex.colorData.isEmpty())
            {
                QString safeName = sanitizeFilename(tex.name);
                QString filename = dir.filePath(QString("textures/%1_%2.png").arg(safeName).arg(tex.id));
                exportTextureToPng(tex, filename);
            }
        }
    }

    // Export materials
    if (settings.exportJSON())
    {
        for (const Material& mat : project.materials)
        {
            if (progress && progress->wasCanceled()) return false;

            updateProgress(progress, QString("Exporting material: %1...").arg(mat.name));

            QString safeName = sanitizeFilename(mat.name);
            QString filename = dir.filePath(QString("materials/%1_%2.json").arg(safeName).arg(mat.id));
            exportMaterialToJson(mat, filename);
        }
    }

    // Export Blender script
    if (settings.exportBlenderScript())
    {
        updateProgress(progress, "Creating Blender import script...");
        QString scriptPath = dir.filePath("import_to_blender.py");
        exportBlenderImportScript(project, scriptPath);
    }

    qDebug() << "Export complete:" << objectCount << "objects," << meshCount << "meshes (including children)";
    return true;
}

void OpfExporter::updateProgress(QProgressDialog* progress, const QString& status)
{
    m_currentProgress++;

    if (progress)
    {
        progress->setValue(m_currentProgress);
    }

    if (!status.isEmpty())
    {
        emit statusChanged(status);
    }

    QApplication::processEvents();
}


bool OpfExporter::exportAssetListToTxt(const PackedProject& project, const QString& filename)
{
    QFile file(filename);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        m_lastError = QString("Cannot write file: %1").arg(filename);
        return false;
    }

    QTextStream out(&file);

    // Header
    out << "================================================================================\n";
    out << "OUTFORCE ASSET LIST\n";
    out << "================================================================================\n\n";

    out << "Project Name: " << project.projectName << "\n";
    out << "Project ID: " << project.projectID << "\n";
    out << "Author: " << project.author << "\n";
    out << "Generated: " << QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss") << "\n";
    out << "\n";

    // Summary
    out << "=== SUMMARY ===\n";
    out << QString("Objects: %1\n").arg(project.objects.size());
    out << QString("Textures: %1\n").arg(project.textures.size());
    out << QString("Materials: %1\n").arg(project.materials.size());
    out << QString("Dependencies: %1\n").arg(project.dependencies.size());
    out << "\n";

    // Count total meshes
    int totalMeshes = 0;
    int totalChildren = 0;
    for (const Object* obj : project.objects)
    {
        if (obj)
        {
            totalMeshes += countAllMeshesRecursive(*obj);
            totalChildren += countChildrenRecursive(obj);
        }
    }
    out << QString("Total Meshes (including children): %1\n").arg(totalMeshes);
    out << QString("Total Child Objects: %1\n").arg(totalChildren);
    out << "\n";

    // Objects - grouped by category
    out << "================================================================================\n";
    out << QString("OBJECTS (%1)\n").arg(project.objects.size());
    out << "================================================================================\n\n";

    QMap<QString, QVector<const Object*>> categorized;
    for (const Object* obj : project.objects)
    {
        if (obj)
        {
            categorized[obj->getCategory()].append(obj);
        }
    }

    for (auto it = categorized.begin(); it != categorized.end(); ++it)
    {
        out << QString("--- %1 (%2) ---\n").arg(it.key()).arg(it.value().size());

        for (const Object* obj : it.value())
        {
            writeObjectToList(out, obj, 0);
        }
        out << "\n";
    }

    // Textures
    out << "================================================================================\n";
    out << QString("TEXTURES (%1)\n").arg(project.textures.size());
    out << "================================================================================\n\n";

    for (const Texture& tex : project.textures)
    {
        out << QString("%1 (ID: %2) - %3x%4")
        .arg(tex.name)
            .arg(tex.id)
            .arg(tex.width)
            .arg(tex.height);

        if (tex.hasAlphaChannel)
        {
            out << " [Alpha]";
        }
        out << "\n";
    }
    out << "\n";

    // Materials
    out << "================================================================================\n";
    out << QString("MATERIALS (%1)\n").arg(project.materials.size());
    out << "================================================================================\n\n";

    for (const Material& mat : project.materials)
    {
        out << QString("%1 (ID: %2) - TextureID: %3")
        .arg(mat.name)
            .arg(mat.id)
            .arg(mat.textureID);

        if (mat.doubleSided)
        {
            out << " [Double-sided]";
        }
        out << "\n";
    }
    out << "\n";

    // Dependencies
    if (!project.dependencies.isEmpty())
    {
        out << "================================================================================\n";
        out << QString("DEPENDENCIES (%1)\n").arg(project.dependencies.size());
        out << "================================================================================\n\n";

        for (const QString& dep : project.dependencies)
        {
            out << dep << "\n";
        }
        out << "\n";
    }

    out << "================================================================================\n";
    out << "END OF ASSET LIST\n";
    out << "================================================================================\n";

    file.close();

    qDebug() << "Exported asset list to:" << filename;
    return true;
}

void OpfExporter::writeObjectToList(QTextStream& out, const Object* obj, int depth)
{
    if (!obj) return;

    QString indent = QString(depth * 2, ' ');
    QString prefix = (depth == 0) ? "" : "[child] ";

    int meshCount = obj->meshes().size();
    int totalMeshCount = countAllMeshesRecursive(*obj);

    out << indent << prefix << obj->name;
    out << QString(" (ID: %1/%2)").arg(obj->uniqueID).arg(obj->projectID);
    out << QString(" - %1 mesh(es)").arg(meshCount);

    if (totalMeshCount != meshCount)
    {
        out << QString(" (%1 total)").arg(totalMeshCount);
    }

    if (!obj->children.isEmpty())
    {
        out << QString(" [%1 children]").arg(obj->children.size());
    }

    if (obj->hasLight)
    {
        out << " *LIGHT*";
    }

    if (!obj->customSettings.isEmpty())
    {
        out << QString(" {%1 settings}").arg(obj->customSettings.size());
    }

    out << "\n";

    // Recurse into children
    for (const Object* child : obj->children)
    {
        writeObjectToList(out, child, depth + 1);
    }
}

int OpfExporter::countChildrenRecursive(const Object* obj)
{
    if (!obj) return 0;

    int count = obj->children.size();
    for (const Object* child : obj->children)
    {
        count += countChildrenRecursive(child);
    }
    return count;
}


} // namespace Opf


