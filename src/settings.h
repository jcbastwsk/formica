#ifndef SETTINGS_H
#define SETTINGS_H

#include <QObject>
#include <QSettings>
#include <QFont>
#include <QColor>

class Settings : public QObject
{
    Q_OBJECT

public:
    enum Theme {
        LightTheme,
        DarkTheme,
        SystemTheme
    };

    static Settings* instance();

    // Theme settings
    Theme currentTheme() const;
    void setTheme(Theme theme);

    // Font settings
    QFont editorFont() const;
    void setEditorFont(const QFont &font);

    int fontSize() const;
    void setFontSize(int size);

    QString fontFamily() const;
    void setFontFamily(const QString &family);

    // Editor settings
    bool lineWrapping() const;
    void setLineWrapping(bool enabled);

    bool showLineNumbers() const;
    void setShowLineNumbers(bool enabled);

    // Colors for current theme
    QColor backgroundColor() const;
    QColor textColor() const;
    QColor selectionColor() const;
    QColor highlightColor() const;

    // Apply theme to application
    void applyTheme();

signals:
    void themeChanged(Theme newTheme);
    void fontChanged(const QFont &newFont);

private:
    explicit Settings(QObject *parent = nullptr);
    void loadSettings();
    void saveSettings();
    QString themeToString(Theme theme) const;
    Theme stringToTheme(const QString &str) const;

    QSettings *m_settings;
    Theme m_currentTheme;
    QFont m_editorFont;
    bool m_lineWrapping;
    bool m_showLineNumbers;

    static Settings *s_instance;
};

#endif // SETTINGS_H