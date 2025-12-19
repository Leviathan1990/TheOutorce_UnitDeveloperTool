#include "EffectsEditorWindow.h"
#include "EmitterEditorWidget.h"
#include "ExplosionEditorWidget.h"
#include "ColorsEditorWidget.h"

#include <QMenuBar>
#include <QStatusBar>
#include <QFileDialog>
#include <QMessageBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSplitter>
#include <QPushButton>
#include <QDir>
#include <QCloseEvent>
#include <QApplication>

EffectsEditorWindow::EffectsEditorWindow(QWidget* parent)
    : QMainWindow(parent), m_isModified(false)
{
    setWindowTitle("Effects Editor");
    resize(1100, 750);

    setupUI();
    setupMenus();
    setupToolbar();
    setupStatusBar();
    updateTitle();
}

EffectsEditorWindow::~EffectsEditorWindow() {
    clearProject();
}

void EffectsEditorWindow::setupUI() {
    m_tabWidget = new QTabWidget(this);
    setCentralWidget(m_tabWidget);

    // ========== EMITTERS TAB ==========
    m_emittersTab = new QWidget();
    QHBoxLayout* emittersLayout = new QHBoxLayout(m_emittersTab);

    QWidget* emitterListPanel = new QWidget();
    QVBoxLayout* listLayout = new QVBoxLayout(emitterListPanel);
    listLayout->setContentsMargins(0, 0, 0, 0);

    m_emitterList = new QListWidget();
    connect(m_emitterList, &QListWidget::itemClicked, this, &EffectsEditorWindow::onEmitterSelected);
    listLayout->addWidget(m_emitterList);

    QHBoxLayout* emitterBtnLayout = new QHBoxLayout();
    QPushButton* addEmitterBtn = new QPushButton("Add");
    connect(addEmitterBtn, &QPushButton::clicked, this, &EffectsEditorWindow::onAddEmitter);
    emitterBtnLayout->addWidget(addEmitterBtn);

    QPushButton* removeEmitterBtn = new QPushButton("Remove");
    connect(removeEmitterBtn, &QPushButton::clicked, this, &EffectsEditorWindow::onRemoveEmitter);
    emitterBtnLayout->addWidget(removeEmitterBtn);
    listLayout->addLayout(emitterBtnLayout);

    m_emitterEditor = new EmitterEditorWidget();
    connect(m_emitterEditor, &EmitterEditorWidget::emitterModified, this, &EffectsEditorWindow::onDataModified);

    QSplitter* emitterSplitter = new QSplitter(Qt::Horizontal);
    emitterSplitter->setChildrenCollapsible(false);
    emitterSplitter->addWidget(emitterListPanel);
    emitterSplitter->addWidget(m_emitterEditor);
    emitterSplitter->setSizes({280, 720});
    emitterSplitter->setStretchFactor(0, 0);
    emitterSplitter->setStretchFactor(1, 1);

    emittersLayout->addWidget(emitterSplitter);
    m_tabWidget->addTab(m_emittersTab, "Emitters");

    // ========== EXPLOSIONS TAB ==========
    m_explosionsTab = new QWidget();
    QHBoxLayout* explosionsLayout = new QHBoxLayout(m_explosionsTab);

    QWidget* explosionListPanel = new QWidget();
    QVBoxLayout* exploListLayout = new QVBoxLayout(explosionListPanel);
    exploListLayout->setContentsMargins(0, 0, 0, 0);

    m_explosionList = new QListWidget();
    connect(m_explosionList, &QListWidget::itemClicked, this, &EffectsEditorWindow::onExplosionSelected);
    exploListLayout->addWidget(m_explosionList);

    QHBoxLayout* exploBtnLayout = new QHBoxLayout();
    QPushButton* addExploBtn = new QPushButton("Add");
    connect(addExploBtn, &QPushButton::clicked, this, &EffectsEditorWindow::onAddExplosion);
    exploBtnLayout->addWidget(addExploBtn);

    QPushButton* removeExploBtn = new QPushButton("Remove");
    connect(removeExploBtn, &QPushButton::clicked, this, &EffectsEditorWindow::onRemoveExplosion);
    exploBtnLayout->addWidget(removeExploBtn);
    exploListLayout->addLayout(exploBtnLayout);

    m_explosionEditor = new ExplosionEditorWidget();
    connect(m_explosionEditor, &ExplosionEditorWidget::explosionModified, this, &EffectsEditorWindow::onDataModified);

    QSplitter* exploSplitter = new QSplitter(Qt::Horizontal);
    exploSplitter->addWidget(explosionListPanel);
    exploSplitter->addWidget(m_explosionEditor);
    exploSplitter->setSizes({280, 720});

    explosionsLayout->addWidget(exploSplitter);
    m_tabWidget->addTab(m_explosionsTab, "Explosions");

    // ========== COLORS TAB ==========
    m_colorsEditor = new ColorsEditorWidget();
    connect(m_colorsEditor, &ColorsEditorWidget::colorsModified, this, &EffectsEditorWindow::onDataModified);
    m_tabWidget->addTab(m_colorsEditor, "Scene Colors");
}

