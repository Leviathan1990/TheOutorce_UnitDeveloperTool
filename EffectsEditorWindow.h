#ifndef EFFECTSEDITORWINDOW_H
#define EFFECTSEDITORWINDOW_H

#include <QMainWindow>
#include <QTabWidget>
#include <QListWidget>
#include <QSplitter>
#include <QLabel>
#include <QToolBar>

#include "EffectsStructs.h"
#include "EffectsParser.h"
#include "EffectsWriter.h"

class EmitterEditorWidget;
class ExplosionEditorWidget;
class ColorsEditorWidget;

class EffectsEditorWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit EffectsEditorWindow(QWidget* parent = nullptr);
    ~EffectsEditorWindow();

    // Open files directly
    void openDirectory(const QString& dirPath);
    void openFiles(const QStringList& files);

private slots:
    void onOpenDirectory();
    void onOpenFiles();
    void onSave();
    void onSaveAs();

    void onEmitterSelected(QListWidgetItem* item);
    void onExplosionSelected(QListWidgetItem* item);
    void onAddEmitter();
    void onRemoveEmitter();
    void onAddExplosion();
    void onRemoveExplosion();

    void onDataModified();

protected:
    void closeEvent(QCloseEvent* event) override;

private:
    Effects::EffectsProject m_project;
    Effects::EffectsParser m_parser;
    Effects::EffectsWriter m_writer;
    bool m_isModified;
    QString m_currentDir;

    // Main layout
    QTabWidget* m_tabWidget;

    // Emitters tab
    QWidget* m_emittersTab;
    QListWidget* m_emitterList;
    EmitterEditorWidget* m_emitterEditor;

    // Explosions tab
    QWidget* m_explosionsTab;
    QListWidget* m_explosionList;
    ExplosionEditorWidget* m_explosionEditor;

    // Colors tab
    ColorsEditorWidget* m_colorsEditor;

    // Status
    QLabel* m_statusLabel;
    QToolBar* m_toolBar;

    void setupUI();
    void setupMenus();
    void setupToolbar();
    void setupStatusBar();

    void updateEmitterList();
    void updateExplosionList();
    void updateTitle();
    void updateMaterialsList();

    void clearProject();
    bool maybeSave();
};

#endif // EFFECTSEDITORWINDOW_H
