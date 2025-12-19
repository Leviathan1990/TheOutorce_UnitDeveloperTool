#ifndef ASSETPREVIEWWIDGET_H
#define ASSETPREVIEWWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QTextEdit>
#include <QVBoxLayout>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsPixmapItem>
#include <QSlider>
#include <QPushButton>
#include <QTabWidget>
#include <QScrollArea>
#include "OpfStructs.h"
#include "CustomSettingsWidget.h"

class AssetPreviewWidget : public QWidget
{
    Q_OBJECT

public:
    explicit AssetPreviewWidget(QWidget *parent = nullptr);

    void showObject(Opf::Object* object);
    void showTexture(const Opf::Texture* texture);
    void showMaterial(const Opf::Material* material);
    void clear();

    void setAvailableUnits(const QStringList& units);

signals:
    void objectModified();

private slots:
    void onZoomChanged(int value);
    void onResetZoom();
    void onToggleAlpha();

private:
    void setupUI();
    void updateStatistics(const QString& stats);
    void showTexturePreview(const Opf::Texture* texture);
    void updateTexturePreview();
    QImage createTextureImage(const Opf::Texture* texture, bool alphaOnly);

    // Main tab widget for everything
    QTabWidget* m_mainTabs;

    // Details tab
    QWidget* m_detailsTab;
    QLabel* m_titleLabel;
    QTextEdit* m_detailsText;

    // Settings tab (for objects)
    QWidget* m_settingsTab;
    QTabWidget* m_settingsTabs;
    CustomSettingsWidget* m_customSettingsWidget;
    CanBuildUnitWidget* m_canBuildWidget;

    // Statistics tab
    QWidget* m_statsTab;
    QLabel* m_statsLabel;

    // Texture preview tab
    QWidget* m_textureTab;
    QGraphicsView* m_textureView;
    QGraphicsScene* m_textureScene;
    QGraphicsPixmapItem* m_textureItem;
    QSlider* m_zoomSlider;
    QPushButton* m_resetZoomButton;
    QPushButton* m_toggleAlphaButton;

    const Opf::Texture* m_currentTexture;
    bool m_showingAlpha;
    double m_currentZoom;
};

#endif // ASSETPREVIEWWIDGET_H
