#include "settings.h"
#include <QApplication>
#include <QPalette>
#include <QStyleFactory>
#include <QStyle>

Settings* Settings::s_instance = nullptr;

Settings::Settings(QObject *parent)
    : QObject(parent)
    , m_settings(new QSettings("Formica", "Formica", this))
    , m_currentTheme(LightTheme)
    , m_lineWrapping(true)
    , m_showLineNumbers(false)
{
    // Set default font
    m_editorFont = QFont("Courier", 12);
    m_editorFont.setStyleHint(QFont::Monospace);

    loadSettings();
}

Settings* Settings::instance()
{
    if (!s_instance) {
        s_instance = new Settings();
    }
    return s_instance;
}

void Settings::loadSettings()
{
    // Load theme
    QString themeStr = m_settings->value("theme", "light").toString();
    m_currentTheme = stringToTheme(themeStr);

    // Load font
    QString fontFamily = m_settings->value("font/family", "Courier").toString();
    int fontSize = m_settings->value("font/size", 12).toInt();
    m_editorFont = QFont(fontFamily, fontSize);
    m_editorFont.setStyleHint(QFont::Monospace);

    // Load editor settings
    m_lineWrapping = m_settings->value("editor/lineWrapping", true).toBool();
    m_showLineNumbers = m_settings->value("editor/showLineNumbers", false).toBool();
}

void Settings::saveSettings()
{
    m_settings->setValue("theme", themeToString(m_currentTheme));
    m_settings->setValue("font/family", m_editorFont.family());
    m_settings->setValue("font/size", m_editorFont.pointSize());
    m_settings->setValue("editor/lineWrapping", m_lineWrapping);
    m_settings->setValue("editor/showLineNumbers", m_showLineNumbers);
    m_settings->sync();
}

Settings::Theme Settings::currentTheme() const
{
    return m_currentTheme;
}

void Settings::setTheme(Theme theme)
{
    if (m_currentTheme != theme) {
        m_currentTheme = theme;
        saveSettings();
        applyTheme();
        emit themeChanged(theme);
    }
}

QFont Settings::editorFont() const
{
    return m_editorFont;
}

void Settings::setEditorFont(const QFont &font)
{
    if (m_editorFont != font) {
        m_editorFont = font;
        saveSettings();
        emit fontChanged(font);
    }
}

int Settings::fontSize() const
{
    return m_editorFont.pointSize();
}

void Settings::setFontSize(int size)
{
    if (m_editorFont.pointSize() != size) {
        m_editorFont.setPointSize(size);
        saveSettings();
        emit fontChanged(m_editorFont);
    }
}

QString Settings::fontFamily() const
{
    return m_editorFont.family();
}

void Settings::setFontFamily(const QString &family)
{
    if (m_editorFont.family() != family) {
        m_editorFont.setFamily(family);
        saveSettings();
        emit fontChanged(m_editorFont);
    }
}

bool Settings::lineWrapping() const
{
    return m_lineWrapping;
}

void Settings::setLineWrapping(bool enabled)
{
    if (m_lineWrapping != enabled) {
        m_lineWrapping = enabled;
        saveSettings();
    }
}

bool Settings::showLineNumbers() const
{
    return m_showLineNumbers;
}

void Settings::setShowLineNumbers(bool enabled)
{
    if (m_showLineNumbers != enabled) {
        m_showLineNumbers = enabled;
        saveSettings();
    }
}

QColor Settings::backgroundColor() const
{
    switch (m_currentTheme) {
    case DarkTheme:
        return QColor(35, 38, 41);    // Dark gray
    case LightTheme:
    default:
        return QColor(255, 255, 255); // White
    }
}

QColor Settings::textColor() const
{
    switch (m_currentTheme) {
    case DarkTheme:
        return QColor(220, 220, 220); // Light gray
    case LightTheme:
    default:
        return QColor(0, 0, 0);       // Black
    }
}

QColor Settings::selectionColor() const
{
    switch (m_currentTheme) {
    case DarkTheme:
        return QColor(75, 110, 175);  // Dark blue
    case LightTheme:
    default:
        return QColor(173, 214, 255); // Light blue
    }
}

QColor Settings::highlightColor() const
{
    switch (m_currentTheme) {
    case DarkTheme:
        return QColor(128, 128, 0);   // Dark yellow
    case LightTheme:
    default:
        return QColor(255, 255, 0);   // Yellow
    }
}

void Settings::applyTheme()
{
    QPalette palette;

    switch (m_currentTheme) {
    case DarkTheme:
        // Dark theme colors
        palette.setColor(QPalette::Window, QColor(53, 53, 53));
        palette.setColor(QPalette::WindowText, QColor(255, 255, 255));
        palette.setColor(QPalette::Base, QColor(25, 25, 25));
        palette.setColor(QPalette::AlternateBase, QColor(53, 53, 53));
        palette.setColor(QPalette::ToolTipBase, QColor(0, 0, 0));
        palette.setColor(QPalette::ToolTipText, QColor(255, 255, 255));
        palette.setColor(QPalette::Text, QColor(255, 255, 255));
        palette.setColor(QPalette::Button, QColor(53, 53, 53));
        palette.setColor(QPalette::ButtonText, QColor(255, 255, 255));
        palette.setColor(QPalette::BrightText, QColor(255, 0, 0));
        palette.setColor(QPalette::Link, QColor(42, 130, 218));
        palette.setColor(QPalette::Highlight, QColor(42, 130, 218));
        palette.setColor(QPalette::HighlightedText, QColor(0, 0, 0));
        break;

    case LightTheme:
    default:
        // Use default light palette
        palette = QApplication::style()->standardPalette();
        break;
    }

    QApplication::setPalette(palette);
}

QString Settings::themeToString(Theme theme) const
{
    switch (theme) {
    case DarkTheme:
        return "dark";
    case SystemTheme:
        return "system";
    case LightTheme:
    default:
        return "light";
    }
}

Settings::Theme Settings::stringToTheme(const QString &str) const
{
    if (str == "dark") {
        return DarkTheme;
    } else if (str == "system") {
        return SystemTheme;
    } else {
        return LightTheme;
    }
}