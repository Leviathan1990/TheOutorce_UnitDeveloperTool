#include "MainWindow.h"
#include "OpfWriter.h"

//  Custom headers
#include "ui_MainWindow.h"
#include "SettingsDialog.h"
#include "SettingsManager.h"
#include "EffectsEditorWindow.h"
#include "AIEditorWindow.h"


#include <QFileDialog>
#include <QMessageBox>
#include <QSplitter>
#include <QFileInfo>
#include <QDir>
#include <QDesktopServices>
#include <QUrl>
#include <QProgressDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QApplication>
#include <QDateTime>
#include <QCoreApplication>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow), m_project(nullptr), m_effectsEditor(nullptr),m_aiEditor(nullptr)
{
    ui->setupUi(this);

    setWindowTitle("The Outforce - UnitDeveloper Tool. v3.0");
    resize(1400, 900);

    setupUI();
    setupMenus();
    setupToolbar();
    setupStatusBar();
}

MainWindow::~MainWindow()
{
    clearProject();
    delete ui;
}

void MainWindow::setupUI()
{
    QWidget* centralWidget = new QWidget(this);
    QVBoxLayout* mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    // Filter toolbar
    QWidget* filterWidget = new QWidget(this);
    QHBoxLayout* filterLayout = new QHBoxLayout(filterWidget);
    filterLayout->setContentsMargins(5, 5, 5, 5);

    filterLayout->addWidget(new QLabel("Search:", this));

    m_searchBox = new QLineEdit(this);
    m_searchBox->setPlaceholderText("Type to filter assets...");
    m_searchBox->setClearButtonEnabled(true);
    connect(m_searchBox, &QLineEdit::textChanged, this, &MainWindow::onSearchTextChanged);
    filterLayout->addWidget(m_searchBox, 1);

    filterLayout->addWidget(new QLabel("Category:", this));

    m_categoryFilter = new QComboBox(this);
    m_categoryFilter->addItems({"All", "Units", "Buildings", "Weapons", "Environment", "Other"});
    connect(m_categoryFilter, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::onCategoryFilterChanged);
    filterLayout->addWidget(m_categoryFilter);

    m_meshFilterCheckbox = new QCheckBox("Only with meshes", this);
    connect(m_meshFilterCheckbox, &QCheckBox::toggled, this, &MainWindow::onMeshFilterToggled);
    filterLayout->addWidget(m_meshFilterCheckbox);

    mainLayout->addWidget(filterWidget);

    // Main splitter
    QSplitter* mainSplitter = new QSplitter(Qt::Horizontal, this);

    m_treeWidget = new AssetTreeWidget(this);
    connect(m_treeWidget, &AssetTreeWidget::objectSelected, this, &MainWindow::onObjectSelected);
    connect(m_treeWidget, &AssetTreeWidget::textureSelected, this, &MainWindow::onTextureSelected);
    connect(m_treeWidget, &AssetTreeWidget::materialSelected, this, &MainWindow::onMaterialSelected);

    mainSplitter->addWidget(m_treeWidget);

    m_previewWidget = new AssetPreviewWidget(this);
    connect(m_previewWidget, &AssetPreviewWidget::objectModified, this, &MainWindow::onObjectModified);
    mainSplitter->addWidget(m_previewWidget);

    mainSplitter->setSizes(QList<int>() << 600 << 800);

    mainLayout->addWidget(mainSplitter, 1);

    setCentralWidget(centralWidget);

}

