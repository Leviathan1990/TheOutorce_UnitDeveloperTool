#include "AssetPreviewWidget.h"
#include <QGroupBox>
#include <QHBoxLayout>
#include <QScrollArea>
#include <QDebug>

// ============================================================================
// HELPER FUNCTIONS FOR RECURSIVE MESH COUNTING
// ============================================================================

namespace {
// Helper: Count all meshes recursively (including children)
int countAllMeshesRecursive(const Opf::Object* object)
{
    if (!object) return 0;

    int count = object->meshes().size();
    for (const Opf::Object* child : object->children)
    {
        count += countAllMeshesRecursive(child);
    }
    return count;
}

// Helper: Count all vertices recursively
int countAllVerticesRecursive(const Opf::Object* object)
{
    if (!object) return 0;

    int count = 0;
    for (const auto& mesh : object->meshes())
    {
        count += mesh.vertices.size();
    }
    for (const Opf::Object* child : object->children)
    {
        count += countAllVerticesRecursive(child);
    }
    return count;
}

// Helper: Count all faces recursively
int countAllFacesRecursive(const Opf::Object* object)
{
    if (!object) return 0;

    int count = 0;
    for (const auto& mesh : object->meshes())
    {
        count += mesh.indices.size() / 3;
    }
    for (const Opf::Object* child : object->children)
    {
        count += countAllFacesRecursive(child);
    }
    return count;
}
}

// ============================================================================
// CONSTRUCTOR & SETUP
// ============================================================================

AssetPreviewWidget::AssetPreviewWidget(QWidget *parent) : QWidget(parent), m_currentTexture(nullptr), m_showingAlpha(false), m_currentZoom(1.0)
{
    setupUI();
}

void AssetPreviewWidget::setupUI()
{
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    // Main tab widget
    m_mainTabs = new QTabWidget(this);
    layout->addWidget(m_mainTabs);

    // ==================== DETAILS TAB ====================
    m_detailsTab = new QWidget();
    QVBoxLayout* detailsLayout = new QVBoxLayout(m_detailsTab);

    m_titleLabel = new QLabel("No selection", this);
    QFont titleFont = m_titleLabel->font();
    titleFont.setBold(true);
    titleFont.setPointSize(12);
    m_titleLabel->setFont(titleFont);
    detailsLayout->addWidget(m_titleLabel);

    m_detailsText = new QTextEdit(this);
    m_detailsText->setReadOnly(true);
    detailsLayout->addWidget(m_detailsText);

    m_mainTabs->addTab(m_detailsTab, "Details");

    // ==================== SETTINGS TAB ====================
    m_settingsTab = new QWidget();
    QVBoxLayout* settingsMainLayout = new QVBoxLayout(m_settingsTab);
    settingsMainLayout->setContentsMargins(0, 0, 0, 0);

    m_settingsTabs = new QTabWidget(this);

    m_customSettingsWidget = new CustomSettingsWidget(this);
    connect(m_customSettingsWidget, &CustomSettingsWidget::settingsModified,
            this, &AssetPreviewWidget::objectModified);
    m_settingsTabs->addTab(m_customSettingsWidget, "All Settings");

    m_canBuildWidget = new CanBuildUnitWidget(this);
    connect(m_canBuildWidget, &CanBuildUnitWidget::settingsModified,
            this, &AssetPreviewWidget::objectModified);
    m_settingsTabs->addTab(m_canBuildWidget, "Can Build Units");

    settingsMainLayout->addWidget(m_settingsTabs);

    m_mainTabs->addTab(m_settingsTab, "Settings");

    // ==================== STATISTICS TAB ====================
    m_statsTab = new QWidget();
    QVBoxLayout* statsLayout = new QVBoxLayout(m_statsTab);

    QScrollArea* statsScrollArea = new QScrollArea(this);
    statsScrollArea->setWidgetResizable(true);
    statsScrollArea->setFrameShape(QFrame::NoFrame);

    QWidget* statsContent = new QWidget();
    QVBoxLayout* statsContentLayout = new QVBoxLayout(statsContent);

    m_statsLabel = new QLabel("No asset selected", this);
    m_statsLabel->setWordWrap(true);
    m_statsLabel->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    m_statsLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);

    QFont monoFont("Courier");
    monoFont.setPointSize(9);
    m_statsLabel->setFont(monoFont);

    statsContentLayout->addWidget(m_statsLabel);
    statsContentLayout->addStretch();

    statsScrollArea->setWidget(statsContent);
    statsLayout->addWidget(statsScrollArea);

    m_mainTabs->addTab(m_statsTab, "Statistics");

    // ==================== TEXTURE TAB ====================
    m_textureTab = new QWidget();
    QVBoxLayout* textureLayout = new QVBoxLayout(m_textureTab);

    m_textureScene = new QGraphicsScene(this);
    m_textureView = new QGraphicsView(m_textureScene, this);
    m_textureView->setRenderHint(QPainter::SmoothPixmapTransform);
    m_textureView->setDragMode(QGraphicsView::ScrollHandDrag);
    m_textureItem = nullptr;

    textureLayout->addWidget(m_textureView, 1);

    QWidget* controlsWidget = new QWidget(this);
    QHBoxLayout* controlsLayout = new QHBoxLayout(controlsWidget);
    controlsLayout->setContentsMargins(0, 5, 0, 0);

    controlsLayout->addWidget(new QLabel("Zoom:", this));

    m_zoomSlider = new QSlider(Qt::Horizontal, this);
    m_zoomSlider->setRange(10, 400);
    m_zoomSlider->setValue(100);
    connect(m_zoomSlider, &QSlider::valueChanged, this, &AssetPreviewWidget::onZoomChanged);
    controlsLayout->addWidget(m_zoomSlider, 1);

    m_resetZoomButton = new QPushButton("Reset", this);
    connect(m_resetZoomButton, &QPushButton::clicked, this, &AssetPreviewWidget::onResetZoom);
    controlsLayout->addWidget(m_resetZoomButton);

    m_toggleAlphaButton = new QPushButton("Show Alpha", this);
    connect(m_toggleAlphaButton, &QPushButton::clicked, this, &AssetPreviewWidget::onToggleAlpha);
    controlsLayout->addWidget(m_toggleAlphaButton);

    textureLayout->addWidget(controlsWidget);

    m_mainTabs->addTab(m_textureTab, "Texture");

    // Initially hide Settings and Texture tabs
    m_mainTabs->setTabVisible(1, false); // Settings
    m_mainTabs->setTabVisible(3, false); // Texture
}