void EffectsEditorWindow::setupMenus() {
    QMenu* fileMenu = menuBar()->addMenu(tr("&File"));

    QAction* openDirAction = fileMenu->addAction(tr("Open &Directory..."));
    openDirAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_O));
    connect(openDirAction, &QAction::triggered, this, &EffectsEditorWindow::onOpenDirectory);

    QAction* openFilesAction = fileMenu->addAction(tr("Open &Files..."));
    openFilesAction->setShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_O));
    connect(openFilesAction, &QAction::triggered, this, &EffectsEditorWindow::onOpenFiles);

    fileMenu->addSeparator();

    QAction* saveAction = fileMenu->addAction(tr("&Save"));
    saveAction->setShortcut(QKeySequence::Save);
    connect(saveAction, &QAction::triggered, this, &EffectsEditorWindow::onSave);

    QAction* saveAsAction = fileMenu->addAction(tr("Save &As..."));
    saveAsAction->setShortcut(QKeySequence::SaveAs);
    connect(saveAsAction, &QAction::triggered, this, &EffectsEditorWindow::onSaveAs);

    fileMenu->addSeparator();

    QAction* closeAction = fileMenu->addAction(tr("&Close"));
    closeAction->setShortcut(QKeySequence::Close);
    connect(closeAction, &QAction::triggered, this, &QWidget::close);
}

void EffectsEditorWindow::setupToolbar() {
    m_toolBar = addToolBar(tr("Effects Toolbar"));
    m_toolBar->setMovable(false);

    QAction* openAction = m_toolBar->addAction("Open Dir");
    connect(openAction, &QAction::triggered, this, &EffectsEditorWindow::onOpenDirectory);

    QAction* saveAction = m_toolBar->addAction("Save");
    connect(saveAction, &QAction::triggered, this, &EffectsEditorWindow::onSave);
}

void EffectsEditorWindow::setupStatusBar() {
    m_statusLabel = new QLabel("Ready - Open a directory with Effects cfg files", this);
    statusBar()->addWidget(m_statusLabel);
}

void EffectsEditorWindow::updateTitle() {
    QString title = "Effects Editor";
    if (!m_currentDir.isEmpty()) {
        title += " - " + m_currentDir;
    }
    if (m_isModified) {
        title = "*" + title;
    }
    setWindowTitle(title);
}

void EffectsEditorWindow::clearProject() {
    m_project.clear();
    m_emitterList->clear();
    m_explosionList->clear();
    m_emitterEditor->setEmitter(nullptr);
    m_explosionEditor->setExplosion(nullptr);
    m_colorsEditor->setSceneColors(nullptr);
    m_isModified = false;
    m_currentDir.clear();
}

void EffectsEditorWindow::updateEmitterList() {
    m_emitterList->clear();

    QList<int> ids = m_project.emitters.keys();
    std::sort(ids.begin(), ids.end());

    for (int id : ids) {
        const Effects::Emitter& e = m_project.emitters[id];
        QString text = QString("%1: %2").arg(id).arg(e.name);
        QListWidgetItem* item = new QListWidgetItem(text);
        item->setData(Qt::UserRole, id);
        m_emitterList->addItem(item);
    }

    m_statusLabel->setText(QString("Loaded %1 emitters, %2 explosions")
                               .arg(m_project.emitters.size())
                               .arg(m_project.explosions.size()));
}

void EffectsEditorWindow::updateExplosionList() {
    m_explosionList->clear();

    QStringList names = m_project.explosions.keys();
    names.sort();

    for (const QString& name : names) {
        QListWidgetItem* item = new QListWidgetItem(name);
        item->setData(Qt::UserRole, name);
        m_explosionList->addItem(item);
    }
}

