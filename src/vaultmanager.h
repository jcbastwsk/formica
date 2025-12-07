#ifndef VAULTMANAGER_H
#define VAULTMANAGER_H

#include <QObject>
#include <QString>
#include <QStringList>
#include <QDir>
#include <QSettings>
#include <QDateTime>

struct Vault {
    QString name;
    QString path;
    QString description;
    QDateTime lastOpened;
    bool isValid() const { return !name.isEmpty() && !path.isEmpty() && QDir(path).exists(); }
};

class VaultManager : public QObject
{
    Q_OBJECT

public:
    static VaultManager* instance();

    // Vault operations
    QStringList getVaultNames() const;
    QList<Vault> getAllVaults() const;
    Vault getCurrentVault() const;
    Vault getVault(const QString &name) const;

    bool createVault(const QString &name, const QString &path, const QString &description = QString());
    bool openVault(const QString &name);
    bool deleteVault(const QString &name);
    bool renameVault(const QString &oldName, const QString &newName);

    // Current vault
    QString currentVaultName() const { return m_currentVault.name; }
    QString currentVaultPath() const { return m_currentVault.path; }
    bool hasCurrentVault() const { return m_currentVault.isValid(); }

    // Vault discovery
    void scanForVaults(const QString &basePath);
    bool isValidVaultPath(const QString &path) const;

signals:
    void vaultChanged(const Vault &vault);
    void vaultListChanged();

private:
    explicit VaultManager(QObject *parent = nullptr);
    void loadVaults();
    void saveVaults();
    void createVaultStructure(const QString &vaultPath);
    QString generateVaultId() const;

    QSettings *m_settings;
    QList<Vault> m_vaults;
    Vault m_currentVault;

    static VaultManager *s_instance;
};

#endif // VAULTMANAGER_H