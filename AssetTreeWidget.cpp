#include "AssetTreeWidget.h"
#include <QHeaderView>
#include <QDebug>
#include <QKeyEvent>

AssetTreeWidget::AssetTreeWidget(QWidget *parent) : QTreeWidget(parent), m_currentProject(nullptr), m_categoryFilter("All"), m_onlyWithMeshes(false)
{
    setupUI();
}

void AssetTreeWidget::setupUI()
{
    setHeaderLabels(QStringList() << "Name" << "ID" << "Type");
    setColumnWidth(0, 300);
    setColumnWidth(1, 80);
    setColumnWidth(2, 120);

    setAlternatingRowColors(true);
    setAnimated(true);

    connect(this, &QTreeWidget::itemClicked, this, &AssetTreeWidget::onItemClicked);
}

void AssetTreeWidget::loadProject(const Opf::PackedProject& project)
{
    clear();
    m_currentProject = &project;

    addProjectInfo(project);
    addTextures(project.textures);
    addMaterials(project.materials);
    addObjects(project.objects);

    expandToDepth(1);
}

void AssetTreeWidget::clear()
{
    QTreeWidget::clear();
    m_currentProject = nullptr;
}

void AssetTreeWidget::addProjectInfo(const Opf::PackedProject& project)
{
    QTreeWidgetItem* infoItem = new QTreeWidgetItem(this);
    infoItem->setText(0, "Project Info");
    infoItem->setData(0, Qt::UserRole, ProjectInfo);

    new QTreeWidgetItem(infoItem, QStringList() << QString("Name: %1").arg(project.projectName));
    new QTreeWidgetItem(infoItem, QStringList() << QString("Author: %1").arg(project.author));
    new QTreeWidgetItem(infoItem, QStringList() << QString("Version: %1").arg(project.version));
    new QTreeWidgetItem(infoItem, QStringList() << QString("ProjectID: %1").arg(project.projectID));
    new QTreeWidgetItem(infoItem, QStringList() << QString("Dependencies: %1").arg(project.dependencies.size()));
    new QTreeWidgetItem(infoItem, QStringList() << QString("Events: %1").arg(project.eventDescs.size()));
}

void AssetTreeWidget::addTextures(const QVector<Opf::Texture>& textures)
{
    QTreeWidgetItem* categoryItem = new QTreeWidgetItem(this);
    categoryItem->setText(0, QString("Textures (%1)").arg(textures.size()));
    categoryItem->setData(0, Qt::UserRole, TextureCategory);

    for (int i = 0; i < textures.size(); i++)
    {
        const Opf::Texture& texture = textures[i];
        QTreeWidgetItem* item = new QTreeWidgetItem(categoryItem);
        item->setText(0, texture.name);
        item->setText(1, QString::number(texture.id));
        item->setText(2, QString("%1x%2").arg(texture.width).arg(texture.height));
        item->setData(0, Qt::UserRole, TextureItem);
        item->setData(0, Qt::UserRole + 1, i);
    }
}

void AssetTreeWidget::addMaterials(const QVector<Opf::Material>& materials)
{
    QTreeWidgetItem* categoryItem = new QTreeWidgetItem(this);
    categoryItem->setText(0, QString("Materials (%1)").arg(materials.size()));
    categoryItem->setData(0, Qt::UserRole, MaterialCategory);

    for (int i = 0; i < materials.size(); i++)
    {
        const Opf::Material& material = materials[i];
        QTreeWidgetItem* item = new QTreeWidgetItem(categoryItem);
        item->setText(0, material.name);
        item->setText(1, QString::number(material.id));
        item->setText(2, material.doubleSided ? "2-sided" : "1-sided");
        item->setData(0, Qt::UserRole, MaterialItem);
        item->setData(0, Qt::UserRole + 1, i);
    }
}

void AssetTreeWidget::addObjects(const QVector<Opf::Object*>& objects)
{
    QTreeWidgetItem* categoryItem = new QTreeWidgetItem(this);
    categoryItem->setText(0, QString("Objects (%1)").arg(objects.size()));
    categoryItem->setData(0, Qt::UserRole, ObjectCategory);

    QMap<QString, QVector<Opf::Object*>> categorized;
    for (Opf::Object* obj : objects)
    {
        if (obj)
        {
            QString category = obj->getCategory();
            categorized[category].append(obj);
        }
    }

    for (auto it = categorized.begin(); it != categorized.end(); ++it)
    {
        QTreeWidgetItem* subCategoryItem = new QTreeWidgetItem(categoryItem);
        subCategoryItem->setText(0, QString("%1 (%2)").arg(it.key()).arg(it.value().size()));

        for (Opf::Object* obj : it.value())
        {
            addObjectRecursive(subCategoryItem, obj);
        }
    }
}

void AssetTreeWidget::addObjectRecursive(QTreeWidgetItem* parent, Opf::Object* object)
{
    if (!object) return;

    QTreeWidgetItem* item = new QTreeWidgetItem(parent);

    QString displayName = object->name;
    if (object->hasLight) displayName = "* " + displayName;
    if (!object->customSettings.isEmpty()) displayName += QString(" [%1]").arg(object->customSettings.size());

    item->setText(0, displayName);
    item->setText(1, QString("%1/%2").arg(object->uniqueID).arg(object->projectID));
    item->setText(2, object->className);
    item->setData(0, Qt::UserRole, ObjectItem);
    item->setData(0, Qt::UserRole + 1, QVariant::fromValue((void*)object));

    for (Opf::Object* child : object->children)
    {
        addObjectRecursive(item, child);
    }
}