void MainWindow::setupMenus()
{
    QMenu* fileMenu = menuBar()->addMenu(tr("&File"));

    QAction* openAction = fileMenu->addAction(tr("&Open PackedProject.opf..."));
    openAction->setShortcut(QKeySequence::Open);
    connect(openAction, &QAction::triggered, this, &MainWindow::onOpenFile);

    m_recentFilesMenu = fileMenu->addMenu(tr("Open &Recent"));
    updateRecentFilesMenu();

    fileMenu->addSeparator();

    //  Save opf
    QAction* saveAction = new QAction("&Save", this);
    saveAction->setShortcut(QKeySequence::Save);  // Ctrl+S
    connect(saveAction, &QAction::triggered, this, &MainWindow::onSaveFile);
    fileMenu->addAction(saveAction);

    QAction* saveAsAction = new QAction("Save &As...", this);
    saveAsAction->setShortcut(QKeySequence::SaveAs);  // Ctrl+Shift+S
    connect(saveAsAction, &QAction::triggered, this, &MainWindow::onSaveFileAs);
    fileMenu->addAction(saveAsAction);

    fileMenu->addSeparator();

    QAction* extractSelAction = fileMenu->addAction(tr("&Extract Selected..."));
    extractSelAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_E));
    connect(extractSelAction, &QAction::triggered, this, &MainWindow::onExtractSelected);

    QAction* extractAllAction = fileMenu->addAction(tr("Extract &All..."));
    extractAllAction->setShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_E));
    connect(extractAllAction, &QAction::triggered, this, &MainWindow::onExtractAll);

    fileMenu->addSeparator();

    QAction* exportTemplatesAction = fileMenu->addAction(tr("Export &Templates.json..."));
    exportTemplatesAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_T));
    connect(exportTemplatesAction, &QAction::triggered, this, &MainWindow::onExportTemplates);

    fileMenu->addSeparator();

    //  ExportList
    QAction* exportListAction = fileMenu->addAction(tr("Export Asset &List..."));
    exportListAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_L));
    connect(exportListAction, &QAction::triggered, this, &MainWindow::onExportAssetList);

    QAction* exitAction = fileMenu->addAction(tr("E&xit"));
    exitAction->setShortcut(QKeySequence::Quit);
    connect(exitAction, &QAction::triggered, this, &QWidget::close);

    QMenu* settingsMenu = menuBar()->addMenu(tr("&Settings"));
    QAction* preferencesAction = settingsMenu->addAction(tr("&Preferences..."));
    preferencesAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_Comma));
    connect(preferencesAction, &QAction::triggered, this, &MainWindow::onPreferences);

    //  Effects menu
    QMenu* effectsMenu = menuBar()->addMenu(tr("&Effects"));

    QAction* openEffectsEditorAction = effectsMenu->addAction(tr("Open Effects &Editor"));
    openEffectsEditorAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_F));
    connect(openEffectsEditorAction, &QAction::triggered, this, &MainWindow::onOpenEffectsEditor);

    effectsMenu->addSeparator();

    //  AI Editor

    QMenu* aiMenu = menuBar()->addMenu(tr("&AI"));

    QAction* openAIEditorAction = aiMenu->addAction(tr("Open AI &Editor"));
    openAIEditorAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_I));
    connect(openAIEditorAction, &QAction::triggered, this, &MainWindow::onOpenAIEditor);



    QAction* openEffectsDirAction = effectsMenu->addAction(tr("Open Effects &Directory..."));
    connect(openEffectsDirAction, &QAction::triggered, this, &MainWindow::onOpenEffectsDirectory);

    QAction* openEffectsFilesAction = effectsMenu->addAction(tr("Open Effects &Files..."));
    connect(openEffectsFilesAction, &QAction::triggered, this, &MainWindow::onOpenEffectsFiles);

    //  Help menu

    QMenu* helpMenu = menuBar()->addMenu(tr("&Help"));
    QAction* aboutAction = helpMenu->addAction(tr("&About"));
    connect(aboutAction, &QAction::triggered, this, &MainWindow::onAbout);
}

void MainWindow::setupToolbar()
{
    m_toolBar = addToolBar(tr("Main Toolbar"));
    m_toolBar->setMovable(false);
    m_toolBar->setIconSize(QSize(24, 24));

    QAction* openAction = m_toolBar->addAction("Open");
    openAction->setToolTip("Open PackedProject.opf");
    connect(openAction, &QAction::triggered, this, &MainWindow::onOpenFile);

    m_toolBar->addSeparator();

    QAction* extractSelAction = m_toolBar->addAction("Extract");
    extractSelAction->setToolTip("Extract selected asset");
    connect(extractSelAction, &QAction::triggered, this, &MainWindow::onExtractSelected);

    QAction* extractAllAction = m_toolBar->addAction("Export All");
    extractAllAction->setToolTip("Export all assets");
    connect(extractAllAction, &QAction::triggered, this, &MainWindow::onExtractAll);

    m_toolBar->addSeparator();

    QAction* templatesAction = m_toolBar->addAction("Templates");
    templatesAction->setToolTip("Export templates.json");
    connect(templatesAction, &QAction::triggered, this, &MainWindow::onExportTemplates);
}

