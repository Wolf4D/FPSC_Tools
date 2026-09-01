#include "I18n.h"
#include <QMap>

I18n& I18n::instance()
{
    static I18n s_instance;
    return s_instance;
}

I18n::I18n(QObject *parent)
    : QObject(parent)
    , m_setting(Language::Auto)
    , m_isRussian(false)
{
    setLanguageSetting(Language::Auto);
}

I18n::Language I18n::languageSetting() const
{
    return m_setting;
}

void I18n::setLanguageSetting(Language lang)
{
    m_setting = lang;
    if (m_setting == Language::Auto) {
        QLocale::Language sysLang = QLocale::system().language();
        m_isRussian = (sysLang == QLocale::Russian || sysLang == QLocale::Ukrainian || sysLang == QLocale::Belarusian);
    } else if (m_setting == Language::Russian) {
        m_isRussian = true;
    } else {
        m_isRussian = false;
    }

    if (qApp) {
        qApp->removeTranslator(&m_translator);
        if (m_isRussian) {
            if (m_translator.load(":/translations/fpsc_tool_ru.qm")) {
                qApp->installTranslator(&m_translator);
            }
        }
    }

    emit languageChanged();
}

void I18n::setLanguageSettingCode(const QString &code)
{
    if (code.compare("ru", Qt::CaseInsensitive) == 0) {
        setLanguageSetting(Language::Russian);
    } else if (code.compare("en", Qt::CaseInsensitive) == 0) {
        setLanguageSetting(Language::English);
    } else {
        setLanguageSetting(Language::Auto);
    }
}

QString I18n::languageSettingCode() const
{
    switch (m_setting) {
    case Language::Russian: return "ru";
    case Language::English: return "en";
    case Language::Auto:
    default: return "auto";
    }
}

bool I18n::isRussian() const
{
    return m_isRussian;
}

