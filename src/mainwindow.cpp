#include "mainwindow.h"
#include "filetree.h"
#include "editor.h"
#include "search.h"
#include "linkparser.h"
#include "settings.h"
#include "preferencesdialog.h"
#include "vaultmanager.h"
#include "vaultdialog.h"
#include <QMenuBar>
#include <QStatusBar>
#include <QFileDialog>
#include <QMessageBox>
#include <QApplication>
#include <QStandardPaths>
#include <QInputDialog>
#include <QDate>
#include <QDir>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setupMenuBar();
    setupUI();
    setupConnections();

    // Initialize vault system
    initializeVaultSystem();

    // Apply current theme and settings
    Settings::instance()->applyTheme();
    connect(Settings::instance(), &Settings::fontChanged, this, &MainWindow::onFontChanged);

    setWindowTitle("Formica - Note Taking");
    resize(1200, 800);
}

void MainWindow::setupMenuBar()
{
    auto *fileMenu = menuBar()->addMenu("&File");

    auto *selectVaultAction = fileMenu->addAction("Select &Vault...");
    selectVaultAction->setShortcut(QKeySequence("Ctrl+Shift+O"));
    connect(selectVaultAction, &QAction::triggered, this, &MainWindow::selectVault);

    auto *openWorkspaceAction = fileMenu->addAction("&Open Workspace...");
    connect(openWorkspaceAction, &QAction::triggered, this, &MainWindow::openWorkspace);

    fileMenu->addSeparator();

    auto *newFileAction = fileMenu->addAction("&New File");
    newFileAction->setShortcut(QKeySequence::New);
    connect(newFileAction, &QAction::triggered, this, &MainWindow::newFile);

    auto *newZettelAction = fileMenu->addAction("New &Zettel");
    newZettelAction->setShortcut(QKeySequence("Ctrl+Shift+N"));
    connect(newZettelAction, &QAction::triggered, this, &MainWindow::newZettel);

    auto *newDailyAction = fileMenu->addAction("New &Daily Note");
    newDailyAction->setShortcut(QKeySequence("Ctrl+D"));
    connect(newDailyAction, &QAction::triggered, this, &MainWindow::newDailyNote);

    auto *saveFileAction = fileMenu->addAction("&Save");
    saveFileAction->setShortcut(QKeySequence::Save);
    connect(saveFileAction, &QAction::triggered, this, &MainWindow::saveFile);

    fileMenu->addSeparator();

    auto *exitAction = fileMenu->addAction("E&xit");
    exitAction->setShortcut(QKeySequence::Quit);
    connect(exitAction, &QAction::triggered, this, &QWidget::close);

    auto *editMenu = menuBar()->addMenu("&Edit");
    auto *searchAction = editMenu->addAction("&Search in Files...");
    searchAction->setShortcut(QKeySequence::Find);
    connect(searchAction, &QAction::triggered, this, &MainWindow::openSearch);

    editMenu->addSeparator();
    auto *prefsAction = editMenu->addAction("&Preferences...");
    prefsAction->setShortcut(QKeySequence::Preferences);
    connect(prefsAction, &QAction::triggered, this, &MainWindow::openPreferences);
}

void MainWindow::setupUI()
{
    m_centralWidget = new QWidget;
    setCentralWidget(m_centralWidget);

    // Main horizontal splitter
    m_mainSplitter = new QSplitter(Qt::Horizontal);

    // Left panel with file tree and search
    m_leftPanel = new QWidget;
    auto *leftLayout = new QVBoxLayout(m_leftPanel);

    // Search box
    m_searchBox = new QLineEdit;
    m_searchBox->setPlaceholderText("Search files...");
    leftLayout->addWidget(m_searchBox);

    // File tree
    m_fileTree = new FileTree;
    leftLayout->addWidget(m_fileTree);

    // Editor
    m_editor = new Editor;

    // Add to splitter
    m_mainSplitter->addWidget(m_leftPanel);
    m_mainSplitter->addWidget(m_editor);

    // Set splitter proportions
    m_mainSplitter->setSizes({300, 900});

    // Main layout
    auto *mainLayout = new QHBoxLayout(m_centralWidget);
    mainLayout->addWidget(m_mainSplitter);

    // Status bar
    m_statusLabel = new QLabel("Ready");
    statusBar()->addWidget(m_statusLabel);
}