void MainWindow::setupStatusBar()
{
    m_statusLabel = new QLabel("Ready", this);
    statusBar()->addWidget(m_statusLabel);

    statusBar()->addPermanentWidget(new QLabel(" | ", this));

    m_fileInfoLabel = new QLabel("No file loaded", this);
    statusBar()->addPermanentWidget(m_fileInfoLabel);
}

void MainWindow::updateStatusBar()
{
    if (!m_project)
    {
        m_fileInfoLabel->setText("No file loaded");
        return;
    }

    QString info = QString("Objects: %1 | Textures: %2 | Materials: %3").arg(m_project->objects.size()).arg(m_project->textures.size()).arg(m_project->materials.size());

    m_fileInfoLabel->setText(info);
}

void MainWindow::onOpenFile()
{
    QString filename = QFileDialog::getOpenFileName(this, tr("Open PackedProject.opf"), QString(), tr("Outforce Project Files (*.opf);;All Files (*)"));

    if (!filename.isEmpty())
    {
        openFile(filename);
    }
}

void MainWindow::openFile(const QString& filename)
{
    clearProject();

    m_project = new Opf::PackedProject();

    m_statusLabel->setText("Parsing PackedProject.opf...");
    QApplication::processEvents();

    if (!m_parser.parse(filename, *m_project))
    {
        QMessageBox::critical(this, tr("Error"), tr("Failed to parse file:\n%1").arg(m_parser.lastError()));
        clearProject();
        m_statusLabel->setText("Failed to load file");
        return;
    }

    m_currentFilePath = filename;

    SettingsManager::instance().addRecentFile(filename);
    updateRecentFilesMenu();

    m_treeWidget->loadProject(*m_project);

    // Set available units for CanBuildUnit widget
    m_previewWidget->setAvailableUnits(m_project->getAllUnitNames());

    updateStatusBar();
    setWindowTitle(QString("The Outforce - UnitDeveloper Tool. v3.1 - %1").arg(QFileInfo(filename).fileName()));

    m_statusLabel->setText(QString("Loaded: %1").arg(m_project->projectName));

    QMessageBox::information(this, tr("Success"), tr("Successfully loaded:\n\n"
           "Project: %1\n"
           "Objects: %2\n"
           "Textures: %3\n"
           "Materials: %4\n"
           "Dependencies: %5\n"
           "Events: %6").arg(m_project->projectName).arg(m_project->objects.size()).arg(m_project->textures.size()).arg(m_project->materials.size()).arg(m_project->dependencies.size()).arg(m_project->eventDescs.size()));
}

void MainWindow::onExtractSelected()
{
    if (!m_project)
    {
        QMessageBox::information(this, tr("Info"), tr("No project loaded. Please open a PackedProject.opf file first."));
        return;
    }

    Opf::Object* obj = m_treeWidget->getSelectedObject();
    Opf::Texture* tex = m_treeWidget->getSelectedTexture();
    Opf::Material* mat = m_treeWidget->getSelectedMaterial();

    if (!obj && !tex && !mat)
    {
        QMessageBox::information(this, tr("Info"), tr("Please select an asset to extract."));
        return;
    }

    QString defaultName;
    if (obj) defaultName = obj->name + ".json";
    else if (tex) defaultName = tex->name + ".json";
    else if (mat) defaultName = mat->name + ".json";

    QString filename = QFileDialog::getSaveFileName(this, tr("Extract Asset"), defaultName, tr("JSON Files (*.json);;All Files (*)"));

    if (filename.isEmpty()) return;

    bool success = false;
    if (obj)
    {
        success = m_exporter.exportObject(*obj, filename);
    }

    else if (tex)
    {
        success = m_exporter.exportTexture(*tex, filename);
    }

    if (success)
    {
        m_statusLabel->setText(QString("Extracted to: %1").arg(filename));
        QMessageBox::information(this, tr("Success"), tr("Asset extracted successfully!"));
    }

    else
    {
        QMessageBox::critical(this, tr("Error"),tr("Failed to extract asset:\n%1").arg(m_exporter.lastError()));
    }
}