// ============================================================================
// SHOW OBJECT - FIXED WITH RECURSIVE MESH COUNTING
// ============================================================================

void AssetPreviewWidget::showObject(Opf::Object* object)
{
    m_currentTexture = nullptr;

    // Show/hide tabs
    m_mainTabs->setTabVisible(1, true);  // Settings - show
    m_mainTabs->setTabVisible(3, false); // Texture - hide

    if (!object)
    {
        clear();
        return;
    }

    m_customSettingsWidget->setObject(object);
    m_canBuildWidget->setObject(object);

    m_titleLabel->setText(QString("Object: %1").arg(object->name));

    QString details;
    details += QString("<b>Class:</b> %1<br>").arg(object->className);
    details += QString("<b>Unique ID:</b> %1<br>").arg(object->uniqueID);
    details += QString("<b>Project ID:</b> %1<br>").arg(object->projectID);
    details += QString("<b>Version:</b> %1<br>").arg(object->version);
    details += QString("<b>Category:</b> %1<br>").arg(object->getCategory());
    details += "<br>";

    details += QString("<b>Position:</b> (%.2f, %.2f, %.2f)<br>").arg(object->position.x).arg(object->position.y).arg(object->position.z);
    details += QString("<b>Rotation:</b> (%.2f, %.2f, %.2f)<br>").arg(object->rotation.x).arg(object->rotation.y).arg(object->rotation.z);
    details += QString("<b>Scaling:</b> (%.2f, %.2f, %.2f)<br>").arg(object->scaling.x).arg(object->scaling.y).arg(object->scaling.z);

    details += "<br>";
    details += QString("<b>Has Light:</b> %1<br>").arg(object->hasLight ? "Yes" : "No");
    details += QString("<b>Is Billboard:</b> %1<br>").arg(object->isBillboard ? "Yes" : "No");
    details += QString("<b>Is Disabled:</b> %1<br>").arg(object->isDisabled ? "Yes" : "No");
    details += QString("<b>Children:</b> %1<br>").arg(object->children.size());
    details += QString("<b>Custom Settings:</b> %1<br>").arg(object->customSettings.size());

    m_detailsText->setHtml(details);

    // ========================================================================
    // FIXED STATISTICS - Now includes children!
    // ========================================================================
    QString stats;
    stats += "=== OBJECT STATISTICS ===\n\n";

    // This object's direct meshes
    int directMeshes = object->meshes().size();
    int directVertices = 0;
    int directFaces = 0;
    for (const auto& mesh : object->meshes())
    {
        directVertices += mesh.vertices.size();
        directFaces += mesh.indices.size() / 3;
    }

    stats += QString("Direct Meshes: %1\n").arg(directMeshes);
    stats += QString("Direct Vertices: %1\n").arg(directVertices);
    stats += QString("Direct Faces: %1\n").arg(directFaces);

    // Total including children (RECURSIVE)
    int totalMeshes = countAllMeshesRecursive(object);
    int totalVertices = countAllVerticesRecursive(object);
    int totalFaces = countAllFacesRecursive(object);

    // Only show "including children" section if there ARE children with meshes
    if (totalMeshes != directMeshes)
    {
        stats += "\n--- INCLUDING CHILDREN ---\n";
        stats += QString("Total Meshes: %1\n").arg(totalMeshes);
        stats += QString("Total Vertices: %1\n").arg(totalVertices);
        stats += QString("Total Faces: %1\n").arg(totalFaces);
    }

    stats += "\n";

    // Children details
    if (!object->children.isEmpty())
    {
        stats += QString("=== CHILDREN (%1) ===\n").arg(object->children.size());
        for (const Opf::Object* child : object->children)
        {
            if (child)
            {
                int childDirectMeshes = child->meshes().size();
                int childTotalMeshes = countAllMeshesRecursive(child);

                stats += QString("  [+] %1").arg(child->name);

                if (childDirectMeshes > 0)
                {
                    stats += QString(" - %1 mesh(es)").arg(childDirectMeshes);
                }
                else
                {
                    stats += " - no direct meshes";
                }

                if (childTotalMeshes != childDirectMeshes)
                {
                    stats += QString(" (%1 total incl. sub-children)").arg(childTotalMeshes);
                }

                if (!child->children.isEmpty())
                {
                    stats += QString(" [%1 sub-children]").arg(child->children.size());
                }

                stats += "\n";

                // Show grandchildren too (one level deeper)
                for (const Opf::Object* grandchild : child->children)
                {
                    if (grandchild)
                    {
                        int gcMeshes = grandchild->meshes().size();
                        stats += QString("      [+] %1 - %2 mesh(es)\n")
                                     .arg(grandchild->name)
                                     .arg(gcMeshes);
                    }
                }
            }
        }
        stats += "\n";
    }

    stats += QString("Custom Settings: %1\n").arg(object->customSettings.size());

    if (!object->customSettings.isEmpty())
    {
        stats += "\n=== SETTINGS ===\n";
        for (const auto& setting : object->customSettings)
        {
            stats += QString("  %1 = %2\n").arg(setting.name, setting.value);
        }
    }

    updateStatistics(stats);

    // Switch to Details tab
    m_mainTabs->setCurrentIndex(0);
}

