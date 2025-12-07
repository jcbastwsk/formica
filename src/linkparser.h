#ifndef LINKPARSER_H
#define LINKPARSER_H

#include <QObject>
#include <QString>
#include <QRegularExpression>
#include <QStringList>
#include <QDir>

struct WikiLink {
    QString linkText;    // The text inside [[]]
    QString displayText; // Text to display (may differ from linkText)
    QString targetFile;  // Actual file path if found
    int startPos;        // Position in text
    int length;          // Length of the link
    bool exists;         // Whether target file exists
};

struct ZettelId {
    QString id;          // The zettel ID (1, 1a, 1a1, etc.)
    QString title;       // Optional title after ID
    bool isValid;        // Whether ID follows zettelkasten format
};

class LinkParser : public QObject
{
    Q_OBJECT

public:
    explicit LinkParser(QObject *parent = nullptr);

    // Parse wiki links in text
    QList<WikiLink> parseWikiLinks(const QString &text, const QString &workspacePath = QString());

    // Parse zettel IDs
    ZettelId parseZettelId(const QString &text);

    // Validate zettelkasten numbering
    bool isValidZettelId(const QString &id);

    // Generate next zettel ID
    QString generateNextZettelId(const QString &parentId, const QString &workspacePath);
    QString generateChildZettelId(const QString &parentId, const QString &workspacePath);

    // Find existing notes by ID or title
    QString findNoteByTitle(const QString &title, const QString &workspacePath);
    QString findNoteById(const QString &zettelId, const QString &workspacePath);

    // Get all zettel IDs in workspace
    QStringList getAllZettelIds(const QString &workspacePath);

    // Extract backlinks (notes that link to this note)
    QStringList findBacklinks(const QString &notePath, const QString &workspacePath);

private:
    QString normalizeTitle(const QString &title);
    QString zettelIdToFileName(const QString &zettelId, const QString &title = QString());
    bool isZettelFileName(const QString &fileName);
    QString extractZettelIdFromFileName(const QString &fileName);

    QRegularExpression m_wikiLinkRegex;
    QRegularExpression m_zettelIdRegex;
};

#endif // LINKPARSER_H