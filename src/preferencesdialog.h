#ifndef PREFERENCESDIALOG_H
#define PREFERENCESDIALOG_H

#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTabWidget>
#include <QGroupBox>
#include <QRadioButton>
#include <QCheckBox>
#include <QFontComboBox>
#include <QSpinBox>
#include <QLabel>
#include <QPushButton>
#include "settings.h"

class PreferencesDialog : public QDialog
{
    Q_OBJECT

public:
    explicit PreferencesDialog(QWidget *parent = nullptr);

private slots:
    void applySettings();
    void resetToDefaults();
    void onFontSampleUpdate();

private:
    void setupUI();
    void setupAppearanceTab();
    void setupEditorTab();
    void loadCurrentSettings();
    void updateFontSample();

    QTabWidget *m_tabWidget;

    // Appearance tab
    QWidget *m_appearanceTab;
    QRadioButton *m_lightThemeRadio;
    QRadioButton *m_darkThemeRadio;
    QRadioButton *m_systemThemeRadio;

    // Editor tab
    QWidget *m_editorTab;
    QFontComboBox *m_fontFamilyCombo;
    QSpinBox *m_fontSizeSpinBox;
    QLabel *m_fontSampleLabel;
    QCheckBox *m_lineWrappingCheckBox;
    QCheckBox *m_showLineNumbersCheckBox;

    // Dialog buttons
    QPushButton *m_okButton;
    QPushButton *m_cancelButton;
    QPushButton *m_applyButton;
    QPushButton *m_resetButton;
};

#endif // PREFERENCESDIALOG_H