#include "search.h"
#include <QDir>
#include <QDirIterator>
#include <QFile>
#include <QTextStream>
#include <QFileInfo>
#include <QKeyEvent>
#include <QApplication>

Search::Search(const QString &workspacePath, QWidget *parent)
    : QDialog(parent), m_workspacePath(workspacePath)
{
    setupUI();

    m_searchTimer = new QTimer(this);
    m_searchTimer->setSingleShot(true);
    m_searchTimer->setInterval(300); // 300ms delay

    connect(m_searchTimer, &QTimer::timeout, this, &Search::onSearchTimerTimeout);
    connect(m_searchEdit, &QLineEdit::textChanged, this, &Search::onSearchTextChanged);
    connect(m_resultsList, &QListWidget::itemDoubleClicked, this, &Search::onResultDoubleClicked);
    connect(m_searchButton, &QPushButton::clicked, this, &Search::performSearch);

    setWindowTitle("Search in Files");
    resize(600, 400);
}

void Search::setupUI()
{
    auto *layout = new QVBoxLayout(this);

    // Search input
    auto *searchLayout = new QHBoxLayout;
    m_searchEdit = new QLineEdit;
    m_searchEdit->setPlaceholderText("Search for text in files...");
    m_searchButton = new QPushButton("Search");
    m_searchButton->setDefault(true);

    searchLayout->addWidget(m_searchEdit);
    searchLayout->addWidget(m_searchButton);

    // Results list
    m_resultsList = new QListWidget;
    m_resultsList->setAlternatingRowColors(true);

    // Status label
    m_statusLabel = new QLabel("Enter search text to begin");

    layout->addLayout(searchLayout);
    layout->addWidget(m_resultsList);
    layout->addWidget(m_statusLabel);

    m_searchEdit->setFocus();
}

void Search::setWorkspacePath(const QString &path)
{
    m_workspacePath = path;
}

void Search::onSearchTextChanged()
{
    m_searchTimer->stop();
    if (!m_searchEdit->text().isEmpty()) {
        m_searchTimer->start();
    } else {
        m_resultsList->clear();
        m_statusLabel->setText("Enter search text to begin");
    }
}

void Search::onSearchTimerTimeout()
{
    performSearch();
}

void Search::performSearch()
{
    QString searchText = m_searchEdit->text().trimmed();
    if (searchText.isEmpty()) {
        return;
    }

    m_resultsList->clear();
    m_statusLabel->setText("Searching...");

    QApplication::processEvents(); // Update UI

    auto results = FileSearcher::searchInFiles(m_workspacePath, searchText);

    m_resultsList->clear();
    for (const auto &result : results) {
        auto *item = new QListWidgetItem(result.displayText);
        item->setData(Qt::UserRole, result.filePath);
        item->setData(Qt::UserRole + 1, result.lineNumber);
        m_resultsList->addItem(item);
    }

    if (results.isEmpty()) {
        m_statusLabel->setText(QString("No results found for '%1'").arg(searchText));
    } else {
        m_statusLabel->setText(QString("Found %1 results").arg(results.size()));
    }
}

void Search::onResultDoubleClicked(QListWidgetItem *item)
{
    if (!item) return;

    QString filePath = item->data(Qt::UserRole).toString();
    int lineNumber = item->data(Qt::UserRole + 1).toInt();

    emit fileSelected(filePath, lineNumber);
    accept();
}

// FileSearcher implementation

QList<Search::SearchResult> FileSearcher::searchInFiles(const QString &directory, const QString &searchText)
{
    QList<Search::SearchResult> allResults;

    QDirIterator it(directory,
                    QStringList() << "*.md" << "*.markdown" << "*.txt",
                    QDir::Files,
                    QDirIterator::Subdirectories);

    while (it.hasNext()) {
        QString filePath = it.next();
        if (isTextFile(filePath)) {
            auto fileResults = searchInSingleFile(filePath, searchText);
            allResults.append(fileResults);
        }
    }

    return allResults;
}

bool FileSearcher::isTextFile(const QString &filePath)
{
    QFileInfo info(filePath);
    QString suffix = info.suffix().toLower();
    return suffix == "md" || suffix == "markdown" || suffix == "txt";
}

QList<Search::SearchResult> FileSearcher::searchInSingleFile(const QString &filePath, const QString &searchText)
{
    QList<Search::SearchResult> results;

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return results;
    }

    QTextStream in(&file);
    int lineNumber = 1;
    QString line;

    while (in.readLineInto(&line)) {
        if (line.contains(searchText, Qt::CaseInsensitive)) {
            Search::SearchResult result;
            result.filePath = filePath;
            result.lineNumber = lineNumber;
            result.lineText = line.trimmed();

            QFileInfo fileInfo(filePath);
            result.displayText = QString("%1:%2: %3")
                .arg(fileInfo.fileName())
                .arg(lineNumber)
                .arg(line.trimmed());

            results.append(result);
        }
        lineNumber++;
    }

    return results;
}