void EffectsEditorWindow::updateMaterialsList() {
    QStringList materials;
    materials << "Effect_smoke_puff2" << "Effect_Stars_HD_8st"
              << "effect_Gobin_Blood_gibba" << "Pang";

    for (auto it = m_project.emitterMaterials.begin(); it != m_project.emitterMaterials.end(); ++it) {
        if (!materials.contains(it.value())) {
            materials << it.value();
        }
    }

    materials.removeDuplicates();
    materials.sort();
    m_emitterEditor->setMaterialsList(materials);
}

void EffectsEditorWindow::openDirectory(const QString& dirPath) {
    if (!maybeSave()) return;

    clearProject();

    if (!m_parser.parseDirectory(dirPath, m_project)) {
        QMessageBox::warning(this, "Error", QString("Failed to parse:\n%1").arg(m_parser.lastError()));
        return;
    }

    m_currentDir = dirPath;
    updateEmitterList();
    updateExplosionList();
    updateMaterialsList();
    m_colorsEditor->setSceneColors(&m_project.sceneColors);
    updateTitle();
}

void EffectsEditorWindow::openFiles(const QStringList& files) {
    if (!maybeSave()) return;

    clearProject();

    for (const QString& file : files) {
        QString name = QFileInfo(file).fileName().toLower();

        if (name == "emitters.cfg") m_parser.parseEmittersCfg(file, m_project);
        else if (name == "emittermaterials.cfg") m_parser.parseEmitterMaterialsCfg(file, m_project);
        else if (name == "explosions.cfg") m_parser.parseExplosionsCfg(file, m_project);
        else if (name == "explosionssettings.cfg") m_parser.parseExplosionsSettingsCfg(file, m_project);
        else if (name == "colors.cfg") m_parser.parseColorsCfg(file, m_project);
    }

    if (!files.isEmpty()) m_currentDir = QFileInfo(files[0]).absolutePath();

    updateEmitterList();
    updateExplosionList();
    updateMaterialsList();
    m_colorsEditor->setSceneColors(&m_project.sceneColors);
    updateTitle();
}

void EffectsEditorWindow::onOpenDirectory() {
    QString dir = QFileDialog::getExistingDirectory(this, "Open Effects Directory",
                                                    m_currentDir.isEmpty() ? QDir::homePath() : m_currentDir);
    if (!dir.isEmpty()) {
        openDirectory(dir);
    }
}

void EffectsEditorWindow::onOpenFiles() {
    QStringList files = QFileDialog::getOpenFileNames(this, "Open Effect Files",
                                                      m_currentDir.isEmpty() ? QDir::homePath() : m_currentDir,
                                                      "Config files (*.cfg);;All files (*)");
    if (!files.isEmpty()) {
        openFiles(files);
    }
}

void EffectsEditorWindow::onSave() {
    if (m_project.emitters.isEmpty() && m_project.explosions.isEmpty()) {
        QMessageBox::warning(this, "Warning", "Nothing to save!");
        return;
    }

    if (m_currentDir.isEmpty()) {
        onSaveAs();
        return;
    }

    if (!m_writer.writeAll(m_project)) {
        QMessageBox::warning(this, "Error", QString("Failed to save:\n%1").arg(m_writer.lastError()));
        return;
    }

    m_isModified = false;
    updateTitle();
    m_statusLabel->setText("Saved successfully!");
}

void EffectsEditorWindow::onSaveAs() {
    QString dir = QFileDialog::getExistingDirectory(this, "Save Effects To Directory",
                                                    m_currentDir.isEmpty() ? QDir::homePath() : m_currentDir);
    if (dir.isEmpty()) return;

    m_project.emittersCfgPath = dir + "/Emitters.cfg";
    m_project.emitterMaterialsCfgPath = dir + "/EmitterMaterials.cfg";
    m_project.explosionsCfgPath = dir + "/Explosions.cfg";
    m_project.explosionsSettingsCfgPath = dir + "/ExplosionsSettings.cfg";
    m_project.colorsCfgPath = dir + "/Colors.cfg";

    m_currentDir = dir;
    onSave();
}

void EffectsEditorWindow::onEmitterSelected(QListWidgetItem* item) {
    if (!item) return;
    int id = item->data(Qt::UserRole).toInt();
    if (m_project.emitters.contains(id)) {
        m_emitterEditor->setEmitter(&m_project.emitters[id]);
    }
}

