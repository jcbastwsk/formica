#include "vaultdialog.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QStandardPaths>
#include <QSplitter>

VaultDialog::VaultDialog(QWidget *parent)
    : QDialog(parent)
{
    setupUI();
    refreshVaultList();

    connect(VaultManager::instance(), &VaultManager::vaultListChanged,
            this, &VaultDialog::onVaultListChanged);

    setWindowTitle("Select Vault");
    setModal(true);
    resize(600, 400);
}

void VaultDialog::setupUI()
{
    auto *layout = new QVBoxLayout(this);

    // Main splitter
    auto *splitter = new QSplitter(Qt::Horizontal);

    // Left side - vault list
    auto *leftWidget = new QWidget;
    auto *leftLayout = new QVBoxLayout(leftWidget);

    leftLayout->addWidget(new QLabel("Available Vaults:"));

    m_vaultList = new QListWidget;
    leftLayout->addWidget(m_vaultList);

    // Vault management buttons
    auto *buttonLayout = new QHBoxLayout;
    m_createButton = new QPushButton("Create...");
    m_deleteButton = new QPushButton("Delete");
    m_deleteButton->setEnabled(false);

    buttonLayout->addWidget(m_createButton);
    buttonLayout->addWidget(m_deleteButton);
    buttonLayout->addStretch();

    leftLayout->addLayout(buttonLayout);

    // Right side - vault details
    auto *rightWidget = new QWidget;
    auto *rightLayout = new QVBoxLayout(rightWidget);

    auto *detailsGroup = new QGroupBox("Vault Details");
    auto *detailsLayout = new QVBoxLayout(detailsGroup);

    m_vaultNameLabel = new QLabel("No vault selected");
    m_vaultPathLabel = new QLabel("");
    m_vaultDescLabel = new QLabel("");
    m_lastOpenedLabel = new QLabel("");

    QFont boldFont;
    boldFont.setBold(true);
    m_vaultNameLabel->setFont(boldFont);

    detailsLayout->addWidget(m_vaultNameLabel);
    detailsLayout->addWidget(new QLabel("Path:"));
    detailsLayout->addWidget(m_vaultPathLabel);
    detailsLayout->addWidget(new QLabel("Description:"));
    detailsLayout->addWidget(m_vaultDescLabel);
    detailsLayout->addWidget(new QLabel("Last Opened:"));
    detailsLayout->addWidget(m_lastOpenedLabel);
    detailsLayout->addStretch();

    rightLayout->addWidget(detailsGroup);

    splitter->addWidget(leftWidget);
    splitter->addWidget(rightWidget);
    splitter->setSizes({300, 300});

    layout->addWidget(splitter);

    // Dialog buttons
    auto *dialogButtonLayout = new QHBoxLayout;
    dialogButtonLayout->addStretch();

    m_openButton = new QPushButton("Open Vault");
    m_openButton->setDefault(true);
    m_openButton->setEnabled(false);

    m_cancelButton = new QPushButton("Cancel");

    dialogButtonLayout->addWidget(m_openButton);
    dialogButtonLayout->addWidget(m_cancelButton);

    layout->addLayout(dialogButtonLayout);

    // Connect signals
    connect(m_vaultList, &QListWidget::itemSelectionChanged,
            this, &VaultDialog::onVaultSelectionChanged);
    connect(m_vaultList, &QListWidget::itemDoubleClicked,
            this, &VaultDialog::onOpenVault);

    connect(m_openButton, &QPushButton::clicked, this, &VaultDialog::onOpenVault);
    connect(m_createButton, &QPushButton::clicked, this, &VaultDialog::onCreateVault);
    connect(m_deleteButton, &QPushButton::clicked, this, &VaultDialog::onDeleteVault);
    connect(m_cancelButton, &QPushButton::clicked, this, &QDialog::reject);
}

void VaultDialog::refreshVaultList()
{
    m_vaultList->clear();

    auto vaults = VaultManager::instance()->getAllVaults();
    for (const auto &vault : vaults) {
        auto *item = new QListWidgetItem(vault.name);
        item->setData(Qt::UserRole, vault.name);

        // Show current vault with different styling
        if (vault.name == VaultManager::instance()->currentVaultName()) {
            QFont font = item->font();
            font.setBold(true);
            item->setFont(font);
            item->setText(vault.name + " (current)");
        }

        m_vaultList->addItem(item);
    }
}

void VaultDialog::onVaultSelectionChanged()
{
    auto currentItem = m_vaultList->currentItem();
    if (currentItem) {
        QString vaultName = currentItem->data(Qt::UserRole).toString();
        m_selectedVaultName = vaultName;

        Vault vault = VaultManager::instance()->getVault(vaultName);
        showVaultDetails(vault);

        m_openButton->setEnabled(true);
        m_deleteButton->setEnabled(true);
    } else {
        m_selectedVaultName.clear();
        showVaultDetails(Vault());

        m_openButton->setEnabled(false);
        m_deleteButton->setEnabled(false);
    }
}

