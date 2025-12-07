#include "linkparser.h"
#include <QFileInfo>
#include <QDirIterator>
#include <QTextStream>
#include <QFile>
#include <QDebug>

LinkParser::LinkParser(QObject *parent)
    : QObject(parent)
{
    // Regex for [[wiki links]] - supports [[title]] or [[id|title]]
    m_wikiLinkRegex = QRegularExpression(R"(\[\[([^\]]+)\]\])");

    // Regex for zettelkasten IDs: number followed by letters
    // Matches: 1, 1a, 1a1, 1a1a, 2, 2b3, etc.
    m_zettelIdRegex = QRegularExpression(R"(^(\d+(?:[a-z]+\d*)*))");
}

QList<WikiLink> LinkParser::parseWikiLinks(const QString &text, const QString &workspacePath)
{
    QList<WikiLink> links;

    QRegularExpressionMatchIterator iterator = m_wikiLinkRegex.globalMatch(text);
    while (iterator.hasNext()) {
        QRegularExpressionMatch match = iterator.next();
        WikiLink link;

        QString fullMatch = match.captured(1); // Content inside [[]]

        // Check for [[id|title]] format
        if (fullMatch.contains("|")) {
            QStringList parts = fullMatch.split("|");
            link.linkText = parts[0].trimmed();
            link.displayText = parts[1].trimmed();
        } else {
            link.linkText = fullMatch.trimmed();
            link.displayText = fullMatch.trimmed();
        }

        link.startPos = match.capturedStart();
        link.length = match.capturedLength();

        // Find target file
        if (!workspacePath.isEmpty()) {
            // First try to find by zettel ID
            link.targetFile = findNoteById(link.linkText, workspacePath);

            // If not found, try by title
            if (link.targetFile.isEmpty()) {
                link.targetFile = findNoteByTitle(link.linkText, workspacePath);
            }

            link.exists = !link.targetFile.isEmpty() && QFileInfo::exists(link.targetFile);
        } else {
            link.exists = false;
        }

        links.append(link);
    }

    return links;
}

ZettelId LinkParser::parseZettelId(const QString &text)
{
    ZettelId zettel;
    zettel.isValid = false;

    QString trimmed = text.trimmed();

    // Try to match zettelkasten ID at the beginning
    QRegularExpressionMatch match = m_zettelIdRegex.match(trimmed);
    if (match.hasMatch()) {
        zettel.id = match.captured(1);
        zettel.isValid = isValidZettelId(zettel.id);

        // Extract title (everything after the ID)
        QString remaining = trimmed.mid(match.capturedEnd()).trimmed();
        if (remaining.startsWith("-") || remaining.startsWith(":")) {
            remaining = remaining.mid(1).trimmed();
        }
        zettel.title = remaining;
    }

    return zettel;
}

bool LinkParser::isValidZettelId(const QString &id)
{
    // Valid zettelkasten IDs: 1, 1a, 1a1, 1a1a, 2b, 2b3c, etc.
    QRegularExpression validIdRegex(R"(^\d+(?:[a-z]+\d*)*$)");
    return validIdRegex.match(id).hasMatch();
}

QString LinkParser::generateNextZettelId(const QString &parentId, const QString &workspacePath)
{
    if (parentId.isEmpty()) {
        // Generate top-level ID (1, 2, 3, ...)
        QStringList existingIds = getAllZettelIds(workspacePath);

        int maxNum = 0;
        QRegularExpression topLevelRegex("^(\\d+)$");

        for (const QString &id : existingIds) {
            QRegularExpressionMatch match = topLevelRegex.match(id);
            if (match.hasMatch()) {
                int num = match.captured(1).toInt();
                maxNum = qMax(maxNum, num);
            }
        }

        return QString::number(maxNum + 1);
    }

    return generateChildZettelId(parentId, workspacePath);
}

QString LinkParser::generateChildZettelId(const QString &parentId, const QString &workspacePath)
{
    if (!isValidZettelId(parentId)) {
        return QString();
    }

    QStringList existingIds = getAllZettelIds(workspacePath);

    // For parent "1", generate "1a", "1b", "1c", etc.
    // For parent "1a", generate "1a1", "1a2", "1a3", etc.
    // For parent "1a1", generate "1a1a", "1a1b", "1a1c", etc.

    if (parentId.at(parentId.length() - 1).isDigit()) {
        // Last character is a number, append letters
        QString baseId = parentId + "a";
        if (!existingIds.contains(baseId)) {
            return baseId;
        }

        // Find next available letter
        for (char c = 'a'; c <= 'z'; ++c) {
            QString candidateId = parentId + c;
            if (!existingIds.contains(candidateId)) {
                return candidateId;
            }
        }
    } else {
        // Last character is a letter, append numbers
        QString baseId = parentId + "1";
        if (!existingIds.contains(baseId)) {
            return baseId;
        }

        // Find next available number
        for (int i = 1; i <= 999; ++i) {
            QString candidateId = parentId + QString::number(i);
            if (!existingIds.contains(candidateId)) {
                return candidateId;
            }
        }
    }

    return QString(); // Should rarely happen
}