void MainWindow::onExtractAll()
{
    if (!m_project)
    {
        QMessageBox::information(this, tr("Info"), tr("No project loaded. Please open a PackedProject.opf file first."));
        return;
    }

    QString directory = QFileDialog::getExistingDirectory(this, tr("Select Export Directory"), QString(),QFileDialog::ShowDirsOnly);

    if (directory.isEmpty()) return;

    int totalSteps = m_project->objects.size() + m_project->textures.size() + m_project->materials.size() + 5;

    QProgressDialog progress(tr("Exporting assets..."), tr("Cancel"), 0, totalSteps, this);
    progress.setWindowModality(Qt::WindowModal);
    progress.setMinimumDuration(0);

    m_statusLabel->setText("Exporting all assets...");

    bool success = m_exporter.exportAll(*m_project, directory, &progress);

    if (progress.wasCanceled())
    {
        m_statusLabel->setText("Export canceled");
        return;
    }

    if (success)
    {
        m_statusLabel->setText(QString("Exported all to: %1").arg(directory));

        QMessageBox::StandardButton reply = QMessageBox::question(this, tr("Success"),tr("All assets exported successfully!\n\nOpen folder?"),QMessageBox::Yes | QMessageBox::No);

        if (reply == QMessageBox::Yes)
        {
            QDesktopServices::openUrl(QUrl::fromLocalFile(directory));
        }
    }

    else
    {
        QMessageBox::critical(this, tr("Error"),tr("Failed to export assets:\n%1").arg(m_exporter.lastError()));
    }
}

void MainWindow::onExportTemplates()
{
    if (!m_project)
    {
        QMessageBox::information(this, tr("Info"),tr("No project loaded."));
        return;
    }

    QString filename = QFileDialog::getSaveFileName(this, tr("Export templates.json"), "templates.json",tr("JSON Files (*.json);;All Files (*)"));

    if (filename.isEmpty()) return;

    if (m_exporter.exportTemplatesToJson(*m_project, filename))
    {
        m_statusLabel->setText(QString("Templates exported to: %1").arg(filename));
        QMessageBox::information(this, tr("Success"),tr("Templates exported successfully!\n\nFile: %1").arg(filename));
    }

    else
    {
        QMessageBox::critical(this, tr("Error"), tr("Failed to export templates:\n%1").arg(m_exporter.lastError()));
    }
}

void MainWindow::onPreferences()
{
    SettingsDialog dialog(this);
    dialog.exec();
}

void MainWindow::onAbout()
{
    QMessageBox::about(this, tr("About The Outforce - UnitDeveloper Tool."),
        tr("<h3>The Outforce - UnitDeveloper Tool. v3.1</h3>"
           "<p>A complete asset explorer and extractor for The Outforce game.</p>"
           "<p><b>Features:</b></p>"
           "<ul>"
           "<li>Browse all assets (Objects, Textures, Materials)</li>"
           "<li>Image Inspector</li>"
           "<li>View and edit Custom Settings (Game Logic)</li>"
           "<li>Edit CanBuildUnit relationships</li>"
           "<li>Export to OBJ, PNG, JSON</li>"
           "<li>Generate templates.json for Map Editor</li>"
           "<li>Emitter Editor</li>"
           "<li>Special Effects Editor</li>"
           "<li>AI Editor</li>"
           "<li>BuildFitness Editor</li>"
           "</ul>"
           "<p><b>100% Header Implementation</b></p>"
           "<p>Developer: Krisztian Kispeti</p>"));
}

void MainWindow::onObjectSelected(Opf::Object* object)
{
    m_previewWidget->showObject(object);

    if (object)
    {
        m_statusLabel->setText(QString("Selected: %1 [ID:%2, Settings:%3]").arg(object->name).arg(object->uniqueID).arg(object->customSettings.size()));
    }
}

void MainWindow::onTextureSelected(Opf::Texture* texture)
{
    m_previewWidget->showTexture(texture);

    if (texture)
    {
        m_statusLabel->setText(QString("Selected: %1 [%2x%3]").arg(texture->name).arg(texture->width).arg(texture->height));
    }
}

void MainWindow::onMaterialSelected(Opf::Material* material)
{
    m_previewWidget->showMaterial(material);

    if (material)
    {
        m_statusLabel->setText(QString("Selected: %1 [ID:%2]").arg(material->name).arg(material->id));
    }
}

void MainWindow::onSearchTextChanged(const QString& text)
{
    m_treeWidget->setSearchFilter(text);
}

void MainWindow::onCategoryFilterChanged(int index)
{
    Q_UNUSED(index);
    m_treeWidget->setCategoryFilter(m_categoryFilter->currentText());
}