void MainWindow::setupConnections()
{
    connect(m_fileTree, &FileTree::fileSelected, this, &MainWindow::onFileSelected);
    connect(m_searchBox, &QLineEdit::textChanged, this, &MainWindow::onSearchTextChanged);
    connect(m_editor, &Editor::linkClicked, this, &MainWindow::onLinkClicked);
}

void MainWindow::onFileSelected(const QString &filePath)
{
    if (m_editor->loadFile(filePath)) {
        m_statusLabel->setText("Loaded: " + filePath);
    } else {
        m_statusLabel->setText("Failed to load: " + filePath);
    }
}

void MainWindow::onSearchTextChanged(const QString &text)
{
    m_fileTree->filterFiles(text);
}

void MainWindow::onLinkClicked(const QString &linkTarget)
{
    if (m_currentWorkspace.isEmpty()) {
        return;
    }

    LinkParser linkParser;

    // Try to find the target file
    QString targetFile = linkParser.findNoteById(linkTarget, m_currentWorkspace);
    if (targetFile.isEmpty()) {
        targetFile = linkParser.findNoteByTitle(linkTarget, m_currentWorkspace);
    }

    if (!targetFile.isEmpty() && QFileInfo::exists(targetFile)) {
        // Open the target file
        if (m_editor->loadFile(targetFile)) {
            m_statusLabel->setText("Opened: " + linkTarget);
        }
    } else {
        // File doesn't exist, ask user if they want to create it
        QMessageBox::StandardButton reply = QMessageBox::question(this,
            "Create Note",
            QString("Note '%1' doesn't exist. Would you like to create it?").arg(linkTarget),
            QMessageBox::Yes | QMessageBox::No);

        if (reply == QMessageBox::Yes) {
            createNewNote(linkTarget);
        }
    }
}

void MainWindow::openWorkspace()
{
    QString dir = QFileDialog::getExistingDirectory(this, "Select Workspace Directory");
    if (!dir.isEmpty()) {
        m_currentWorkspace = dir;
        m_fileTree->setRootPath(dir);
        m_editor->setWorkspacePath(dir);
        setWindowTitle("Formica - " + dir);
        m_statusLabel->setText("Workspace: " + dir);
    }
}

void MainWindow::newFile()
{
    if (m_currentWorkspace.isEmpty()) {
        QMessageBox::information(this, "No Workspace", "Please open a workspace first.");
        return;
    }

    m_editor->newFile();
    m_statusLabel->setText("New file created");
}

void MainWindow::newZettel()
{
    if (m_currentWorkspace.isEmpty()) {
        QMessageBox::information(this, "No Workspace", "Please open a workspace first.");
        return;
    }

    LinkParser linkParser;

    // Get the next Zettel ID
    QString zettelId = linkParser.generateNextZettelId(QString(), m_currentWorkspace);

    // Ask for optional title
    bool ok;
    QString title = QInputDialog::getText(this, "New Zettel",
        QString("Creating Zettel %1\nOptional title:").arg(zettelId),
        QLineEdit::Normal, "", &ok);

    if (ok) {
        QString fileName;
        QString content;

        if (title.isEmpty()) {
            fileName = QString("%1.md").arg(zettelId);
            content = QString("%1\n\n").arg(zettelId);
        } else {
            fileName = QString("%1 %2.md").arg(zettelId, title.replace(" ", "_"));
            content = QString("%1 %2\n\n").arg(zettelId, title);
        }

        QString filePath = QDir(m_currentWorkspace).filePath(fileName);

        // Create and save the file
        QFile file(filePath);
        if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream out(&file);
            out << content;
            file.close();

            // Load the new file in editor
            if (m_editor->loadFile(filePath)) {
                m_fileTree->refresh();
                m_statusLabel->setText("Created Zettel: " + zettelId);
            }
        } else {
            QMessageBox::warning(this, "Error", "Could not create Zettel: " + fileName);
        }
    }
}

void MainWindow::newDailyNote()
{
    if (m_currentWorkspace.isEmpty()) {
        QMessageBox::information(this, "No Workspace", "Please open a workspace first.");
        return;
    }

    // Generate today's date
    QString today = QDate::currentDate().toString("yyyy-MM-dd");
    QString fileName = QString("%1.md").arg(today);
    QString filePath = QDir(m_currentWorkspace).filePath(fileName);

    // Check if daily note already exists
    if (QFileInfo::exists(filePath)) {
        // Open existing daily note
        if (m_editor->loadFile(filePath)) {
            m_statusLabel->setText("Opened daily note: " + today);
        }
    } else {
        // Create new daily note
        QString content = QString("# Daily Note - %1\n\n## Today\n\n## Tomorrow\n\n## Notes\n\n").arg(today);

        QFile file(filePath);
        if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream out(&file);
            out << content;
            file.close();

            // Load the new file in editor
            if (m_editor->loadFile(filePath)) {
                m_fileTree->refresh();
                m_statusLabel->setText("Created daily note: " + today);
            }
        } else {
            QMessageBox::warning(this, "Error", "Could not create daily note: " + fileName);
        }
    }
}

