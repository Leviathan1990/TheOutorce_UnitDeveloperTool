#include "AIEditorWindow.h"
#include "InitParamsWidget.h"
#include "BuildFitnessWidget.h"
#include "MaxUnitsWidget.h"

#include <QMenuBar>
#include <QStatusBar>
#include <QFileDialog>
#include <QMessageBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QDir>
#include <QCloseEvent>
#include <QApplication>

AIEditorWindow::AIEditorWindow(QWidget* parent)
    : QMainWindow(parent), m_isModified(false)
{
    setWindowTitle("AI Editor");
    resize(950, 700);

    setupUI();
    setupMenus();
    setupToolbar();
    setupStatusBar();
    updateTitle();
}

AIEditorWindow::~AIEditorWindow() {
    clearProject();
}

void AIEditorWindow::setupUI() {
    QWidget* centralWidget = new QWidget(this);
    QVBoxLayout* mainLayout = new QVBoxLayout(centralWidget);

    // Selector panel
    QGroupBox* selectorGroup = new QGroupBox("Select AI Configuration", this);
    QHBoxLayout* selectorLayout = new QHBoxLayout(selectorGroup);

    selectorLayout->addWidget(new QLabel("Race:", this));
    m_raceCombo = new QComboBox(this);
    m_raceCombo->setMinimumWidth(150);
    connect(m_raceCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &AIEditorWindow::onRaceChanged);
    selectorLayout->addWidget(m_raceCombo);

    selectorLayout->addSpacing(20);

    selectorLayout->addWidget(new QLabel("Difficulty:", this));
    m_difficultyCombo = new QComboBox(this);
    m_difficultyCombo->addItem("0 - Easy", 0);
    m_difficultyCombo->addItem("1 - Medium", 1);
    m_difficultyCombo->addItem("2 - Hard", 2);
    connect(m_difficultyCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &AIEditorWindow::onDifficultyChanged);
    selectorLayout->addWidget(m_difficultyCombo);

    selectorLayout->addStretch();

    mainLayout->addWidget(selectorGroup);

    // Tab widget
    m_tabWidget = new QTabWidget(this);

    // Init Parameters tab
    m_initParamsWidget = new AI::InitParamsWidget(this);
    connect(m_initParamsWidget, &AI::InitParamsWidget::paramsModified,
            this, &AIEditorWindow::onDataModified);
    m_tabWidget->addTab(m_initParamsWidget, "Init Parameters");

    // Build Fitness tab
    m_buildFitnessWidget = new AI::BuildFitnessWidget(this);
    connect(m_buildFitnessWidget, &AI::BuildFitnessWidget::fitnessModified,
            this, &AIEditorWindow::onDataModified);
    m_tabWidget->addTab(m_buildFitnessWidget, "Build Fitness");

    // Max Units tab
    m_maxUnitsWidget = new AI::MaxUnitsWidget(this);
    connect(m_maxUnitsWidget, &AI::MaxUnitsWidget::maxUnitsModified,
            this, &AIEditorWindow::onDataModified);
    m_tabWidget->addTab(m_maxUnitsWidget, "Max Units");

    mainLayout->addWidget(m_tabWidget, 1);

    setCentralWidget(centralWidget);
}

void AIEditorWindow::setupMenus() {
    QMenu* fileMenu = menuBar()->addMenu(tr("&File"));

    QAction* openDirAction = fileMenu->addAction(tr("Open AIVALUES &Directory..."));
    openDirAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_O));
    connect(openDirAction, &QAction::triggered, this, &AIEditorWindow::onOpenDirectory);

    fileMenu->addSeparator();

    QAction* saveAction = fileMenu->addAction(tr("&Save Current Tab"));
    saveAction->setShortcut(QKeySequence::Save);
    connect(saveAction, &QAction::triggered, this, &AIEditorWindow::onSave);

    QAction* saveAllAction = fileMenu->addAction(tr("Save &All"));
    saveAllAction->setShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_S));
    connect(saveAllAction, &QAction::triggered, this, &AIEditorWindow::onSaveAll);

    fileMenu->addSeparator();

    QAction* closeAction = fileMenu->addAction(tr("&Close"));
    closeAction->setShortcut(QKeySequence::Close);
    connect(closeAction, &QAction::triggered, this, &QWidget::close);
}

