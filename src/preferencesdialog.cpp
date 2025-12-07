#include "preferencesdialog.h"
#include <QDialogButtonBox>
#include <QApplication>

PreferencesDialog::PreferencesDialog(QWidget *parent)
    : QDialog(parent)
{
    setupUI();
    loadCurrentSettings();

    setWindowTitle("Preferences");
    setModal(true);
    resize(500, 400);
}

void PreferencesDialog::setupUI()
{
    auto *layout = new QVBoxLayout(this);

    m_tabWidget = new QTabWidget;

    setupAppearanceTab();
    setupEditorTab();

    m_tabWidget->addTab(m_appearanceTab, "Appearance");
    m_tabWidget->addTab(m_editorTab, "Editor");

    layout->addWidget(m_tabWidget);

    // Dialog buttons
    auto *buttonLayout = new QHBoxLayout;

    m_resetButton = new QPushButton("Reset to Defaults");
    m_applyButton = new QPushButton("Apply");
    m_cancelButton = new QPushButton("Cancel");
    m_okButton = new QPushButton("OK");

    m_okButton->setDefault(true);

    buttonLayout->addWidget(m_resetButton);
    buttonLayout->addStretch();
    buttonLayout->addWidget(m_applyButton);
    buttonLayout->addWidget(m_cancelButton);
    buttonLayout->addWidget(m_okButton);

    layout->addLayout(buttonLayout);

    // Connect buttons
    connect(m_okButton, &QPushButton::clicked, this, [this]() {
        applySettings();
        accept();
    });
    connect(m_cancelButton, &QPushButton::clicked, this, &QDialog::reject);
    connect(m_applyButton, &QPushButton::clicked, this, &PreferencesDialog::applySettings);
    connect(m_resetButton, &QPushButton::clicked, this, &PreferencesDialog::resetToDefaults);
}

void PreferencesDialog::setupAppearanceTab()
{
    m_appearanceTab = new QWidget;
    auto *layout = new QVBoxLayout(m_appearanceTab);

    // Theme selection
    auto *themeGroup = new QGroupBox("Theme");
    auto *themeLayout = new QVBoxLayout(themeGroup);

    m_lightThemeRadio = new QRadioButton("Light Theme");
    m_darkThemeRadio = new QRadioButton("Dark Theme");
    m_systemThemeRadio = new QRadioButton("System Theme (Auto)");

    themeLayout->addWidget(m_lightThemeRadio);
    themeLayout->addWidget(m_darkThemeRadio);
    themeLayout->addWidget(m_systemThemeRadio);

    layout->addWidget(themeGroup);
    layout->addStretch();
}

void PreferencesDialog::setupEditorTab()
{
    m_editorTab = new QWidget;
    auto *layout = new QVBoxLayout(m_editorTab);

    // Font settings
    auto *fontGroup = new QGroupBox("Font");
    auto *fontLayout = new QVBoxLayout(fontGroup);

    // Font family
    auto *familyLayout = new QHBoxLayout;
    familyLayout->addWidget(new QLabel("Font Family:"));
    m_fontFamilyCombo = new QFontComboBox;
    m_fontFamilyCombo->setFontFilters(QFontComboBox::MonospacedFonts);
    familyLayout->addWidget(m_fontFamilyCombo);
    fontLayout->addLayout(familyLayout);

    // Font size
    auto *sizeLayout = new QHBoxLayout;
    sizeLayout->addWidget(new QLabel("Font Size:"));
    m_fontSizeSpinBox = new QSpinBox;
    m_fontSizeSpinBox->setRange(8, 72);
    m_fontSizeSpinBox->setSuffix(" pt");
    sizeLayout->addWidget(m_fontSizeSpinBox);
    sizeLayout->addStretch();
    fontLayout->addLayout(sizeLayout);

    // Font sample
    m_fontSampleLabel = new QLabel("The quick brown fox jumps over the lazy dog\n1234567890");
    m_fontSampleLabel->setStyleSheet("border: 1px solid gray; padding: 8px; background: white; color: black;");
    m_fontSampleLabel->setAlignment(Qt::AlignCenter);
    fontLayout->addWidget(new QLabel("Preview:"));
    fontLayout->addWidget(m_fontSampleLabel);

    layout->addWidget(fontGroup);

    // Editor options
    auto *optionsGroup = new QGroupBox("Editor Options");
    auto *optionsLayout = new QVBoxLayout(optionsGroup);

    m_lineWrappingCheckBox = new QCheckBox("Enable line wrapping");
    m_showLineNumbersCheckBox = new QCheckBox("Show line numbers");

    optionsLayout->addWidget(m_lineWrappingCheckBox);
    optionsLayout->addWidget(m_showLineNumbersCheckBox);

    layout->addWidget(optionsGroup);
    layout->addStretch();

    // Connect font changes to update sample
    connect(m_fontFamilyCombo, &QFontComboBox::currentFontChanged, this, &PreferencesDialog::onFontSampleUpdate);
    connect(m_fontSizeSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &PreferencesDialog::onFontSampleUpdate);
}

void PreferencesDialog::loadCurrentSettings()
{
    Settings *settings = Settings::instance();

    // Load theme settings
    switch (settings->currentTheme()) {
    case Settings::LightTheme:
        m_lightThemeRadio->setChecked(true);
        break;
    case Settings::DarkTheme:
        m_darkThemeRadio->setChecked(true);
        break;
    case Settings::SystemTheme:
        m_systemThemeRadio->setChecked(true);
        break;
    }

    // Load font settings
    QFont editorFont = settings->editorFont();
    m_fontFamilyCombo->setCurrentFont(editorFont);
    m_fontSizeSpinBox->setValue(editorFont.pointSize());

    // Load editor options
    m_lineWrappingCheckBox->setChecked(settings->lineWrapping());
    m_showLineNumbersCheckBox->setChecked(settings->showLineNumbers());

    updateFontSample();
}

void PreferencesDialog::applySettings()
{
    Settings *settings = Settings::instance();

    // Apply theme
    if (m_lightThemeRadio->isChecked()) {
        settings->setTheme(Settings::LightTheme);
    } else if (m_darkThemeRadio->isChecked()) {
        settings->setTheme(Settings::DarkTheme);
    } else if (m_systemThemeRadio->isChecked()) {
        settings->setTheme(Settings::SystemTheme);
    }

    // Apply font
    QFont newFont = m_fontFamilyCombo->currentFont();
    newFont.setPointSize(m_fontSizeSpinBox->value());
    settings->setEditorFont(newFont);

    // Apply editor options
    settings->setLineWrapping(m_lineWrappingCheckBox->isChecked());
    settings->setShowLineNumbers(m_showLineNumbersCheckBox->isChecked());
}

void PreferencesDialog::resetToDefaults()
{
    // Reset to defaults
    m_lightThemeRadio->setChecked(true);
    m_fontFamilyCombo->setCurrentFont(QFont("Courier"));
    m_fontSizeSpinBox->setValue(12);
    m_lineWrappingCheckBox->setChecked(true);
    m_showLineNumbersCheckBox->setChecked(false);

    updateFontSample();
}

void PreferencesDialog::onFontSampleUpdate()
{
    updateFontSample();
}

void PreferencesDialog::updateFontSample()
{
    QFont sampleFont = m_fontFamilyCombo->currentFont();
    sampleFont.setPointSize(m_fontSizeSpinBox->value());
    m_fontSampleLabel->setFont(sampleFont);
}