QString I18n::t(const QString &key) const
{
    // English (default) vs Russian dictionary
    static const QMap<QString, QString> ruDict = {
        // Toolbar
        {"toolbar_title", "FPSC Tools (Перетащите для перемещения)"},
        {"profile_tooltip", "Активный профиль лайтмаппинга: %1\nlightmapping=%2\nlightmaptexsize=%3\nlightmapquality=%4\nlightao=%5\n(Нажмите для смены профиля)"},
        {"profile_btn", "💡 %1 ▾"},
        {"restart_btn", "⚡ Перезапуск"},
        {"restart_tooltip", "Завершить все процессы FPS Creator и запустить заново"},
        {"clean_btn", "🧹 Очистка ▾"},
        {"clean_tooltip", "Быстрая очистка временных файлов"},
        {"clean_bin_dbo", ".bin и .dbo"},
        {"clean_lightmaps", "Лайтмапы (testlevel)"},
        {"clean_all", "Всё"},
        {"clean_level_data", "Данные уровня"},
        {"clean_type_level_data", "Данные текущего уровня"},
        {"clean_engine_temp", "Временные файлы движка (%TEMP%/dbpdata*)"},
        {"clean_type_engine_temp", "Временные данные %TEMP%"},
        {"stash_btn", "💾 Stash ▾"},
        {"stash_tooltip", "Работа со снимками папки Files/levelbank/testlevel"},
        {"stash_save_quick", "💾 Быстрое сохранение (Quick Stash)"},
        {"stash_restore_quick", "🔄 Восстановить последний снимок"},
        {"stash_manager", "📁 Менеджер снимков..."},
        {"stash_folder", "📂 Открыть папку снимков"},
        {"pin_tooltip", "Закрепить поверх всех окон"},
        {"fold_tooltip", "Свернуть в плавающую мини-иконку"},
        {"settings_tooltip", "Настройки FPSC Tools..."},
        {"close_tooltip", "Закрыть приложение (Выход)"},
        {"status_running", "FPS Creator запущен"},
        {"status_stopped", "FPS Creator не запущен"},
        {"icon_restart_tooltip_running", "FPS Creator запущен (Клик — перезапустить, перетаскивание — переместить окно)"},
        {"icon_restart_tooltip_stopped", "FPS Creator не запущен (Клик — запустить, перетаскивание — переместить окно)"},
        {"app_bima_support", "(Сделано в поддержку проекта BIMA)"},

        // Profiles Display Names
        {"profile_disabled", "Отключено"},
        {"profile_fast", "Быстрый"},
        {"profile_normal", "Обычный"},
        {"profile_release", "Релиз"},
        {"profile_ultra", "Ультра"},

        // Context menu
        {"menu_expand", "🪟 Развернуть панель"},
        {"menu_profiles", "⚡ Профили лайтмаппинга"},
        {"menu_run_lightmapper", "⚡ Запустить LightMapper (BIMlightmapper)"},
        {"menu_restart", "🔄 Перезапустить FPS Creator"},
        {"menu_kill", "⏹ Закрыть FPS Creator"},
        {"menu_clean", "🧹 Очистка кэша"},
        {"menu_stash", "💾 Stash (testlevel)"},
        {"menu_fold", "🗕 Свернуть в мини-иконку"},
        {"menu_tray", "Свернуть в трей"},
        {"menu_toggle_toolbar", "🪟 Показать/Скрыть панель"},
        {"menu_settings", "⚙ Настройки..."},
        {"menu_exit", "❌ Выход"},

        // Notifications & Messages
        {"msg_profile_activated", "Активирован профиль «%1», FPS Creator перезапущен"},
        {"msg_restarted", "FPS Creator перезапущен"},
        {"msg_killed", "Завершено процессов: %1"},
        {"msg_stash_saved", "Снимок testlevel успешно сохранен!"},
        {"msg_stash_restored", "Снимок успешно восстановлен в testlevel!"},
        {"msg_minimized_tray", "FPSC Tools свернут в трей"},
        {"msg_folder_not_set", "Пожалуйста, укажите папку FPS Creator в настройках."},
        {"msg_lightmapper_success", "Расчёт освещения (LightMapper) успешно завершён!"},
        {"msg_lightmapper_cancelled", "Расчёт освещения отменён пользователем."},
        {"msg_lightmapper_failed", "Ошибка расчёта освещения (код завершения: %1)"},
        {"lightmapper_wait_title", "Пожалуйста, подождите..."},
        {"lightmapper_wait_msg", "Выполняется расчёт освещения уровня (LightMapper.exe)..."},
        {"err_lightmapper_not_found", "Исполняемый файл LightMapper.exe не найден в каталоге FPS Creator!"},
        {"stash_map_file", "Файл карты"},
        {"clean_summary_clean", "Очищено %1 файлов (%2) за %3 мс"},
        {"clean_summary_empty", "%1: лишних файлов не обнаружено (%2 мс)"},
        {"clean_type_bin_dbo", ".bin и .dbo кэш"},
        {"clean_type_lightmaps", "Лайтмапы"},
        {"clean_type_all", "Временные файлы"},
        {"unit_bytes", "Б"},
        {"unit_kb", "КБ"},
        {"unit_mb", "МБ"},
        {"unit_gb", "ГБ"},
        {"err_folder_not_set", "Каталог FPS Creator не указан"},
        {"err_testlevel_not_found", "Папка testlevel не найдена"},
        {"err_copy_testlevel", "Ошибка копирования файлов testlevel"},
        {"err_no_stashes_restore", "Нет сохраненных снимков для восстановления"},
        {"err_stash_not_found", "Данные снимка не найдены"},
        {"err_clean_testlevel", "Не удалось очистить текущее содержимое testlevel (возможно, файлы заблокированы)"},
        {"err_restore_testlevel", "Ошибка при восстановлении файлов в testlevel"},
        {"err_delete_stash", "Не удалось удалить каталог снимка"},
        {"stash_default_name", "Снимок %1"},

        // Settings Dialog
        {"settings_title", "Настройки FPSC Tools"},
        {"settings_dir_group", "Папка FPS Creator"},
        {"settings_dir_placeholder", "Укажите путь к корневой папке FPS Creator (содержащей Files и setup.ini)..."},
        {"settings_browse", "Обзор..."},
        {"settings_autodetect", "Автопоиск"},
        {"settings_profiles_group", "Параметры профилей лайтмаппинга (setup.ini)"},
        {"settings_reset_profiles", "Сбросить профили к стандартам"},
        {"settings_general_group", "Общие настройки"},
        {"settings_language", "Язык интерфейса:"},
        {"settings_auto_clean", "Автоматически очищать .bin и .dbo при смене профиля"},
        {"settings_always_on_top", "Закреплять плавающую панель поверх всех окон"},
        {"settings_show_notifications", "Показывать всплывающие уведомления в трее"},
        {"settings_start_minimized", "Запускать приложение свернутым в трей"},
        {"settings_save", "Сохранить"},
        {"settings_cancel", "Отмена"},
        {"settings_valid_path", "✔ Корректная папка FPS Creator найдена!"},
        {"settings_invalid_path", "⚠ Папка не содержит файлов FPS Creator (нет Files или setup.ini)"},
        {"settings_empty_path", "Путь не задан. Укажите папку с FPS Creator."},
        {"settings_autodetect_fail", "Не удалось автоматически обнаружить папку FPS Creator. Пожалуйста, укажите путь вручную через кнопку «Обзор»."},
        {"settings_reset_confirm", "Восстановить стандартные значения для всех профилей?"},

        // Stash Dialog
        {"stash_title", "Менеджер Stash (testlevel)"},
        {"stash_info_label", "Сохраненные снимки состояния папки <b>Files/levelbank/testlevel</b>:"},
        {"stash_col_name", "Название снимка"},
        {"stash_col_date", "Дата создания"},
        {"stash_col_files", "Файлов"},
        {"stash_col_size", "Размер"},
        {"stash_btn_create", "➕ Сохранить снимок..."},
        {"stash_btn_restore", "🔄 Восстановить в testlevel"},
        {"stash_btn_delete", "🗑 Удалить"},
        {"stash_btn_open", "📂 Открыть папку"},
        {"stash_btn_close", "Закрыть"},
        {"stash_empty_label", "Снимков пока нет. Нажмите «Сохранить снимок...», чтобы создать копию testlevel."},
        {"stash_total_count", "Всего сохраненных снимков: %1"},
        {"stash_create_title", "Создание снимка Stash"},
        {"stash_create_prompt", "Введите название снимка:"},
        {"stash_restore_confirm", "Внимание! Текущее содержимое папки <b>Files/levelbank/testlevel</b> будет полностью заменено выбранным снимком.\n\nПродолжить?"},
        {"stash_delete_confirm", "Вы уверены, что хотите удалить выбранный снимок?"},
        {"stash_delete_success", "Снимок удален."},
        {"stash_no_folder_yet", "Папка со снимками ещё не создана (нет сохраненных снимков)."}
    };

    static const QMap<QString, QString> enDict = {
        // Toolbar
        {"toolbar_title", "FPSC Tools (Drag to move)"},
        {"profile_tooltip", "Active Lightmapping Profile: %1\nlightmapping=%2\nlightmaptexsize=%3\nlightmapquality=%4\nlightao=%5\n(Click to switch profile)"},
        {"profile_btn", "💡 %1 ▾"},
        {"restart_btn", "⚡ Restart"},
        {"restart_tooltip", "Terminate all FPS Creator processes and relaunch cleanly"},
        {"clean_btn", "🧹 Clean ▾"},
        {"clean_tooltip", "Fast temporary files cleanup"},
        {"clean_bin_dbo", ".bin and .dbo"},
        {"clean_lightmaps", "Lightmaps (testlevel)"},
        {"clean_all", "All"},
        {"clean_level_data", "Level Build Data"},
        {"clean_type_level_data", "Level build data"},
        {"clean_engine_temp", "Engine Temp Cache (%TEMP%/dbpdata*)"},
        {"clean_type_engine_temp", "%TEMP% engine cache"},
        {"stash_btn", "💾 Stash ▾"},
        {"stash_tooltip", "Manage testlevel snapshots (Files/levelbank/testlevel)"},
        {"stash_save_quick", "💾 Quick Save (Quick Stash)"},
        {"stash_restore_quick", "🔄 Restore Latest Snapshot"},
        {"stash_manager", "📁 Stash Manager..."},
        {"stash_folder", "📂 Open Stashes Folder"},
        {"pin_tooltip", "Pin Always on Top"},
        {"fold_tooltip", "Fold to Floating Mini Icon"},
        {"settings_tooltip", "FPSC Tools Settings..."},
        {"close_tooltip", "Close Application (Exit)"},
        {"status_running", "FPS Creator is running"},
        {"status_stopped", "FPS Creator is not running"},
        {"icon_restart_tooltip_running", "FPS Creator is running (Click to restart, drag to move window)"},
        {"icon_restart_tooltip_stopped", "FPS Creator is not running (Click to launch, drag to move window)"},
        {"app_bima_support", "(Made in support of the BIMA project)"},

        // Profiles Display Names
        {"profile_disabled", "Disabled"},
        {"profile_fast", "Fast"},
        {"profile_normal", "Normal"},
        {"profile_release", "Release"},
        {"profile_ultra", "Ultra"},

        // Context menu
        {"menu_expand", "🪟 Expand Toolbar"},
        {"menu_profiles", "⚡ Lightmapping Profiles"},
        {"menu_run_lightmapper", "⚡ Run LightMapper (BIMlightmapper)"},
        {"menu_restart", "🔄 Restart FPS Creator"},
        {"menu_kill", "⏹ Close FPS Creator"},
        {"menu_clean", "🧹 Cache Cleanup"},
        {"menu_stash", "💾 Stash (testlevel)"},
        {"menu_fold", "🗕 Fold to Mini Icon"},
        {"menu_tray", "Minimize to Tray"},
        {"menu_toggle_toolbar", "🪟 Show/Hide Toolbar"},
        {"menu_settings", "⚙ Settings..."},
        {"menu_exit", "❌ Exit"},

        // Notifications & Messages
        {"msg_profile_activated", "Profile \"%1\" applied, FPS Creator restarted"},
        {"msg_restarted", "FPS Creator restarted"},
        {"msg_killed", "Processes terminated: %1"},
        {"msg_stash_saved", "testlevel snapshot saved successfully!"},
        {"msg_stash_restored", "Snapshot restored to testlevel!"},
        {"msg_minimized_tray", "FPSC Tools minimized to tray"},
        {"msg_folder_not_set", "Please configure the FPS Creator directory in Settings."},
        {"msg_lightmapper_success", "Lightmapping (LightMapper) completed successfully!"},
        {"msg_lightmapper_cancelled", "Lightmapping cancelled by user."},
        {"msg_lightmapper_failed", "Lightmapping failed (exit code: %1)"},
        {"lightmapper_wait_title", "Please wait..."},
        {"lightmapper_wait_msg", "Calculating level lightmaps (LightMapper.exe)..."},
        {"err_lightmapper_not_found", "LightMapper.exe executable not found in FPS Creator directory!"},
        {"stash_map_file", "Map file"},
        {"clean_summary_clean", "Cleaned %1 files (%2) in %3 ms"},
        {"clean_summary_empty", "%1: no extra files found (%2 ms)"},
        {"clean_type_bin_dbo", ".bin and .dbo cache"},
        {"clean_type_lightmaps", "Lightmaps"},
        {"clean_type_all", "Temporary files"},
        {"unit_bytes", "B"},
        {"unit_kb", "KB"},
        {"unit_mb", "MB"},
        {"unit_gb", "GB"},
        {"err_folder_not_set", "FPS Creator directory not specified"},
        {"err_testlevel_not_found", "testlevel folder not found"},
        {"err_copy_testlevel", "Error copying testlevel files"},
        {"err_no_stashes_restore", "No saved snapshots to restore"},
        {"err_stash_not_found", "Snapshot data not found"},
        {"err_clean_testlevel", "Failed to clean testlevel contents (files might be in use)"},
        {"err_restore_testlevel", "Error restoring files to testlevel"},
        {"err_delete_stash", "Failed to delete snapshot directory"},
        {"stash_default_name", "Snapshot %1"},

        // Settings Dialog
        {"settings_title", "FPSC Tools Settings"},
        {"settings_dir_group", "FPS Creator Installation Directory"},
        {"settings_dir_placeholder", "Specify path to FPS Creator root folder (containing Files and setup.ini)..."},
        {"settings_browse", "Browse..."},
        {"settings_autodetect", "Auto-detect"},
        {"settings_profiles_group", "Lightmapping Profile Parameters (setup.ini)"},
        {"settings_reset_profiles", "Reset Profiles to Defaults"},
        {"settings_general_group", "General Settings"},
        {"settings_language", "Language:"},
        {"settings_auto_clean", "Automatically clean .bin and .dbo on profile switch"},
        {"settings_always_on_top", "Keep floating toolbar Always on Top"},
        {"settings_show_notifications", "Show system notifications in tray"},
        {"settings_start_minimized", "Start application minimized to tray"},
        {"settings_save", "Save"},
        {"settings_cancel", "Cancel"},
        {"settings_valid_path", "✔ Valid FPS Creator directory found!"},
        {"settings_invalid_path", "⚠ Directory does not contain FPS Creator files (missing Files or setup.ini)"},
        {"settings_empty_path", "Path is empty. Please specify FPS Creator folder."},
        {"settings_autodetect_fail", "Could not automatically locate FPS Creator folder. Please select it manually via Browse."},
        {"settings_reset_confirm", "Restore default parameters for all profiles?"},

        // Stash Dialog
        {"stash_title", "Stash Manager (testlevel)"},
        {"stash_info_label", "Saved snapshots of <b>Files/levelbank/testlevel</b>:"},
        {"stash_col_name", "Snapshot Name"},
        {"stash_col_date", "Date Created"},
        {"stash_col_files", "Files"},
        {"stash_col_size", "Size"},
        {"stash_btn_create", "➕ Save Snapshot..."},
        {"stash_btn_restore", "🔄 Restore to testlevel"},
        {"stash_btn_delete", "🗑 Delete"},
        {"stash_btn_open", "📂 Open Folder"},
        {"stash_btn_close", "Close"},
        {"stash_empty_label", "No snapshots saved yet. Click \"Save Snapshot...\" to create a copy of testlevel."},
        {"stash_total_count", "Total saved snapshots: %1"},
        {"stash_create_title", "Create Stash Snapshot"},
        {"stash_create_prompt", "Enter snapshot name:"},
        {"stash_restore_confirm", "Warning! The current contents of <b>Files/levelbank/testlevel</b> will be completely overwritten by the selected snapshot.\n\nContinue?"},
        {"stash_delete_confirm", "Are you sure you want to delete the selected snapshot?"},
        {"stash_delete_success", "Snapshot deleted."},
        {"stash_no_folder_yet", "Stashes directory does not exist yet (no snapshots created)."}
    };

    if (m_isRussian) {
        QString translated = QCoreApplication::translate("FPSC", key.toUtf8().constData());
        if (!translated.isEmpty() && translated != key) {
            return translated;
        }
        return ruDict.value(key, enDict.value(key, key));
    } else {
        return enDict.value(key, key);
    }
}
