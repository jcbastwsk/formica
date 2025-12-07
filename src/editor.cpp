#include "editor.h"
#include "linkparser.h"
#include "settings.h"
#include <QFile>
#include <QTextStream>
#include <QMessageBox>
#include <QFileDialog>
#include <QFileInfo>
#include <QFont>
#include <QSyntaxHighlighter>
#include <QTextCharFormat>
#include <QRegularExpression>
#include <QApplication>
#include <QMouseEvent>
#include <QEvent>
#include <QDesktopServices>
#include <QUrl>
#include <QProcess>
#include <QStandardPaths>

Editor::Editor(QWidget *parent)
    : QWidget(parent), m_isModified(false), m_previewVisible(false)
{
    setupUI();
    setupEditorContextMenu();
    m_linkParser = new LinkParser(this);

    // Apply current font settings
    applyCurrentSettings();

    // Connect to settings changes
    connect(Settings::instance(), &Settings::fontChanged, this, &Editor::onFontChanged);
}

void Editor::setupUI()
{
    auto *layout = new QVBoxLayout(this);

    // Top bar with file info and preview button
    auto *topBar = new QHBoxLayout;
    m_fileLabel = new QLabel("No file opened");
    m_previewButton = new QPushButton("Show Preview");
    m_previewButton->setCheckable(true);

    topBar->addWidget(m_fileLabel);
    topBar->addStretch();
    topBar->addWidget(m_previewButton);

    // Main editor area
    m_splitter = new QSplitter(Qt::Horizontal);

    // Text editor
    m_textEdit = new QTextEdit;
    m_textEdit->setAcceptRichText(false);
    m_textEdit->setLineWrapMode(QTextEdit::WidgetWidth);

    // Set a monospace font
    QFont font("Courier");
    font.setPointSize(12);
    m_textEdit->setFont(font);

    // Syntax highlighter
    m_highlighter = new MarkdownHighlighter(m_textEdit->document());

    // Preview area (initially hidden)
    m_previewEdit = new QTextEdit;
    m_previewEdit->setReadOnly(true);
    m_previewEdit->hide();

    m_splitter->addWidget(m_textEdit);
    m_splitter->addWidget(m_previewEdit);

    layout->addLayout(topBar);
    layout->addWidget(m_splitter);

    // Connect signals
    connect(m_textEdit, &QTextEdit::textChanged, this, &Editor::onTextChanged);
    connect(m_previewButton, &QPushButton::clicked, this, &Editor::togglePreview);

    // Enable mouse tracking for link clicks
    m_textEdit->setMouseTracking(true);
    m_textEdit->viewport()->installEventFilter(this);

    // Enable context menu for editor
    m_textEdit->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(m_textEdit, &QTextEdit::customContextMenuRequested, this, &Editor::showEditorContextMenu);
}

bool Editor::loadFile(const QString &filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "Error", "Cannot read file " + filePath);
        return false;
    }

    QTextStream in(&file);
    m_textEdit->setPlainText(in.readAll());

    setCurrentFile(filePath);
    m_isModified = false;
    updatePreview();

    return true;
}

bool Editor::saveFile()
{
    if (m_currentFilePath.isEmpty()) {
        QString fileName = QFileDialog::getSaveFileName(this,
            "Save File", "", "Markdown Files (*.md);;Text Files (*.txt)");
        if (fileName.isEmpty()) {
            return false;
        }
        setCurrentFile(fileName);
    }

    QFile file(m_currentFilePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "Error", "Cannot write file " + m_currentFilePath);
        return false;
    }

    QTextStream out(&file);
    out << m_textEdit->toPlainText();

    m_isModified = false;
    updatePreview();

    return true;
}

void Editor::newFile()
{
    m_textEdit->clear();
    setCurrentFile("");
    m_isModified = false;
}

bool Editor::isModified() const
{
    return m_isModified;
}

void Editor::onTextChanged()
{
    m_isModified = true;
    updatePreview();
}

void Editor::togglePreview()
{
    m_previewVisible = !m_previewVisible;

    if (m_previewVisible) {
        m_previewEdit->show();
        m_previewButton->setText("Hide Preview");
        m_splitter->setSizes({600, 600});
        updatePreview();
    } else {
        m_previewEdit->hide();
        m_previewButton->setText("Show Preview");
    }
}

void Editor::updatePreview()
{
    if (!m_previewVisible) {
        return;
    }

    // Simple markdown to HTML conversion
    QString text = m_textEdit->toPlainText();
    QString html = text;

    // Convert headers
    html.replace(QRegularExpression("^### (.+)$", QRegularExpression::MultilineOption), "<h3>\\1</h3>");
    html.replace(QRegularExpression("^## (.+)$", QRegularExpression::MultilineOption), "<h2>\\1</h2>");
    html.replace(QRegularExpression("^# (.+)$", QRegularExpression::MultilineOption), "<h1>\\1</h1>");

    // Convert bold and italic
    html.replace(QRegularExpression("\\*\\*(.+?)\\*\\*"), "<strong>\\1</strong>");
    html.replace(QRegularExpression("\\*(.+?)\\*"), "<em>\\1</em>");

    // Convert code blocks
    html.replace(QRegularExpression("```([\\s\\S]*?)```"), "<pre><code>\\1</code></pre>");
    html.replace(QRegularExpression("`(.+?)`"), "<code>\\1</code>");

    // Convert links
    html.replace(QRegularExpression("\\[(.+?)\\]\\((.+?)\\)"), "<a href=\"\\2\">\\1</a>");

    // Convert line breaks
    html.replace("\n", "<br>");

    m_previewEdit->setHtml(html);
}