// ============================================================================
// SHOW TEXTURE
// ============================================================================

void AssetPreviewWidget::showTexture(const Opf::Texture* texture)
{
    m_customSettingsWidget->clear();
    m_canBuildWidget->clear();

    // Show/hide tabs
    m_mainTabs->setTabVisible(1, false); // Settings - hide
    m_mainTabs->setTabVisible(3, true);  // Texture - show

    if (!texture)
    {
        clear();
        return;
    }

    m_currentTexture = texture;
    m_showingAlpha = false;

    m_titleLabel->setText(QString("Texture: %1").arg(texture->name));

    QString details;
    details += QString("<b>ID:</b> %1<br>").arg(texture->id);
    details += QString("<b>Project ID:</b> %1<br>").arg(texture->projectID);
    details += QString("<b>Size:</b> %1 x %2<br>").arg(texture->width).arg(texture->height);
    details += QString("<b>Color Channel:</b> %1<br>").arg(texture->hasColorChannel ? "Yes" : "No");
    details += QString("<b>Alpha Channel:</b> %1<br>").arg(texture->hasAlphaChannel ? "Yes" : "No");
    details += QString("<b>MipMap:</b> %1<br>").arg(texture->mipMap ? "Yes" : "No");
    details += QString("<b>GrayScale:</b> %1<br>").arg(texture->grayScale ? "Yes" : "No");
    details += QString("<b>MultiTexture:</b> %1<br>").arg(texture->multiTexture ? "Yes" : "No");

    m_detailsText->setHtml(details);

    // Statistics
    QString stats;
    stats += "=== TEXTURE STATISTICS ===\n\n";
    stats += QString("Resolution: %1 x %2\n").arg(texture->width).arg(texture->height);
    stats += QString("Pixels: %1\n\n").arg(texture->width * texture->height);

    if (texture->hasColorChannel)
    {
        stats += QString("Color Data: %1 KB\n").arg(texture->colorData.size() / 1024.0, 0, 'f', 2);
        stats += QString("Color BPP: %1\n").arg(texture->colorBitsPerPixel);
        stats += QString("Color Format: %1\n\n").arg(texture->colorBitmapType == 0 ? "BMP" : "JPEG");
    }

    if (texture->hasAlphaChannel)
    {
        stats += QString("Alpha Data: %1 KB\n").arg(texture->alphaData.size() / 1024.0, 0, 'f', 2);
        stats += QString("Alpha BPP: %1\n").arg(texture->alphaBitsPerPixel);
    }

    updateStatistics(stats);
    showTexturePreview(texture);

    // Switch to Texture tab
    m_mainTabs->setCurrentIndex(3);
}