void EffectsEditorWindow::onExplosionSelected(QListWidgetItem* item) {
    if (!item) return;
    QString name = item->data(Qt::UserRole).toString();
    if (m_project.explosions.contains(name)) {
        m_explosionEditor->setExplosion(&m_project.explosions[name]);
    }
}

void EffectsEditorWindow::onAddEmitter() {
    int newId = 1;
    while (m_project.emitters.contains(newId)) newId++;

    Effects::Emitter newEmitter;
    newEmitter.id = newId;
    newEmitter.name = QString("NewEmitter%1").arg(newId);
    newEmitter.material = "Effect_smoke_puff2";
    newEmitter.gradientPoints.append(Effects::GradientPoint(0, 1.0f, 1.0f, 1.0f, 1.0f));
    newEmitter.gradientPoints.append(Effects::GradientPoint(1000, 0.0f, 0.0f, 0.0f, 0.0f));

    m_project.emitters[newId] = newEmitter;
    updateEmitterList();
    onDataModified();

    for (int i = 0; i < m_emitterList->count(); i++) {
        if (m_emitterList->item(i)->data(Qt::UserRole).toInt() == newId) {
            m_emitterList->setCurrentRow(i);
            onEmitterSelected(m_emitterList->item(i));
            break;
        }
    }
}

void EffectsEditorWindow::onRemoveEmitter() {
    QListWidgetItem* item = m_emitterList->currentItem();
    if (!item) return;

    int id = item->data(Qt::UserRole).toInt();
    QString name = m_project.emitters.contains(id) ? m_project.emitters[id].name : QString::number(id);

    if (QMessageBox::question(this, "Confirm Delete", QString("Delete emitter '%1'?").arg(name),
                              QMessageBox::Yes | QMessageBox::No) != QMessageBox::Yes) return;

    m_project.emitters.remove(id);
    m_project.emitterMaterials.remove(id);
    m_emitterEditor->setEmitter(nullptr);
    updateEmitterList();
    onDataModified();
}

void EffectsEditorWindow::onAddExplosion() {
    QString baseName = "NewExplosion";
    QString name = baseName;
    int counter = 1;
    while (m_project.explosions.contains(name)) {
        name = QString("%1%2").arg(baseName).arg(counter++);
    }

    Effects::Explosion newExplo;
    newExplo.name = name;
    newExplo.debrisExplosion = "PolygonBlow";

    m_project.explosions[name] = newExplo;
    updateExplosionList();
    onDataModified();

    for (int i = 0; i < m_explosionList->count(); i++) {
        if (m_explosionList->item(i)->data(Qt::UserRole).toString() == name) {
            m_explosionList->setCurrentRow(i);
            onExplosionSelected(m_explosionList->item(i));
            break;
        }
    }
}

void EffectsEditorWindow::onRemoveExplosion() {
    QListWidgetItem* item = m_explosionList->currentItem();
    if (!item) return;

    QString name = item->data(Qt::UserRole).toString();

    if (QMessageBox::question(this, "Confirm Delete", QString("Delete explosion '%1'?").arg(name),
                              QMessageBox::Yes | QMessageBox::No) != QMessageBox::Yes) return;

    m_project.explosions.remove(name);
    m_explosionEditor->setExplosion(nullptr);
    updateExplosionList();
    onDataModified();
}

void EffectsEditorWindow::onDataModified() {
    m_isModified = true;
    updateTitle();

    QListWidgetItem* item = m_emitterList->currentItem();
    if (item && m_emitterEditor->currentEmitter()) {
        int id = item->data(Qt::UserRole).toInt();
        item->setText(QString("%1: %2").arg(id).arg(m_emitterEditor->currentEmitter()->name));
    }
}

bool EffectsEditorWindow::maybeSave() {
    if (!m_isModified) return true;

    QMessageBox::StandardButton ret = QMessageBox::warning(this, "Effects Editor",
                                                           "The effects have been modified.\nDo you want to save your changes?",
                                                           QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);

    if (ret == QMessageBox::Save) {
        onSave();
        return !m_isModified;
    } else if (ret == QMessageBox::Cancel) {
        return false;
    }
    return true;
}

void EffectsEditorWindow::closeEvent(QCloseEvent* event) {
    if (maybeSave()) {
        event->accept();
    } else {
        event->ignore();
    }
}