void VaultDialog::showVaultDetails(const Vault &vault)
{
    if (vault.isValid()) {
        m_vaultNameLabel->setText(vault.name);
        m_vaultPathLabel->setText(vault.path);
        m_vaultDescLabel->setText(vault.description.isEmpty() ? "No description" : vault.description);
        m_lastOpenedLabel->setText(vault.lastOpened.toString("yyyy-MM-dd hh:mm"));
    } else {
        m_vaultNameLabel->setText("No vault selected");
        m_vaultPathLabel->setText("");
        m_vaultDescLabel->setText("");
        m_lastOpenedLabel->setText("");
    }
}

void VaultDialog::onOpenVault()
{
    if (!m_selectedVaultName.isEmpty()) {
        accept();
    }
}

void VaultDialog::onCreateVault()
{
    CreateVaultDialog dialog(this);
    if (dialog.exec() == QDialog::Accepted) {
        QString name = dialog.vaultName();
        QString path = dialog.vaultPath();
        QString desc = dialog.vaultDescription();

        if (VaultManager::instance()->createVault(name, path, desc)) {
            QMessageBox::information(this, "Success",
                QString("Vault '%1' created successfully!").arg(name));
        } else {
            QMessageBox::warning(this, "Error",
                QString("Failed to create vault '%1'. Check if the name already exists or path is invalid.").arg(name));
        }
    }
}

void VaultDialog::onDeleteVault()
{
    if (m_selectedVaultName.isEmpty()) {
        return;
    }

    QMessageBox::StandardButton reply = QMessageBox::question(this,
        "Delete Vault",
        QString("Are you sure you want to remove vault '%1' from the list?\n\n"
                "Note: This only removes it from Formica - your files won't be deleted.").arg(m_selectedVaultName),
        QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        if (VaultManager::instance()->deleteVault(m_selectedVaultName)) {
            QMessageBox::information(this, "Success", "Vault removed from list.");
        }
    }
}

void VaultDialog::onVaultListChanged()
{
    refreshVaultList();
}

QString VaultDialog::selectedVaultName() const
{
    return m_selectedVaultName;
}

// CreateVaultDialog implementation

CreateVaultDialog::CreateVaultDialog(QWidget *parent)
    : QDialog(parent)
{
    setupUI();
    setWindowTitle("Create New Vault");
    setModal(true);
    resize(500, 300);
}

void CreateVaultDialog::setupUI()
{
    auto *layout = new QVBoxLayout(this);

    // Vault name
    layout->addWidget(new QLabel("Vault Name:"));
    m_nameEdit = new QLineEdit;
    m_nameEdit->setPlaceholderText("My Knowledge Base");
    layout->addWidget(m_nameEdit);

    // Vault path
    layout->addWidget(new QLabel("Vault Location:"));
    auto *pathLayout = new QHBoxLayout;

    m_pathEdit = new QLineEdit;
    QString defaultPath = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) + "/Vaults";
    m_pathEdit->setText(defaultPath);

    m_browseButton = new QPushButton("Browse...");

    pathLayout->addWidget(m_pathEdit);
    pathLayout->addWidget(m_browseButton);
    layout->addLayout(pathLayout);

    // Description
    layout->addWidget(new QLabel("Description (optional):"));
    m_descEdit = new QTextEdit;
    m_descEdit->setMaximumHeight(80);
    m_descEdit->setPlaceholderText("Describe what this vault will contain...");
    layout->addWidget(m_descEdit);

    // Dialog buttons
    auto *buttonLayout = new QHBoxLayout;
    buttonLayout->addStretch();

    m_okButton = new QPushButton("Create");
    m_okButton->setDefault(true);
    m_okButton->setEnabled(false);

    m_cancelButton = new QPushButton("Cancel");

    buttonLayout->addWidget(m_okButton);
    buttonLayout->addWidget(m_cancelButton);
    layout->addLayout(buttonLayout);

    // Connect signals
    connect(m_browseButton, &QPushButton::clicked, this, &CreateVaultDialog::onBrowsePath);
    connect(m_nameEdit, &QLineEdit::textChanged, this, &CreateVaultDialog::validateInput);
    connect(m_pathEdit, &QLineEdit::textChanged, this, &CreateVaultDialog::validateInput);

    connect(m_okButton, &QPushButton::clicked, this, &QDialog::accept);
    connect(m_cancelButton, &QPushButton::clicked, this, &QDialog::reject);

    validateInput();
}

void CreateVaultDialog::onBrowsePath()
{
    QString dir = QFileDialog::getExistingDirectory(this,
        "Select Vault Location", m_pathEdit->text());
    if (!dir.isEmpty()) {
        // Append vault name to path
        QString vaultName = m_nameEdit->text();
        if (!vaultName.isEmpty()) {
            dir = QDir(dir).filePath(vaultName);
        }
        m_pathEdit->setText(dir);
    }
}

void CreateVaultDialog::validateInput()
{
    bool valid = !m_nameEdit->text().trimmed().isEmpty() &&
                 !m_pathEdit->text().trimmed().isEmpty();
    m_okButton->setEnabled(valid);
}

QString CreateVaultDialog::vaultName() const
{
    return m_nameEdit->text().trimmed();
}

QString CreateVaultDialog::vaultPath() const
{
    return m_pathEdit->text().trimmed();
}

QString CreateVaultDialog::vaultDescription() const
{
    return m_descEdit->toPlainText().trimmed();
}