void AssetTreeWidget::keyPressEvent(QKeyEvent* event)
{
    QTreeWidget::keyPressEvent(event);

    if (event->key() == Qt::Key_Up || event->key() == Qt::Key_Down || event->key() == Qt::Key_Home || event->key() == Qt::Key_End || event->key() == Qt::Key_PageUp || event->key() == Qt::Key_PageDown)
    {
        QTreeWidgetItem* item = currentItem();
        if (item)
        {
            onItemClicked(item, 0);
        }
    }
}

void AssetTreeWidget::onItemClicked(QTreeWidgetItem* item, int column)
{
    Q_UNUSED(column);
    if (!item || !m_currentProject) return;

    int itemType = item->data(0, Qt::UserRole).toInt();

    if (itemType == ObjectItem)
    {
        Opf::Object* obj = static_cast<Opf::Object*>(item->data(0, Qt::UserRole + 1).value<void*>());
        emit objectSelected(obj);
    }
    else if (itemType == TextureItem)
    {
        int index = item->data(0, Qt::UserRole + 1).toInt();
        if (index >= 0 && index < m_currentProject->textures.size())
        {
            emit textureSelected(const_cast<Opf::Texture*>(&m_currentProject->textures[index]));
        }
    }
    else if (itemType == MaterialItem)
    {
        int index = item->data(0, Qt::UserRole + 1).toInt();
        if (index >= 0 && index < m_currentProject->materials.size())
        {
            emit materialSelected(const_cast<Opf::Material*>(&m_currentProject->materials[index]));
        }
    }
}

Opf::Object* AssetTreeWidget::getSelectedObject() const
{
    QTreeWidgetItem* item = currentItem();
    if (!item) return nullptr;

    if (item->data(0, Qt::UserRole).toInt() != ObjectItem) return nullptr;

    return static_cast<Opf::Object*>(item->data(0, Qt::UserRole + 1).value<void*>());
}

Opf::Texture* AssetTreeWidget::getSelectedTexture() const
{
    QTreeWidgetItem* item = currentItem();
    if (!item || !m_currentProject) return nullptr;

    if (item->data(0, Qt::UserRole).toInt() != TextureItem) return nullptr;

    int index = item->data(0, Qt::UserRole + 1).toInt();
    if (index >= 0 && index < m_currentProject->textures.size())
    {
        return const_cast<Opf::Texture*>(&m_currentProject->textures[index]);
    }
    return nullptr;
}

Opf::Material* AssetTreeWidget::getSelectedMaterial() const
{
    QTreeWidgetItem* item = currentItem();
    if (!item || !m_currentProject) return nullptr;

    if (item->data(0, Qt::UserRole).toInt() != MaterialItem) return nullptr;

    int index = item->data(0, Qt::UserRole + 1).toInt();
    if (index >= 0 && index < m_currentProject->materials.size())
    {
        return const_cast<Opf::Material*>(&m_currentProject->materials[index]);
    }
    return nullptr;
}

void AssetTreeWidget::setSearchFilter(const QString& filter)
{
    m_searchFilter = filter.toLower();
    applyFilters();
}

void AssetTreeWidget::setCategoryFilter(const QString& category)
{
    m_categoryFilter = category;
    applyFilters();
}

void AssetTreeWidget::setOnlyWithMeshes(bool enabled)
{
    m_onlyWithMeshes = enabled;
    applyFilters();
}

void AssetTreeWidget::applyFilters()
{
    if (!m_currentProject) return;

    QTreeWidgetItemIterator it(this);
    while (*it)
    {
        QTreeWidgetItem* item = *it;

        int itemType = item->data(0, Qt::UserRole).toInt();
        if (itemType == TextureCategory || itemType == MaterialCategory || itemType == ObjectCategory || itemType == ProjectInfo)
        {
            item->setHidden(false);
            ++it;
            continue;
        }

        bool visible = matchesFilter(item);
        item->setHidden(!visible);

        ++it;
    }
}

bool AssetTreeWidget::matchesFilter(QTreeWidgetItem* item)
{
    int itemType = item->data(0, Qt::UserRole).toInt();

    if (!m_searchFilter.isEmpty())
    {
        QString itemText = item->text(0).toLower();
        if (!itemText.contains(m_searchFilter))
        {
            return false;
        }
    }

    if (itemType == ObjectItem && m_categoryFilter != "All")
    {
        Opf::Object* obj = static_cast<Opf::Object*>(item->data(0, Qt::UserRole + 1).value<void*>());
        if (obj && obj->getCategory() != m_categoryFilter)
        {
            return false;
        }
    }

    if (m_onlyWithMeshes && itemType == ObjectItem)
    {
        Opf::Object* obj = static_cast<Opf::Object*>(item->data(0, Qt::UserRole + 1).value<void*>());
        if (obj && obj->meshes().isEmpty())
        {
            return false;
        }
    }

    return true;
}
