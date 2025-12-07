#ifndef FILETREE_H
#define FILETREE_H

#include <QWidget>
#include <QTreeView>
#include <QFileSystemModel>
#include <QVBoxLayout>
#include <QMenu>
#include <QAction>

class FileTree : public QWidget
{
    Q_OBJECT

public:
    explicit FileTree(QWidget *parent = nullptr);

    void setRootPath(const QString &path);
    void filterFiles(const QString &filter);
    void refresh();

signals:
    void fileSelected(const QString &filePath);

private slots:
    void onItemClicked(const QModelIndex &index);
    void showContextMenu(const QPoint &pos);
    void showInFileExplorer();
    void deleteFile();
    void renameFile();

private:
    void setupUI();
    void setupContextMenu();
    bool isMarkdownFile(const QString &filePath) const;
    QString findFileManager();

    QTreeView *m_treeView;
    QFileSystemModel *m_model;
    QString m_currentFilter;
    QString m_rootPath;

    QMenu *m_contextMenu;
    QModelIndex m_contextMenuIndex;
};

#endif // FILETREE_H