void MainWindow::onMeshFilterToggled(bool checked)
{
    m_treeWidget->setOnlyWithMeshes(checked);
}

void MainWindow::onObjectModified()
{
    setModified(true);
    m_statusLabel->setText("Object modified - unsaved changes");
}

void MainWindow::clearProject()
{
    if (m_project)
    {
        delete m_project;
        m_project = nullptr;
    }

    m_treeWidget->clear();
    m_previewWidget->clear();
    m_currentFilePath.clear();

    updateStatusBar();
    setWindowTitle("The Outforce - UnitDeveloper Tool. v3.1");
}

void MainWindow::updateRecentFilesMenu()
{
    m_recentFilesMenu->clear();

    QStringList recentFiles = SettingsManager::instance().recentFiles();

    if (recentFiles.isEmpty())
    {
        QAction* noFilesAction = m_recentFilesMenu->addAction(tr("No recent files"));
        noFilesAction->setEnabled(false);
        return;
    }

    for (const QString& filepath : recentFiles)
    {
        QFileInfo fileInfo(filepath);
        QAction* action = m_recentFilesMenu->addAction(fileInfo.fileName());
        action->setData(filepath);
        action->setToolTip(filepath);
        action->setEnabled(fileInfo.exists());
        connect(action, &QAction::triggered, this, &MainWindow::onOpenRecentFile);
    }

    m_recentFilesMenu->addSeparator();

    QAction* clearAction = m_recentFilesMenu->addAction(tr("Clear Recent Files"));
    connect(clearAction, &QAction::triggered, this, &MainWindow::onClearRecentFiles);
}

void MainWindow::onOpenRecentFile()
{
    QAction* action = qobject_cast<QAction*>(sender());
    if (action)
    {
        openFile(action->data().toString());
    }
}

void MainWindow::onClearRecentFiles()
{
    SettingsManager::instance().clearRecentFiles();
    updateRecentFilesMenu();
}


// ============================================================================
// SAVE FUNCTIONS
// ============================================================================

void MainWindow::onSaveFile()
{
    if (!m_project)
    {
        QMessageBox::information(this, tr("Info"), tr("No project loaded."));
        return;
    }

    if (m_currentFilePath.isEmpty())
    {
        onSaveFileAs();
        return;
    }

    // Confirm overwrite
    QMessageBox::StandardButton reply = QMessageBox::question(this, tr("Save Project"),tr("Save changes to:\n%1\n\nA backup will be created automatically.").arg(m_currentFilePath), QMessageBox::Yes | QMessageBox::No);

    if (reply != QMessageBox::Yes)
    {
        return;
    }

    m_statusLabel->setText("Saving...");
    QApplication::processEvents();

    Opf::OpfWriter writer;
    if (writer.writeBackup(m_currentFilePath, *m_project))
    {
        setModified(false);
        m_statusLabel->setText(QString("Saved: %1").arg(m_currentFilePath));
        QMessageBox::information(this, tr("Success"),tr("Project saved successfully!\n\nBackup created in the same folder."));
    }

    else
    {
        QMessageBox::critical(this, tr("Error"), tr("Failed to save project:\n%1").arg(writer.lastError()));
        m_statusLabel->setText("Save failed");
    }
}

void MainWindow::onSaveFileAs()
{
    if (!m_project)
    {
        QMessageBox::information(this, tr("Info"),tr("No project loaded."));
        return;
    }

    QString filename = QFileDialog::getSaveFileName(this, tr("Save Project As"), m_currentFilePath.isEmpty() ? "PackedProject.opf" : m_currentFilePath, tr("Outforce Project Files (*.opf);;All Files (*)"));

    if (filename.isEmpty())
    {
        return;
    }

    m_statusLabel->setText("Saving...");
    QApplication::processEvents();

    Opf::OpfWriter writer;
    if (writer.write(filename, *m_project))
    {
        m_currentFilePath = filename;
        setModified(false);
        setWindowTitle(QString("The Outforce - UnitDeveloper Tool. v3.1 - %1").arg(QFileInfo(filename).fileName()));
        m_statusLabel->setText(QString("Saved: %1").arg(filename));
        QMessageBox::information(this, tr("Success"),tr("Project saved successfully!"));
    }

    else
    {
        QMessageBox::critical(this, tr("Error"), tr("Failed to save project:\n%1").arg(writer.lastError()));
        m_statusLabel->setText("Save failed");
    }
}

