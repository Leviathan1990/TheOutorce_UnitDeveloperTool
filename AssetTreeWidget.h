#ifndef ASSETTREEWIDGET_H
#define ASSETTREEWIDGET_H

#include <QTreeWidget>
#include "OpfStructs.h"

class AssetTreeWidget : public QTreeWidget
{
    Q_OBJECT

public:
    explicit AssetTreeWidget(QWidget *parent = nullptr);

    void loadProject(const Opf::PackedProject& project);
    void clear();

    Opf::Object* getSelectedObject() const;
    Opf::Texture* getSelectedTexture() const;
    Opf::Material* getSelectedMaterial() const;

    void setSearchFilter(const QString& filter);
    void setCategoryFilter(const QString& category);
    void setOnlyWithMeshes(bool enabled);

    enum ItemType
    {
        ProjectInfo,
        TextureCategory,
        MaterialCategory,
        ObjectCategory,
        TextureItem,
        MaterialItem,
        ObjectItem
    };

signals:
    void objectSelected(Opf::Object* object);
    void textureSelected(Opf::Texture* texture);
    void materialSelected(Opf::Material* material);

protected:
    void keyPressEvent(QKeyEvent* event) override;

private slots:
    void onItemClicked(QTreeWidgetItem* item, int column);

private:
    void setupUI();
    void addProjectInfo(const Opf::PackedProject& project);
    void addTextures(const QVector<Opf::Texture>& textures);
    void addMaterials(const QVector<Opf::Material>& materials);
    void addObjects(const QVector<Opf::Object*>& objects);
    void addObjectRecursive(QTreeWidgetItem* parent, Opf::Object* object);

    void applyFilters();
    bool matchesFilter(QTreeWidgetItem* item);

    const Opf::PackedProject* m_currentProject;
    QString m_searchFilter;
    QString m_categoryFilter;
    bool m_onlyWithMeshes;
};

#endif // ASSETTREEWIDGET_H