void MainWindow::saveFile()
{
    if (m_editor->saveFile()) {
        m_statusLabel->setText("File saved");
        m_fileTree->refresh();
    } else {
        m_statusLabel->setText("Failed to save file");
    }
}

void MainWindow::openSearch()
{
    if (m_currentWorkspace.isEmpty()) {
        QMessageBox::information(this, "No Workspace", "Please open a workspace first.");
        return;
    }

    Search searchDialog(m_currentWorkspace, this);
    connect(&searchDialog, &Search::fileSelected,
            [this](const QString &filePath, int line) {
                if (m_editor->loadFile(filePath)) {
                    m_statusLabel->setText("Loaded: " + filePath);
                    // TODO: Jump to specific line if needed
                }
            });

    searchDialog.exec();
}

void MainWindow::createNewNote(const QString &title)
{
    LinkParser linkParser;

    // Check if this looks like a zettel ID
    ZettelId zettelId = linkParser.parseZettelId(title);

    QString fileName;
    QString content;

    if (zettelId.isValid && linkParser.isValidZettelId(zettelId.id)) {
        // Create zettelkasten note
        if (zettelId.title.isEmpty()) {
            fileName = QString("%1.md").arg(zettelId.id);
            content = QString("%1\n\n").arg(zettelId.id);
        } else {
            fileName = QString("%1 %2.md").arg(zettelId.id, zettelId.title);
            content = QString("%1 %2\n\n").arg(zettelId.id, zettelId.title);
        }
    } else {
        // Regular note
        fileName = QString("%1.md").arg(QString(title).replace(" ", "_"));
        content = QString("# %1\n\n").arg(title);
    }

    QString filePath = QDir(m_currentWorkspace).filePath(fileName);

    // Create and save the file
    QFile file(filePath);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&file);
        out << content;
        file.close();

        // Load the new file in editor
        if (m_editor->loadFile(filePath)) {
            m_fileTree->refresh();
            m_statusLabel->setText("Created: " + fileName);
        }
    } else {
        QMessageBox::warning(this, "Error", "Could not create file: " + fileName);
    }
}

void MainWindow::openPreferences()
{
    PreferencesDialog dialog(this);
    dialog.exec();
}

void MainWindow::onFontChanged(const QFont &font)
{
    // Update editor font - the editor will connect to this signal separately
}

void MainWindow::initializeVaultSystem()
{
    VaultManager *vaultManager = VaultManager::instance();

    // Check if there's a current vault
    if (vaultManager->hasCurrentVault()) {
        setCurrentVault(vaultManager->currentVaultPath());
    } else {
        // Create a default vault if none exists
        auto vaults = vaultManager->getAllVaults();
        if (vaults.isEmpty()) {
            // Create default vault
            QString defaultPath = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) + "/Formica Vaults/My First Vault";
            if (vaultManager->createVault("My First Vault", defaultPath, "Your first Formica vault")) {
                vaultManager->openVault("My First Vault");
                setCurrentVault(defaultPath);
            }
        } else {
            // Show vault selection dialog
            selectVault();
        }
    }

    // Connect to vault changes
    connect(vaultManager, &VaultManager::vaultChanged, [this](const Vault &vault) {
        setCurrentVault(vault.path);
    });
}

void MainWindow::setCurrentVault(const QString &vaultPath)
{
    m_currentWorkspace = vaultPath;
    m_fileTree->setRootPath(vaultPath);
    m_editor->setWorkspacePath(vaultPath);

    // Update window title
    VaultManager *vaultManager = VaultManager::instance();
    QString vaultName = vaultManager->getCurrentVault().name;
    setWindowTitle(QString("Formica - %1").arg(vaultName));
    m_statusLabel->setText(QString("Vault: %1").arg(vaultName));
}

void MainWindow::selectVault()
{
    VaultDialog dialog(this);
    if (dialog.exec() == QDialog::Accepted) {
        QString selectedVault = dialog.selectedVaultName();
        if (!selectedVault.isEmpty()) {
            VaultManager::instance()->openVault(selectedVault);
        }
    }
}