QString LinkParser::findNoteByTitle(const QString &title, const QString &workspacePath)
{
    QString normalizedTitle = normalizeTitle(title);

    QDirIterator iterator(workspacePath,
                          QStringList() << "*.md" << "*.txt",
                          QDir::Files,
                          QDirIterator::Subdirectories);

    while (iterator.hasNext()) {
        QString filePath = iterator.next();
        QFileInfo fileInfo(filePath);

        // Check if filename (without extension) matches
        QString baseFileName = fileInfo.completeBaseName();
        if (normalizeTitle(baseFileName) == normalizedTitle) {
            return filePath;
        }

        // Check if file contains this title as a header
        QFile file(filePath);
        if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QTextStream in(&file);
            QString firstLine = in.readLine();
            if (firstLine.startsWith("#")) {
                QString headerTitle = firstLine.remove(QRegularExpression("^#+\\s*")).trimmed();
                if (normalizeTitle(headerTitle) == normalizedTitle) {
                    return filePath;
                }
            }
        }
    }

    return QString();
}

QString LinkParser::findNoteById(const QString &zettelId, const QString &workspacePath)
{
    if (!isValidZettelId(zettelId)) {
        return QString();
    }

    QDirIterator iterator(workspacePath,
                          QStringList() << "*.md" << "*.txt",
                          QDir::Files,
                          QDirIterator::Subdirectories);

    while (iterator.hasNext()) {
        QString filePath = iterator.next();
        QFileInfo fileInfo(filePath);

        // Check if filename starts with zettel ID
        QString fileName = fileInfo.baseName();
        if (fileName.startsWith(zettelId + " ") || fileName == zettelId) {
            return filePath;
        }

        // Also check file content for zettel ID at the beginning
        QFile file(filePath);
        if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QTextStream in(&file);
            QString firstLine = in.readLine().trimmed();
            ZettelId parsedId = parseZettelId(firstLine);
            if (parsedId.isValid && parsedId.id == zettelId) {
                return filePath;
            }
        }
    }

    return QString();
}

QStringList LinkParser::getAllZettelIds(const QString &workspacePath)
{
    QStringList zettelIds;

    QDirIterator iterator(workspacePath,
                          QStringList() << "*.md" << "*.txt",
                          QDir::Files,
                          QDirIterator::Subdirectories);

    while (iterator.hasNext()) {
        QString filePath = iterator.next();
        QFileInfo fileInfo(filePath);
        QString fileName = fileInfo.baseName();

        // Try to extract zettel ID from filename
        QString idFromName = extractZettelIdFromFileName(fileName);
        if (!idFromName.isEmpty()) {
            zettelIds.append(idFromName);
            continue;
        }

        // Try to extract from file content
        QFile file(filePath);
        if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QTextStream in(&file);
            QString firstLine = in.readLine().trimmed();
            ZettelId parsedId = parseZettelId(firstLine);
            if (parsedId.isValid) {
                zettelIds.append(parsedId.id);
            }
        }
    }

    zettelIds.removeDuplicates();
    return zettelIds;
}

QStringList LinkParser::findBacklinks(const QString &notePath, const QString &workspacePath)
{
    QStringList backlinks;

    QFileInfo noteInfo(notePath);
    QString noteTitle = noteInfo.completeBaseName();

    // Extract zettel ID if this note has one
    QString zettelId = extractZettelIdFromFileName(noteTitle);

    QDirIterator iterator(workspacePath,
                          QStringList() << "*.md" << "*.txt",
                          QDir::Files,
                          QDirIterator::Subdirectories);

    while (iterator.hasNext()) {
        QString filePath = iterator.next();
        if (filePath == notePath) continue; // Skip self

        QFile file(filePath);
        if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QString content = QTextStream(&file).readAll();

            // Parse wiki links in this file
            auto links = parseWikiLinks(content);
            for (const auto &link : links) {
                // Check if this link points to our note
                if ((link.linkText == noteTitle) ||
                    (link.linkText == zettelId && !zettelId.isEmpty()) ||
                    (link.targetFile == notePath)) {
                    backlinks.append(filePath);
                    break; // Only add once per file
                }
            }
        }
    }

    return backlinks;
}

QString LinkParser::normalizeTitle(const QString &title)
{
    return title.trimmed().toLower().replace(" ", "_");
}

QString LinkParser::zettelIdToFileName(const QString &zettelId, const QString &title)
{
    if (title.isEmpty()) {
        return QString("%1.md").arg(zettelId);
    }
    return QString("%1 %2.md").arg(zettelId, title);
}

bool LinkParser::isZettelFileName(const QString &fileName)
{
    return m_zettelIdRegex.match(fileName).hasMatch();
}

QString LinkParser::extractZettelIdFromFileName(const QString &fileName)
{
    QRegularExpressionMatch match = m_zettelIdRegex.match(fileName);
    if (match.hasMatch()) {
        return match.captured(1);
    }
    return QString();
}