// ============================================================================
// SHOW MATERIAL
// ============================================================================

void AssetPreviewWidget::showMaterial(const Opf::Material* material)
{
    m_currentTexture = nullptr;

    // Show/hide tabs
    m_mainTabs->setTabVisible(1, false); // Settings - hide
    m_mainTabs->setTabVisible(3, false); // Texture - hide

    if (!material)
    {
        clear();
        return;
    }

    m_titleLabel->setText(QString("Material: %1").arg(material->name));

    QString details;
    details += QString("<b>ID:</b> %1<br>").arg(material->id);
    details += QString("<b>Project ID:</b> %1<br>").arg(material->projectID);
    details += QString("<b>Version:</b> %1<br>").arg(material->version);
    details += QString("<b>Double Sided:</b> %1<br>").arg(material->doubleSided ? "Yes" : "No");
    details += QString("<b>Lighting:</b> %1<br>").arg(material->enabledLighting ? "Enabled" : "Disabled");
    details += QString("<b>Texture ID:</b> %1<br>").arg(material->textureID);

    m_detailsText->setHtml(details);

    // Statistics
    QString stats;
    stats += "=== MATERIAL STATISTICS ===\n\n";
    stats += QString("1-Stage Passes: %1\n").arg(material->renderPasses1Stage.size());
    stats += QString("2-Stage Passes: %1\n").arg(material->renderPasses2Stage.size());
    stats += QString("3-Stage Passes: %1\n").arg(material->renderPasses3Stage.size());

    int totalPasses = material->renderPasses1Stage.size() + material->renderPasses2Stage.size() + material->renderPasses3Stage.size();
    stats += QString("\nTotal Passes: %1\n").arg(totalPasses);

    updateStatistics(stats);

    // Switch to Details tab
    m_mainTabs->setCurrentIndex(0);
}

// ============================================================================
// CLEAR & UTILITIES
// ============================================================================

void AssetPreviewWidget::clear()
{
    m_titleLabel->setText("No selection");
    m_detailsText->clear();
    m_statsLabel->setText("No asset selected");

    m_textureScene->clear();
    m_textureItem = nullptr;
    m_currentTexture = nullptr;

    m_customSettingsWidget->clear();
    m_canBuildWidget->clear();

    // Hide optional tabs
    m_mainTabs->setTabVisible(1, false); // Settings
    m_mainTabs->setTabVisible(3, false); // Texture

    m_mainTabs->setCurrentIndex(0);
}

