#ifndef VAULTDIALOG_H
#define VAULTDIALOG_H

#include <QDialog>
#include <QListWidget>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QLineEdit>
#include <QTextEdit>
#include "vaultmanager.h"

class VaultDialog : public QDialog
{
    Q_OBJECT

public:
    explicit VaultDialog(QWidget *parent = nullptr);

    QString selectedVaultName() const;

private slots:
    void onVaultSelectionChanged();
    void onOpenVault();
    void onCreateVault();
    void onDeleteVault();
    void onVaultListChanged();

private:
    void setupUI();
    void refreshVaultList();
    void showVaultDetails(const Vault &vault);

    QListWidget *m_vaultList;
    QLabel *m_vaultNameLabel;
    QLabel *m_vaultPathLabel;
    QLabel *m_vaultDescLabel;
    QLabel *m_lastOpenedLabel;

    QPushButton *m_openButton;
    QPushButton *m_createButton;
    QPushButton *m_deleteButton;
    QPushButton *m_cancelButton;

    QString m_selectedVaultName;
};

class CreateVaultDialog : public QDialog
{
    Q_OBJECT

public:
    explicit CreateVaultDialog(QWidget *parent = nullptr);

    QString vaultName() const;
    QString vaultPath() const;
    QString vaultDescription() const;

private slots:
    void onBrowsePath();
    void validateInput();

private:
    void setupUI();

    QLineEdit *m_nameEdit;
    QLineEdit *m_pathEdit;
    QTextEdit *m_descEdit;
    QPushButton *m_browseButton;
    QPushButton *m_okButton;
    QPushButton *m_cancelButton;
};

#endif // VAULTDIALOG_H