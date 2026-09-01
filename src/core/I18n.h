#ifndef I18N_H
#define I18N_H

#include <QString>
#include <QObject>
#include <QLocale>
#include <QTranslator>
#include <QCoreApplication>

class I18n : public QObject
{
    Q_OBJECT

public:
    enum class Language {
        Auto,
        English,
        Russian
    };

    static I18n& instance();

    Language languageSetting() const;
    void setLanguageSetting(Language lang);
    void setLanguageSettingCode(const QString &code);
    QString languageSettingCode() const;

    bool isRussian() const;

    // Helper translation accessor
    QString t(const QString &key) const;

signals:
    void languageChanged();

private:
    explicit I18n(QObject *parent = nullptr);
    ~I18n() override = default;
    I18n(const I18n&) = delete;
    I18n& operator=(const I18n&) = delete;

    Language m_setting;
    bool m_isRussian;
    QTranslator m_translator;
};

// Global shorthand for localization
inline QString trText(const QString &key) {
    return I18n::instance().t(key);
}

#endif // I18N_H