void AIEditorWindow::setupToolbar() {
    m_toolBar = addToolBar(tr("AI Toolbar"));
    m_toolBar->setMovable(false);

    QAction* openAction = m_toolBar->addAction("Open Dir");
    openAction->setToolTip("Open AIVALUES directory");
    connect(openAction, &QAction::triggered, this, &AIEditorWindow::onOpenDirectory);

    QAction* saveAction = m_toolBar->addAction("Save");
    saveAction->setToolTip("Save current tab");
    connect(saveAction, &QAction::triggered, this, &AIEditorWindow::onSave);

    QAction* saveAllAction = m_toolBar->addAction("Save All");
    saveAllAction->setToolTip("Save all tabs");
    connect(saveAllAction, &QAction::triggered, this, &AIEditorWindow::onSaveAll);
}

void AIEditorWindow::setupStatusBar() {
    m_statusLabel = new QLabel("Ready - Open an AIVALUES directory", this);
    statusBar()->addWidget(m_statusLabel);
}

void AIEditorWindow::updateTitle() {
    QString title = "AI Editor";
    if (!m_project.baseDir.isEmpty()) {
        title += " - " + m_project.baseDir;
    }
    if (m_isModified) {
        title = "*" + title;
    }
    setWindowTitle(title);
}

void AIEditorWindow::clearProject() {
    m_project.clear();
    m_raceCombo->clear();
    m_initParamsWidget->clear();
    m_buildFitnessWidget->clear();
    m_maxUnitsWidget->clear();
    m_isModified = false;
}

void AIEditorWindow::openDirectory(const QString& dirPath) {
    if (!maybeSave()) return;

    clearProject();

    if (!m_parser.scanDirectory(dirPath, m_project)) {
        QMessageBox::warning(this, "Error",
                             QString("Failed to scan directory:\n%1").arg(m_parser.lastError()));
        return;
    }

    // Populate race combo
    m_raceCombo->blockSignals(true);
    for (const QString& race : m_project.availableRaces) {
        m_raceCombo->addItem(race);
    }
    m_raceCombo->blockSignals(false);

    updateTitle();

    m_statusLabel->setText(QString("Loaded: %1 races found").arg(m_project.availableRaces.size()));

    // Load first race if available
    if (!m_project.availableRaces.isEmpty()) {
        m_raceCombo->setCurrentIndex(0);
        loadCurrentFiles();
    }
}

void AIEditorWindow::setAvailableUnits(const QStringList& units) {
    m_availableUnits = units;
    m_buildFitnessWidget->setAvailableUnits(units);
    m_maxUnitsWidget->setAvailableUnits(units);
}

void AIEditorWindow::loadCurrentFiles() {
    if (m_project.baseDir.isEmpty() || m_raceCombo->currentText().isEmpty()) {
        return;
    }

    QString race = m_raceCombo->currentText();
    int difficulty = m_difficultyCombo->currentData().toInt();

    // Load Init Parameters
    QString initPath = AI::AIParser::buildFilePath(m_project.baseDir, race, "InitParameters", difficulty);
    if (QFile::exists(initPath)) {
        m_parser.parseInitParameters(initPath, m_project.initParams);
        m_initParamsWidget->setInitParameters(&m_project.initParams);
    } else {
        m_project.initParams.clear();
        m_project.initParams.filePath = initPath;
        m_project.initParams.raceName = race;
        m_project.initParams.difficulty = difficulty;
        m_initParamsWidget->setInitParameters(&m_project.initParams);
    }

    // Load Build Fitness
    QString fitnessPath = AI::AIParser::buildFilePath(m_project.baseDir, race, "BuildFitness", difficulty);
    if (QFile::exists(fitnessPath)) {
        m_parser.parseBuildFitness(fitnessPath, m_project.buildFitness);
        m_buildFitnessWidget->setBuildFitness(&m_project.buildFitness);
    } else {
        m_project.buildFitness.clear();
        m_project.buildFitness.filePath = fitnessPath;
        m_project.buildFitness.raceName = race;
        m_project.buildFitness.difficulty = difficulty;
        m_buildFitnessWidget->setBuildFitness(&m_project.buildFitness);
    }

    // Load Max Units
    QString maxPath = AI::AIParser::buildFilePath(m_project.baseDir, race, "MaxNumberOfUnits", difficulty);
    if (QFile::exists(maxPath)) {
        m_parser.parseMaxUnits(maxPath, m_project.maxUnits);
        m_maxUnitsWidget->setMaxUnits(&m_project.maxUnits);
    } else {
        m_project.maxUnits.clear();
        m_project.maxUnits.filePath = maxPath;
        m_project.maxUnits.raceName = race;
        m_project.maxUnits.difficulty = difficulty;
        m_maxUnitsWidget->setMaxUnits(&m_project.maxUnits);
    }

    m_statusLabel->setText(QString("Loaded: %1 / %2")
                               .arg(race)
                               .arg(AI::difficultyToString(difficulty)));
}

