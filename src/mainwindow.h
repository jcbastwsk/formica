#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSplitter>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QLabel>

class FileTree;
class Editor;
class Search;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow() = default;

private slots:
    void onFileSelected(const QString &filePath);
    void onSearchTextChanged(const QString &text);
    void onLinkClicked(const QString &linkTarget);
    void selectVault();
    void openWorkspace();
    void newFile();
    void newZettel();
    void newDailyNote();
    void saveFile();
    void openSearch();
    void openPreferences();
    void onFontChanged(const QFont &font);

private:
    void setupMenuBar();
    void setupUI();
    void setupConnections();
    void initializeVaultSystem();
    void setCurrentVault(const QString &vaultPath);
    void createNewNote(const QString &title);

    QWidget *m_centralWidget;
    QSplitter *m_mainSplitter;
    QWidget *m_leftPanel;

    FileTree *m_fileTree;
    Editor *m_editor;
    QLineEdit *m_searchBox;
    QLabel *m_statusLabel;

    QString m_currentWorkspace;
};

#endif // MAINWINDOW_H