void MainWindow::setModified(bool modified)
{
    m_isModified = modified;

    QString title = windowTitle();
    if (modified && !title.startsWith("*"))
    {
        setWindowTitle("*" + title);
    }

    else if (!modified && title.startsWith("*"))
    {
        setWindowTitle(title.mid(1));
    }
}


// ============================================================================
// EFFECTS EDITOR FUNCTIONS
// ============================================================================

void MainWindow::onOpenEffectsEditor()
{
    if (!m_effectsEditor)
    {
        m_effectsEditor = new EffectsEditorWindow(this);
        m_effectsEditor->setAttribute(Qt::WA_DeleteOnClose, false);
    }

    m_effectsEditor->show();
    m_effectsEditor->raise();
    m_effectsEditor->activateWindow();
}

void MainWindow::onOpenEffectsDirectory()
{
    QString dir = QFileDialog::getExistingDirectory(this, tr("Open Effects Directory"), QDir::homePath());
    if (dir.isEmpty()) return;

    // Create editor if needed and open directory
    if (!m_effectsEditor)
    {
        m_effectsEditor = new EffectsEditorWindow(this);
        m_effectsEditor->setAttribute(Qt::WA_DeleteOnClose, false);
    }

    m_effectsEditor->openDirectory(dir);
    m_effectsEditor->show();
    m_effectsEditor->raise();
    m_effectsEditor->activateWindow();
}

void MainWindow::onOpenEffectsFiles()
{
    QStringList files = QFileDialog::getOpenFileNames(this, tr("Open Effects Files"), QDir::homePath(), tr("Config files (*.cfg);;All files (*)"));
    if (files.isEmpty()) return;

    // Create editor if needed and open files
    if (!m_effectsEditor)
    {
        m_effectsEditor = new EffectsEditorWindow(this);
        m_effectsEditor->setAttribute(Qt::WA_DeleteOnClose, false);
    }

    m_effectsEditor->openFiles(files);
    m_effectsEditor->show();
    m_effectsEditor->raise();
    m_effectsEditor->activateWindow();
}


void MainWindow::onOpenAIEditor()
{
    if (!m_aiEditor)
    {
        m_aiEditor = new AIEditorWindow(this);
        m_aiEditor->setAttribute(Qt::WA_DeleteOnClose, false);

        // Pass available units from loaded project
        if (m_project)
        {
            m_aiEditor->setAvailableUnits(m_project->getAllUnitNames());
        }
    }

    m_aiEditor->show();
    m_aiEditor->raise();
    m_aiEditor->activateWindow();
}


void MainWindow::onExportAssetList()
{
    if (!m_project)
    {
        QMessageBox::information(this, tr("Info"), tr("No project loaded."));
        return;
    }

    // Get application directory and create Dat folder
    QString appDir = QCoreApplication::applicationDirPath();
    QString datDir = appDir + "/Dat";

    QDir dir(datDir);
    if (!dir.exists())
    {
        dir.mkpath(".");
    }

    // Generate default filename with project name and timestamp
    QString timestamp = QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss");
    QString defaultFilename = QString("%1/AssetList_%2_%3.txt").arg(datDir).arg(m_project->projectName).arg(timestamp);

    // Ask user for save location
    QString filename = QFileDialog::getSaveFileName(
        this,
        tr("Export Asset List"),
        defaultFilename,
        tr("Text Files (*.txt);;All Files (*)")
        );

    if (filename.isEmpty())
    {
        return;
    }

    m_statusLabel->setText("Exporting asset list...");
    QApplication::processEvents();

    if (m_exporter.exportAssetListToTxt(*m_project, filename))
    {
        m_statusLabel->setText(QString("Asset list exported to: %1").arg(filename));

        QMessageBox::StandardButton reply = QMessageBox::question(
            this,
            tr("Success"),
            tr("Asset list exported successfully!\n\nOpen file?"),
            QMessageBox::Yes | QMessageBox::No
            );

        if (reply == QMessageBox::Yes)
        {
            QDesktopServices::openUrl(QUrl::fromLocalFile(filename));
        }
    }
    else
    {
        QMessageBox::critical(this, tr("Error"), tr("Failed to export asset list:\n%1").arg(m_exporter.lastError()));
        m_statusLabel->setText("Export failed");
    }
}
