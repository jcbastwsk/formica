#include "filetree.h"
#include <QDir>
#include <QFileInfo>
#include <QContextMenuEvent>
#include <QDesktopServices>
#include <QUrl>
#include <QProcess>
#include <QStandardPaths>
#include <QMessageBox>
#include <QInputDialog>

FileTree::FileTree(QWidget *parent)
    : QWidget(parent)
{
    setupUI();
    setupContextMenu();
}

void FileTree::setupUI()
{
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    m_treeView = new QTreeView;
    m_model = new QFileSystemModel;

    // Set up model to show only directories and markdown files
    m_model->setRootPath(QDir::rootPath());
    QStringList filters;
    filters << "*.md" << "*.markdown" << "*.txt";
    m_model->setNameFilters(filters);
    m_model->setNameFilterDisables(false);

    m_treeView->setModel(m_model);

    // Hide columns we don't need
    m_treeView->setHeaderHidden(true);
    m_treeView->hideColumn(1); // Size
    m_treeView->hideColumn(2); // Type
    m_treeView->hideColumn(3); // Date Modified

    // Enable selection
    m_treeView->setSelectionMode(QAbstractItemView::SingleSelection);

    layout->addWidget(m_treeView);

    // Connect signals
    connect(m_treeView, &QTreeView::clicked, this, &FileTree::onItemClicked);

    // Enable context menu
    m_treeView->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(m_treeView, &QTreeView::customContextMenuRequested, this, &FileTree::showContextMenu);
}

void FileTree::setRootPath(const QString &path)
{
    m_rootPath = path;

    // Create directory if it doesn't exist
    QDir dir(path);
    if (!dir.exists()) {
        dir.mkpath(".");
    }

    QModelIndex rootIndex = m_model->setRootPath(path);
    m_treeView->setRootIndex(rootIndex);
    m_treeView->expandAll();
}

void FileTree::filterFiles(const QString &filter)
{
    m_currentFilter = filter;
    // For now, we'll implement a simple filter
    // In the future, we can add more sophisticated filtering
    if (filter.isEmpty()) {
        QStringList filters;
        filters << "*.md" << "*.markdown" << "*.txt";
        m_model->setNameFilters(filters);
    } else {
        QStringList filters;
        filters << QString("*%1*.md").arg(filter)
                << QString("*%1*.markdown").arg(filter)
                << QString("*%1*.txt").arg(filter);
        m_model->setNameFilters(filters);
    }
}

void FileTree::refresh()
{
    if (!m_rootPath.isEmpty()) {
        m_model->setRootPath(m_rootPath);
    }
}

void FileTree::onItemClicked(const QModelIndex &index)
{
    if (m_model->isDir(index)) {
        // Expand/collapse directory
        if (m_treeView->isExpanded(index)) {
            m_treeView->collapse(index);
        } else {
            m_treeView->expand(index);
        }
        return;
    }

    QString filePath = m_model->filePath(index);
    if (isMarkdownFile(filePath)) {
        emit fileSelected(filePath);
    }
}

bool FileTree::isMarkdownFile(const QString &filePath) const
{
    QFileInfo fileInfo(filePath);
    QString suffix = fileInfo.suffix().toLower();
    return suffix == "md" || suffix == "markdown" || suffix == "txt";
}

void FileTree::setupContextMenu()
{
    m_contextMenu = new QMenu(this);

    auto *showInExplorerAction = m_contextMenu->addAction("Show in File Explorer");
    connect(showInExplorerAction, &QAction::triggered, this, &FileTree::showInFileExplorer);

    m_contextMenu->addSeparator();

    auto *renameAction = m_contextMenu->addAction("Rename");
    connect(renameAction, &QAction::triggered, this, &FileTree::renameFile);

    auto *deleteAction = m_contextMenu->addAction("Delete");
    connect(deleteAction, &QAction::triggered, this, &FileTree::deleteFile);
}

void FileTree::showContextMenu(const QPoint &pos)
{
    QModelIndex index = m_treeView->indexAt(pos);
    if (index.isValid()) {
        m_contextMenuIndex = index;
        m_contextMenu->exec(m_treeView->mapToGlobal(pos));
    }
}

void FileTree::showInFileExplorer()
{
    if (!m_contextMenuIndex.isValid()) {
        return;
    }

    QString filePath = m_model->filePath(m_contextMenuIndex);
    QFileInfo fileInfo(filePath);

    // Open file manager and select the file
#ifdef Q_OS_WIN
    // Windows
    QProcess::startDetached("explorer", {"/select,", QDir::toNativeSeparators(filePath)});
#elif defined(Q_OS_MAC)
    // macOS
    QProcess::startDetached("open", {"-R", filePath});
#else
    // Linux/FreeBSD/Unix
    QString fileManager = findFileManager();
    if (!fileManager.isEmpty()) {
        if (fileManager.contains("dolphin")) {
            QProcess::startDetached(fileManager, {"--select", filePath});
        } else if (fileManager.contains("nautilus")) {
            QProcess::startDetached(fileManager, {"--select", filePath});
        } else if (fileManager.contains("thunar")) {
            QProcess::startDetached(fileManager, {filePath});
        } else {
            // Fallback: open parent directory
            QString parentDir = fileInfo.dir().path();
            QDesktopServices::openUrl(QUrl::fromLocalFile(parentDir));
        }
    } else {
        // Fallback: open parent directory with default handler
        QString parentDir = fileInfo.dir().path();
        QDesktopServices::openUrl(QUrl::fromLocalFile(parentDir));
    }
#endif
}

void FileTree::deleteFile()
{
    if (!m_contextMenuIndex.isValid()) {
        return;
    }

    QString filePath = m_model->filePath(m_contextMenuIndex);
    QFileInfo fileInfo(filePath);

    QMessageBox::StandardButton reply = QMessageBox::question(this,
        "Delete File",
        QString("Are you sure you want to delete '%1'?").arg(fileInfo.fileName()),
        QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        if (QFile::remove(filePath)) {
            refresh();
        } else {
            QMessageBox::warning(this, "Error", "Could not delete the file.");
        }
    }
}

void FileTree::renameFile()
{
    if (!m_contextMenuIndex.isValid()) {
        return;
    }

    QString filePath = m_model->filePath(m_contextMenuIndex);
    QFileInfo fileInfo(filePath);
    QString currentName = fileInfo.completeBaseName();

    bool ok;
    QString newName = QInputDialog::getText(this, "Rename File",
        "New name:", QLineEdit::Normal, currentName, &ok);

    if (ok && !newName.isEmpty() && newName != currentName) {
        QString newFilePath = fileInfo.dir().filePath(newName + "." + fileInfo.suffix());

        if (QFile::rename(filePath, newFilePath)) {
            refresh();
        } else {
            QMessageBox::warning(this, "Error", "Could not rename the file.");
        }
    }
}

QString FileTree::findFileManager()
{
    QStringList candidates = {
        "dolphin",      // KDE
        "nautilus",     // GNOME
        "thunar",       // XFCE
        "pcmanfm",      // LXDE
        "nemo",         // Cinnamon
        "caja"          // MATE
    };

    for (const QString &manager : candidates) {
        QString path = QStandardPaths::findExecutable(manager);
        if (!path.isEmpty()) {
            return path;
        }
    }

    return QString();
}