void Editor::setCurrentFile(const QString &filePath)
{
    m_currentFilePath = filePath;

    if (filePath.isEmpty()) {
        m_fileLabel->setText("New file");
    } else {
        QFileInfo fileInfo(filePath);
        m_fileLabel->setText(fileInfo.fileName());
    }
}

bool Editor::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == m_textEdit->viewport() && event->type() == QEvent::MouseButtonPress) {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
        if (mouseEvent->button() == Qt::LeftButton) {
            QString link = getLinkAtPosition(mouseEvent->pos());
            if (!link.isEmpty()) {
                emit linkClicked(link);
                return true;
            }
        }
    }
    return QWidget::eventFilter(obj, event);
}

QString Editor::getLinkAtPosition(const QPoint &pos)
{
    QTextCursor cursor = m_textEdit->cursorForPosition(pos);
    cursor.select(QTextCursor::WordUnderCursor);

    // Get the text around the cursor to find wiki links
    QTextCursor lineCursor = m_textEdit->cursorForPosition(pos);
    lineCursor.movePosition(QTextCursor::StartOfLine);
    lineCursor.movePosition(QTextCursor::EndOfLine, QTextCursor::KeepAnchor);
    QString lineText = lineCursor.selectedText();

    // Parse wiki links in this line
    auto links = m_linkParser->parseWikiLinks(lineText, m_workspacePath);

    // Find which link was clicked
    int positionInLine = cursor.position() - lineCursor.anchor();
    for (const auto &link : links) {
        if (positionInLine >= link.startPos && positionInLine <= link.startPos + link.length) {
            return link.linkText;
        }
    }

    return QString();
}

void Editor::onLinkClicked()
{
    // This slot can be used for programmatic link clicks
}

void Editor::setupEditorContextMenu()
{
    m_editorContextMenu = new QMenu(this);

    auto *showInExplorerAction = m_editorContextMenu->addAction("Show Current File in Explorer");
    connect(showInExplorerAction, &QAction::triggered, this, &Editor::showCurrentFileInExplorer);
}

void Editor::showEditorContextMenu(const QPoint &pos)
{
    if (!m_currentFilePath.isEmpty()) {
        m_editorContextMenu->exec(m_textEdit->mapToGlobal(pos));
    }
}

void Editor::showCurrentFileInExplorer()
{
    if (!m_currentFilePath.isEmpty()) {
        openFileManagerAndSelect(m_currentFilePath);
    }
}

void Editor::openFileManagerAndSelect(const QString &filePath)
{
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
    QStringList candidates = {
        "dolphin",      // KDE
        "nautilus",     // GNOME
        "thunar",       // XFCE
        "pcmanfm",      // LXDE
        "nemo",         // Cinnamon
        "caja"          // MATE
    };

    QString fileManager;
    for (const QString &manager : candidates) {
        QString path = QStandardPaths::findExecutable(manager);
        if (!path.isEmpty()) {
            fileManager = path;
            break;
        }
    }

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

void Editor::onFontChanged(const QFont &font)
{
    applyCurrentSettings();
}

void Editor::applyCurrentSettings()
{
    Settings *settings = Settings::instance();

    // Apply font to text editor
    m_textEdit->setFont(settings->editorFont());

    // Apply line wrapping
    m_textEdit->setLineWrapMode(settings->lineWrapping() ? QTextEdit::WidgetWidth : QTextEdit::NoWrap);

    // Update highlighter colors for current theme
    if (m_highlighter) {
        // The highlighter will need to be updated for different themes
        // For now, we'll just trigger a rehighlight
        m_highlighter->rehighlight();
    }
}

// MarkdownHighlighter implementation

MarkdownHighlighter::MarkdownHighlighter(QTextDocument *parent)
    : QSyntaxHighlighter(parent)
{
    HighlightingRule rule;

    // Header format
    headerFormat.setForeground(QColor(0, 0, 255));
    headerFormat.setFontWeight(QFont::Bold);
    rule.pattern = QRegularExpression("^#{1,6}\\s.*");
    rule.format = headerFormat;
    highlightingRules.append(rule);

    // Bold format
    boldFormat.setFontWeight(QFont::Bold);
    rule.pattern = QRegularExpression("\\*\\*([^*]+)\\*\\*");
    rule.format = boldFormat;
    highlightingRules.append(rule);

    // Italic format
    italicFormat.setFontItalic(true);
    rule.pattern = QRegularExpression("\\*([^*]+)\\*");
    rule.format = italicFormat;
    highlightingRules.append(rule);

    // Code format
    codeFormat.setForeground(QColor(200, 100, 0));
    codeFormat.setFontFamilies({"Courier"});
    rule.pattern = QRegularExpression("`([^`]+)`");
    rule.format = codeFormat;
    highlightingRules.append(rule);

    // Link format
    linkFormat.setForeground(QColor(0, 100, 200));
    linkFormat.setFontUnderline(true);
    rule.pattern = QRegularExpression("\\[([^]]+)\\]\\(([^)]+)\\)");
    rule.format = linkFormat;
    highlightingRules.append(rule);

    // Wiki link format
    wikiLinkFormat.setForeground(QColor(0, 150, 0));
    wikiLinkFormat.setFontUnderline(true);
    wikiLinkFormat.setFontWeight(QFont::Bold);
    rule.pattern = QRegularExpression("\\[\\[([^\\]]+)\\]\\]");
    rule.format = wikiLinkFormat;
    highlightingRules.append(rule);
}

void MarkdownHighlighter::highlightBlock(const QString &text)
{
    foreach (const HighlightingRule &rule, highlightingRules) {
        QRegularExpressionMatchIterator matchIterator = rule.pattern.globalMatch(text);
        while (matchIterator.hasNext()) {
            QRegularExpressionMatch match = matchIterator.next();
            setFormat(match.capturedStart(), match.capturedLength(), rule.format);
        }
    }
}

