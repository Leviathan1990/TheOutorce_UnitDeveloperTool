#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QToolBar>
#include <QStatusBar>
#include <QLabel>
#include <QLineEdit>
#include <QComboBox>
#include <QCheckBox>
#include <QMenu>

#include "OpfStructs.h"
#include "OpfParser.h"
#include "OpfExporter.h"
#include "AssetTreeWidget.h"
#include "AssetPreviewWidget.h"

//  Forward declaration for Effect Editor

class EffectsEditorWindow;

//  Forward declaration for AI editor
class AIEditorWindow;

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onOpenFile();
    void onOpenRecentFile();
    void onClearRecentFiles();
    void onExtractSelected();
    void onExtractAll();
    void onExportTemplates();
    void onPreferences();
    void onAbout();

    void onObjectSelected(Opf::Object* object);
    void onTextureSelected(Opf::Texture* texture);
    void onMaterialSelected(Opf::Material* material);

    void onSearchTextChanged(const QString& text);
    void onCategoryFilterChanged(int index);
    void onMeshFilterToggled(bool checked);

    void onObjectModified();

    //  opf save
    void onSaveFile();
    void onSaveFileAs();

    //  Effects Editor
    void onOpenEffectsEditor();
    void onOpenEffectsDirectory();
    void onOpenEffectsFiles();

    //  AI Editor
    void onOpenAIEditor();

    //  Filelist generator
    void onExportAssetList();

private:
    Ui::MainWindow *ui;

    Opf::PackedProject* m_project;
    Opf::OpfParser m_parser;
    Opf::OpfExporter m_exporter;
    QString m_currentFilePath;

    AssetTreeWidget* m_treeWidget;
    AssetPreviewWidget* m_previewWidget;
    QToolBar* m_toolBar;
    QLabel* m_statusLabel;
    QLabel* m_fileInfoLabel;

    QLineEdit* m_searchBox;
    QComboBox* m_categoryFilter;
    QCheckBox* m_meshFilterCheckbox;
    QMenu* m_recentFilesMenu;

    //  Effects Editor window (single instance)
    EffectsEditorWindow* m_effectsEditor;

    void setupUI();
    void setupMenus();
    void setupToolbar();
    void setupStatusBar();
    void updateStatusBar();
    void clearProject();
    void updateRecentFilesMenu();
    void openFile(const QString& filename);

    // Opf save
    bool m_isModified = false;
    void setModified(bool modified);

    //  AI Editor
    AIEditorWindow* m_aiEditor;

};

#endif // MAINWINDOW_H
