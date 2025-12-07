#include "vaultmanager.h"
#include <QStandardPaths>
#include <QDateTime>
#include <QUuid>
#include <QFileInfo>
#include <QDirIterator>

VaultManager* VaultManager::s_instance = nullptr;

VaultManager::VaultManager(QObject *parent)
    : QObject(parent)
    , m_settings(new QSettings("Formica", "VaultManager", this))
{
    loadVaults();
}

VaultManager* VaultManager::instance()
{
    if (!s_instance) {
        s_instance = new VaultManager();
    }
    return s_instance;
}

QStringList VaultManager::getVaultNames() const
{
    QStringList names;
    for (const auto &vault : m_vaults) {
        names.append(vault.name);
    }
    return names;
}

QList<Vault> VaultManager::getAllVaults() const
{
    return m_vaults;
}

Vault VaultManager::getCurrentVault() const
{
    return m_currentVault;
}

Vault VaultManager::getVault(const QString &name) const
{
    for (const auto &vault : m_vaults) {
        if (vault.name == name) {
            return vault;
        }
    }
    return Vault();
}

bool VaultManager::createVault(const QString &name, const QString &path, const QString &description)
{
    // Check if vault name already exists
    if (getVault(name).isValid()) {
        return false;
    }

    // Create directory if it doesn't exist
    QDir dir;
    if (!dir.mkpath(path)) {
        return false;
    }

    // Create vault structure
    createVaultStructure(path);

    // Add to vault list
    Vault newVault;
    newVault.name = name;
    newVault.path = path;
    newVault.description = description;
    newVault.lastOpened = QDateTime::currentDateTime();

    m_vaults.append(newVault);
    saveVaults();

    emit vaultListChanged();
    return true;
}

bool VaultManager::openVault(const QString &name)
{
    Vault vault = getVault(name);
    if (!vault.isValid()) {
        return false;
    }

    // Update last opened time
    for (auto &v : m_vaults) {
        if (v.name == name) {
            v.lastOpened = QDateTime::currentDateTime();
            break;
        }
    }

    m_currentVault = vault;
    saveVaults();

    emit vaultChanged(vault);
    return true;
}

bool VaultManager::deleteVault(const QString &name)
{
    for (int i = 0; i < m_vaults.size(); ++i) {
        if (m_vaults[i].name == name) {
            // If this is the current vault, clear it
            if (m_currentVault.name == name) {
                m_currentVault = Vault();
            }

            m_vaults.removeAt(i);
            saveVaults();
            emit vaultListChanged();
            return true;
        }
    }
    return false;
}

bool VaultManager::renameVault(const QString &oldName, const QString &newName)
{
    if (getVault(newName).isValid()) {
        return false; // New name already exists
    }

    for (auto &vault : m_vaults) {
        if (vault.name == oldName) {
            vault.name = newName;

            // Update current vault if it's the one being renamed
            if (m_currentVault.name == oldName) {
                m_currentVault.name = newName;
            }

            saveVaults();
            emit vaultListChanged();
            return true;
        }
    }
    return false;
}

void VaultManager::scanForVaults(const QString &basePath)
{
    QDirIterator it(basePath, QDir::Dirs | QDir::NoDotAndDotDot, QDirIterator::NoIteratorFlags);

    while (it.hasNext()) {
        QString dirPath = it.next();
        if (isValidVaultPath(dirPath)) {
            QFileInfo info(dirPath);
            QString vaultName = info.baseName();

            // Only add if not already in our list
            if (!getVault(vaultName).isValid()) {
                Vault vault;
                vault.name = vaultName;
                vault.path = dirPath;
                vault.description = "Discovered vault";
                vault.lastOpened = info.lastModified();

                m_vaults.append(vault);
            }
        }
    }

    if (!m_vaults.isEmpty()) {
        saveVaults();
        emit vaultListChanged();
    }
}

bool VaultManager::isValidVaultPath(const QString &path) const
{
    QDir dir(path);

    // Check if it contains markdown files or looks like a notes directory
    QStringList entries = dir.entryList(QStringList() << "*.md" << "*.txt", QDir::Files);

    // Or has a .formica-vault marker file
    if (dir.exists(".formica-vault")) {
        return true;
    }

    // Or contains at least one markdown file
    return !entries.isEmpty();
}

void VaultManager::loadVaults()
{
    int size = m_settings->beginReadArray("vaults");
    for (int i = 0; i < size; ++i) {
        m_settings->setArrayIndex(i);

        Vault vault;
        vault.name = m_settings->value("name").toString();
        vault.path = m_settings->value("path").toString();
        vault.description = m_settings->value("description").toString();
        vault.lastOpened = m_settings->value("lastOpened").toDateTime();

        if (vault.isValid()) {
            m_vaults.append(vault);
        }
    }
    m_settings->endArray();

    // Load current vault
    QString currentVaultName = m_settings->value("currentVault").toString();
    if (!currentVaultName.isEmpty()) {
        m_currentVault = getVault(currentVaultName);
    }
}

void VaultManager::saveVaults()
{
    m_settings->beginWriteArray("vaults");
    for (int i = 0; i < m_vaults.size(); ++i) {
        m_settings->setArrayIndex(i);
        const Vault &vault = m_vaults[i];

        m_settings->setValue("name", vault.name);
        m_settings->setValue("path", vault.path);
        m_settings->setValue("description", vault.description);
        m_settings->setValue("lastOpened", vault.lastOpened);
    }
    m_settings->endArray();

    // Save current vault
    m_settings->setValue("currentVault", m_currentVault.name);
    m_settings->sync();
}

void VaultManager::createVaultStructure(const QString &vaultPath)
{
    QDir dir(vaultPath);

    // Create .formica-vault marker
    QFile markerFile(dir.filePath(".formica-vault"));
    if (markerFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&markerFile);
        out << "# Formica Vault\n";
        out << "Created: " << QDateTime::currentDateTime().toString(Qt::ISODate) << "\n";
        markerFile.close();
    }

    // Create welcome file
    QFile welcomeFile(dir.filePath("Welcome.md"));
    if (welcomeFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&welcomeFile);
        out << "# Welcome to Your Formica Vault\n\n";
        out << "This is your personal knowledge base. Start creating notes and linking them together!\n\n";
        out << "## Getting Started\n\n";
        out << "1. **Create your first Zettel**: Press `Ctrl+Shift+N` to create a new Zettel with automatic numbering\n";
        out << "2. **Daily notes**: Press `Ctrl+D` to create or open today's daily note\n";
        out << "3. **Wiki links**: Use `[[Note Name]]` to link between notes\n";
        out << "4. **Search**: Press `Ctrl+F` to search across all your notes\n\n";
        out << "## Zettelkasten System\n\n";
        out << "Formica follows the traditional Zettelkasten numbering:\n";
        out << "- `1` - Main topic\n";
        out << "- `1a` - Subtopic of 1\n";
        out << "- `1a1` - Subtopic of 1a\n";
        out << "- `1a1a` - Subtopic of 1a1\n";
        out << "- `2` - Another main topic\n\n";
        out << "Happy note-taking! 📝\n";
        welcomeFile.close();
    }
}

QString VaultManager::generateVaultId() const
{
    return QUuid::createUuid().toString(QUuid::WithoutBraces);
}