void AIEditorWindow::onOpenDirectory() {
    QString dir = QFileDialog::getExistingDirectory(
        this, "Open AIVALUES Directory",
        m_project.baseDir.isEmpty() ? QDir::homePath() : m_project.baseDir);

    if (!dir.isEmpty()) {
        openDirectory(dir);
    }
}

void AIEditorWindow::onSave() {
    int currentTab = m_tabWidget->currentIndex();
    bool success = false;

    if (currentTab == 0 && !m_project.initParams.filePath.isEmpty()) {
        success = m_writer.writeInitParameters(m_project.initParams.filePath, m_project.initParams);
        if (success) m_statusLabel->setText("Saved: InitParameters");
    } else if (currentTab == 1 && !m_project.buildFitness.filePath.isEmpty()) {
        success = m_writer.writeBuildFitness(m_project.buildFitness.filePath, m_project.buildFitness);
        if (success) m_statusLabel->setText("Saved: BuildFitness");
    } else if (currentTab == 2 && !m_project.maxUnits.filePath.isEmpty()) {
        success = m_writer.writeMaxUnits(m_project.maxUnits.filePath, m_project.maxUnits);
        if (success) m_statusLabel->setText("Saved: MaxNumberOfUnits");
    }

    if (!success) {
        QMessageBox::warning(this, "Error", QString("Failed to save:\n%1").arg(m_writer.lastError()));
    } else {
        m_isModified = false;
        updateTitle();
    }
}

void AIEditorWindow::onSaveAll() {
    bool allSuccess = true;
    QStringList saved;

    if (!m_project.initParams.filePath.isEmpty()) {
        if (m_writer.writeInitParameters(m_project.initParams.filePath, m_project.initParams)) {
            saved << "InitParameters";
        } else {
            allSuccess = false;
        }
    }

    if (!m_project.buildFitness.filePath.isEmpty()) {
        if (m_writer.writeBuildFitness(m_project.buildFitness.filePath, m_project.buildFitness)) {
            saved << "BuildFitness";
        } else {
            allSuccess = false;
        }
    }

    if (!m_project.maxUnits.filePath.isEmpty()) {
        if (m_writer.writeMaxUnits(m_project.maxUnits.filePath, m_project.maxUnits)) {
            saved << "MaxUnits";
        } else {
            allSuccess = false;
        }
    }

    if (allSuccess) {
        m_isModified = false;
        updateTitle();
        m_statusLabel->setText(QString("Saved all: %1").arg(saved.join(", ")));
    } else {
        QMessageBox::warning(this, "Error", QString("Some files failed to save:\n%1").arg(m_writer.lastError()));
    }
}

void AIEditorWindow::onRaceChanged(int index) {
    Q_UNUSED(index);
    if (!m_project.baseDir.isEmpty()) {
        loadCurrentFiles();
    }
}

void AIEditorWindow::onDifficultyChanged(int index) {
    Q_UNUSED(index);
    if (!m_project.baseDir.isEmpty()) {
        loadCurrentFiles();
    }
}

void AIEditorWindow::onDataModified() {
    m_isModified = true;
    updateTitle();
}

bool AIEditorWindow::maybeSave() {
    if (!m_isModified) return true;

    QMessageBox::StandardButton ret = QMessageBox::warning(
        this, "AI Editor",
        "The AI configuration has been modified.\nDo you want to save your changes?",
        QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);

    if (ret == QMessageBox::Save) {
        onSaveAll();
        return !m_isModified;
    } else if (ret == QMessageBox::Cancel) {
        return false;
    }
    return true;
}

void AIEditorWindow::closeEvent(QCloseEvent* event) {
    if (maybeSave()) {
        event->accept();
    } else {
        event->ignore();
    }
}