void AssetPreviewWidget::setAvailableUnits(const QStringList& units)
{
    m_canBuildWidget->setAvailableUnits(units);
}

void AssetPreviewWidget::updateStatistics(const QString& stats)
{
    m_statsLabel->setText(stats);
}

// ============================================================================
// TEXTURE PREVIEW
// ============================================================================

void AssetPreviewWidget::showTexturePreview(const Opf::Texture* texture)
{
    if (!texture || texture->colorData.isEmpty())
    {
        return;
    }

    m_toggleAlphaButton->setEnabled(texture->hasAlphaChannel && !texture->alphaData.isEmpty());
    m_toggleAlphaButton->setText("Show Alpha");
    m_zoomSlider->setValue(100);
    m_currentZoom = 1.0;

    updateTexturePreview();
}

void AssetPreviewWidget::updateTexturePreview()
{
    if (!m_currentTexture) return;

    QImage image = createTextureImage(m_currentTexture, m_showingAlpha);

    if (image.isNull()) return;

    m_textureScene->clear();
    QPixmap pixmap = QPixmap::fromImage(image);
    m_textureItem = m_textureScene->addPixmap(pixmap);
    m_textureScene->setSceneRect(pixmap.rect());

    m_textureView->resetTransform();
    m_textureView->scale(m_currentZoom, m_currentZoom);
}

QImage AssetPreviewWidget::createTextureImage(const Opf::Texture* texture, bool alphaOnly)
{
    if (!texture) return QImage();

    QImage image;

    if (alphaOnly && texture->hasAlphaChannel && !texture->alphaData.isEmpty())
    {
        if (texture->alphaBitmapType != 0)
        {
            image.loadFromData(texture->alphaData, "JPEG");
        }

        else
        {
            if (texture->alphaBitsPerPixel == 8)
            {
                image = QImage((const uchar*)texture->alphaData.data(), texture->width, texture->height, QImage::Format_Grayscale8).copy();
            }
        }
    }

    else
    {
        if (texture->colorBitmapType != 0)
        {
            image.loadFromData(texture->colorData, "JPEG");
        }

        else
        {
            if (texture->colorBitsPerPixel == 32)
            {
                image = QImage((const uchar*)texture->colorData.data(), texture->width, texture->height, QImage::Format_RGBA8888).copy();
            }

            else if (texture->colorBitsPerPixel == 24)
            {
                image = QImage((const uchar*)texture->colorData.data(), texture->width, texture->height, QImage::Format_RGB888).copy();
            }

            else if (texture->colorBitsPerPixel == 16) {

                image = QImage(texture->width, texture->height, QImage::Format_RGB888);
                const quint16* src = (const quint16*)texture->colorData.data();
                for (quint32 y = 0; y < texture->height; y++)
                {
                    for (quint32 x = 0; x < texture->width; x++)
                    {
                        quint16 pixel = src[y * texture->width + x];
                        quint8 r = ((pixel >> 11) & 0x1F) << 3;
                        quint8 g = ((pixel >> 5) & 0x3F) << 2;
                        quint8 b = (pixel & 0x1F) << 3;
                        image.setPixel(x, y, qRgb(r, g, b));
                    }
                }
            }
        }
    }

    return image;
}

// ============================================================================
// SLOTS
// ============================================================================

void AssetPreviewWidget::onZoomChanged(int value)
{
    m_currentZoom = value / 100.0;

    if (m_currentTexture)
    {
        m_textureView->resetTransform();
        m_textureView->scale(m_currentZoom, m_currentZoom);
    }
}

void AssetPreviewWidget::onResetZoom()
{
    m_zoomSlider->setValue(100);
}

void AssetPreviewWidget::onToggleAlpha()
{
    if (!m_currentTexture) return;

    m_showingAlpha = !m_showingAlpha;
    m_toggleAlphaButton->setText(m_showingAlpha ? "Show Color" : "Show Alpha");
    updateTexturePreview();
}
