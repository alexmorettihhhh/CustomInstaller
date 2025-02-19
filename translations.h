#pragma once
#include <string>
#include <map>

struct Translation {
    std::wstring windowTitle;
    std::wstring installDir;
    std::wstring browse;
    std::wstring license;
    std::wstring licenseText;
    std::wstring acceptLicense;
    std::wstring desktopShortcut;
    std::wstring startMenuShortcut;
    std::wstring autoRun;
    std::wstring install;
    std::wstring ready;
    std::wstring error;
    std::wstring selectDir;
};

const std::map<std::string, Translation> TRANSLATIONS = {
    {"en", {
        L"Custom Installer",
        L"Installation Directory:",
        L"Browse...",
        L"License Agreement",
        L"Please read the following license agreement carefully:",
        L"I accept the terms of the license agreement",
        L"Create desktop shortcut",
        L"Create Start menu shortcut",
        L"Run after installation",
        L"Install",
        L"Ready to install...",
        L"Error",
        L"Select installation directory"
    }},
    {"ru", {
        L"Установщик",
        L"Папка установки:",
        L"Обзор...",
        L"Лицензионное соглашение",
        L"Пожалуйста, внимательно прочтите лицензионное соглашение:",
        L"Я принимаю условия лицензионного соглашения",
        L"Создать ярлык на рабочем столе",
        L"Создать ярлык в меню Пуск",
        L"Запустить после установки",
        L"Установить",
        L"Готов к установке...",
        L"Ошибка",
        L"Выберите папку для установки"
    }},
    {"de", {
        L"Installationsprogramm",
        L"Installationsverzeichnis:",
        L"Durchsuchen...",
        L"Lizenzvereinbarung",
        L"Bitte lesen Sie die folgende Lizenzvereinbarung sorgfältig durch:",
        L"Ich akzeptiere die Bedingungen der Lizenzvereinbarung",
        L"Desktop-Verknüpfung erstellen",
        L"Startmenü-Verknüpfung erstellen",
        L"Nach der Installation ausführen",
        L"Installieren",
        L"Bereit zur Installation...",
        L"Fehler",
        L"Installationsverzeichnis auswählen"
    }}
}; 