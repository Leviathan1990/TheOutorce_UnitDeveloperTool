#ifndef AIEDITORWINDOW_H
#define AIEDITORWINDOW_H

#include <QMainWindow>
#include <QTabWidget>
#include <QComboBox>
#include <QLabel>
#include <QToolBar>
#include "AIStructs.h"
#include "AIParser.h"
#include "AIWriter.h"

namespace AI {
class InitParamsWidget;
class BuildFitnessWidget;
class MaxUnitsWidget;
}

class AIEditorWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit AIEditorWindow(QWidget* parent = nullptr);
    ~AIEditorWindow();

    void openDirectory(const QString& dirPath);
    void setAvailableUnits(const QStringList& units);

private slots:
    void onOpenDirectory();
    void onSave();
    void onSaveAll();

    void onRaceChanged(int index);
    void onDifficultyChanged(int index);

    void onDataModified();

protected:
    void closeEvent(QCloseEvent* event) override;

private:
    AI::AIProject m_project;
    AI::AIParser m_parser;
    AI::AIWriter m_writer;
    bool m_isModified;
    QStringList m_availableUnits;

    // Selectors
    QComboBox* m_raceCombo;
    QComboBox* m_difficultyCombo;

    // Tabs
    QTabWidget* m_tabWidget;
    AI::InitParamsWidget* m_initParamsWidget;
    AI::BuildFitnessWidget* m_buildFitnessWidget;
    AI::MaxUnitsWidget* m_maxUnitsWidget;

    // Status
    QLabel* m_statusLabel;
    QToolBar* m_toolBar;

    void setupUI();
    void setupMenus();
    void setupToolbar();
    void setupStatusBar();
    void updateTitle();
    void clearProject();
    void loadCurrentFiles();
    bool maybeSave();
};

#endif // AIEDITORWINDOW_H
