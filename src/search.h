#ifndef SEARCH_H
#define SEARCH_H

#include <QWidget>
#include <QDialog>
#include <QLineEdit>
#include <QListWidget>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTimer>

class Search : public QDialog
{
    Q_OBJECT

public:
    explicit Search(const QString &workspacePath, QWidget *parent = nullptr);

    void setWorkspacePath(const QString &path);

signals:
    void fileSelected(const QString &filePath, int line = -1);

private slots:
    void onSearchTextChanged();
    void onSearchTimerTimeout();
    void onResultDoubleClicked(QListWidgetItem *item);
    void performSearch();

private:
    void setupUI();
    void searchInFile(const QString &filePath, const QString &searchText);
    void addSearchResult(const QString &filePath, int lineNumber, const QString &lineText);

    QLineEdit *m_searchEdit;
    QListWidget *m_resultsList;
    QLabel *m_statusLabel;
    QPushButton *m_searchButton;

    QTimer *m_searchTimer;
    QString m_workspacePath;

public:
    struct SearchResult {
        QString filePath;
        int lineNumber;
        QString lineText;
        QString displayText;
    };
};

// Helper class for file content search
class FileSearcher : public QObject
{
    Q_OBJECT

public:
    static QList<Search::SearchResult> searchInFiles(const QString &directory, const QString &searchText);

private:
    static bool isTextFile(const QString &filePath);
    static QList<Search::SearchResult> searchInSingleFile(const QString &filePath, const QString &searchText);
};

#endif // SEARCH_H