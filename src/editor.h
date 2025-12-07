#ifndef EDITOR_H
#define EDITOR_H

#include <QWidget>
#include <QTextEdit>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QSplitter>
#include <QSyntaxHighlighter>
#include <QTextCharFormat>
#include <QRegularExpression>
#include <QMenu>

class MarkdownHighlighter;
class LinkParser;

class Editor : public QWidget
{
    Q_OBJECT

public:
    explicit Editor(QWidget *parent = nullptr);

    bool loadFile(const QString &filePath);
    bool saveFile();
    void newFile();

    QString currentFilePath() const { return m_currentFilePath; }
    bool isModified() const;

    void setWorkspacePath(const QString &path) { m_workspacePath = path; }

signals:
    void linkClicked(const QString &linkTarget);

private slots:
    void onTextChanged();
    void togglePreview();
    void onLinkClicked();
    void showEditorContextMenu(const QPoint &pos);
    void showCurrentFileInExplorer();
    void onFontChanged(const QFont &font);

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;

private:
    void setupUI();
    void setupEditorContextMenu();
    void updatePreview();
    void setCurrentFile(const QString &filePath);
    QString getLinkAtPosition(const QPoint &pos);
    void openFileManagerAndSelect(const QString &filePath);
    void applyCurrentSettings();

    QSplitter *m_splitter;
    QTextEdit *m_textEdit;
    QTextEdit *m_previewEdit;
    QPushButton *m_previewButton;
    QLabel *m_fileLabel;

    MarkdownHighlighter *m_highlighter;
    LinkParser *m_linkParser;

    QString m_currentFilePath;
    QString m_workspacePath;
    bool m_isModified;
    bool m_previewVisible;

    QMenu *m_editorContextMenu;
};

// Simple markdown syntax highlighter
class MarkdownHighlighter : public QSyntaxHighlighter
{
    Q_OBJECT

public:
    explicit MarkdownHighlighter(QTextDocument *parent = nullptr);

protected:
    void highlightBlock(const QString &text) override;

private:
    struct HighlightingRule
    {
        QRegularExpression pattern;
        QTextCharFormat format;
    };
    QVector<HighlightingRule> highlightingRules;

    QTextCharFormat headerFormat;
    QTextCharFormat boldFormat;
    QTextCharFormat italicFormat;
    QTextCharFormat codeFormat;
    QTextCharFormat linkFormat;
    QTextCharFormat wikiLinkFormat;
};

#endif // EDITOR_H