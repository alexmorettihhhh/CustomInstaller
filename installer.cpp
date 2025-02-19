#ifndef UNICODE
#define UNICODE
#endif
#ifndef _UNICODE
#define _UNICODE
#endif
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#ifndef STRICT
#define STRICT
#endif

#include <SDKDDKVer.h>

// Windows headers
#include <windows.h>
#include <shellapi.h>  // For ShellExecuteW, HDROP, DragQueryFileW
#include <commctrl.h>
#include <shlobj.h>
#include <commdlg.h>
#include <uxtheme.h>
#include <dwmapi.h>
#include <versionhelpers.h>

// C++ Standard Library headers
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <filesystem>
#include <cstdint>
#include <iomanip>
#include <sstream>
#include <memory>
#include <climits>
#include "translations.h"

#pragma comment(lib, "comctl32.lib")
#pragma comment(lib, "shell32.lib")
#pragma comment(lib, "ole32.lib")
#pragma comment(lib, "advapi32.lib")
#pragma comment(lib, "comdlg32.lib")
#pragma comment(lib, "uxtheme.lib")
#pragma comment(lib, "dwmapi.lib")
#pragma comment(linker,"\"/manifestdependency:type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

// Предварительные объявления функций
static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
static bool CheckSystemCompatibility();
static void UpdateUILanguage();
static void UpdateFilesList();
static void RemoveSelectedFile();
static void CreateInstallerWindow();
static void AddFilesFromFolder(const std::wstring& folderPath);
static HFONT CreateModernFont();
static void ModernizeButton(HWND hButton);
static int CALLBACK CompareListItems(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);

// Объявление wWinMain с правильными аннотациями
_Use_decl_annotations_
int WINAPI wWinMain(
    _In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR lpCmdLine,
    _In_ int nCmdShow);

namespace fs = std::filesystem;

// Определяем структуру FileInfo до её использования
struct FileInfo {
    std::string path;
    size_t size = 0;  // Инициализируем по умолчанию
    std::wstring displayName;

    FileInfo() : path(), size(0), displayName() {}  // Конструктор по умолчанию
    
    // Конструктор с параметрами
    FileInfo(const std::string& p, size_t s, const std::wstring& d) 
        : path(p), size(s), displayName(d) {}
};

// Глобальные переменные для GUI
static HWND hMainWindow = nullptr;
static HWND hProgress = nullptr;
static HWND hStatus = nullptr;
static HWND hBrowseButton = nullptr;
static HWND hInstallButton = nullptr;
static HWND hPathEdit = nullptr;
static HWND hLicenseCheck = nullptr;
static HWND hDesktopShortcutCheck = nullptr;
static HWND hStartMenuShortcutCheck = nullptr;
static HWND hAutoRunCheck = nullptr;
static HWND hLanguageCombo = nullptr;
static std::string installPath;
static std::string currentLanguage = "en";  // Язык по умолчанию
static const Translation* currentTranslation = &TRANSLATIONS.at("en");

// Добавляем глобальные переменные для списка файлов
static std::vector<FileInfo> selectedFiles;
static HWND hFilesList = nullptr;
static HWND hAddFileButton = nullptr;
static HWND hTooltip = nullptr;  // Добавляем переменную для подсказок

// Добавляем глобальные переменные для новых функций
static HWND hVersionEdit = nullptr;
static HWND hIconPathEdit = nullptr;
static HWND hIconBrowseButton = nullptr;
static HWND hRemoveFileButton = nullptr;
static std::wstring iconPath;

// Добавляем глобальные переменные для современного дизайна
static HBRUSH hBackgroundBrush = NULL;
static HFONT hModernFont = NULL;
static COLORREF accentColor = RGB(0, 120, 215);  // Windows 10 accent color
static HWND hAddFolderButton = nullptr;

// Обновляем текст лицензионного соглашения
const char* LICENSE_TEXT = 
"END USER LICENSE AGREEMENT\r\n"
"\r\n"
"IMPORTANT: PLEASE READ THIS LICENSE CAREFULLY BEFORE USING THIS SOFTWARE.\r\n"
"\r\n"
"1. LICENSE GRANT\r\n"
"This software is licensed, not sold. Under this license, you are granted the non-exclusive right to:\r\n"
"a) Install and use this software on your computer\r\n"
"b) Make one copy of the software for backup purposes\r\n"
"c) Use the software for personal or commercial purposes\r\n"
"\r\n"
"2. RESTRICTIONS\r\n"
"You may NOT:\r\n"
"a) Modify, reverse engineer, decompile, or disassemble the software\r\n"
"b) Rent, lease, or distribute the software to third parties\r\n"
"c) Remove any proprietary notices or labels on the software\r\n"
"\r\n"
"3. WARRANTY DISCLAIMER\r\n"
"This software is provided 'AS IS' without any warranty of any kind, either express or implied, including, \r\n"
"but not limited to, the implied warranties of merchantability and fitness for a particular purpose. \r\n"
"The entire risk as to the quality and performance of the software is with you.\r\n"
"\r\n"
"4. LIMITATION OF LIABILITY\r\n"
"In no event shall the author be liable for any damages whatsoever (including, without limitation, \r\n"
"damages for loss of business profits, business interruption, loss of business information, or any other \r\n"
"pecuniary loss) arising out of the use of or inability to use this software.\r\n"
"\r\n"
"5. UPDATES AND SUPPORT\r\n"
"The author may provide updates or support for the software at their discretion. Any such updates or \r\n"
"support shall be subject to these same terms.\r\n"
"\r\n"
"6. TERMINATION\r\n"
"This license is effective until terminated. Your rights under this license will terminate automatically \r\n"
"without notice if you fail to comply with any terms of this agreement.\r\n"
"\r\n"
"7. GOVERNING LAW\r\n"
"This agreement shall be governed by the laws of your country of residence.\r\n"
"\r\n"
"By installing or using this software, you acknowledge that you have read this agreement, understand it, \r\n"
"and agree to be bound by its terms and conditions.\r\n"
"\r\n"
"Do you accept these terms and conditions?";

// Функции преобразования строк
static std::string WideToAnsi(const std::wstring& wstr) {
    if (wstr.empty()) return std::string();
    int size_needed = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, NULL, 0, NULL, NULL);
    std::string strTo(size_needed - 1, 0);
    WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, &strTo[0], size_needed, NULL, NULL);
    return strTo;
}

static std::wstring AnsiToWide(const std::string& str) {
    if (str.empty()) return std::wstring();
    int size_needed = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, NULL, 0);
    std::wstring wstrTo(size_needed - 1, 0);
    MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, &wstrTo[0], size_needed);
    return wstrTo;
}

class Installer {
private:
    struct FileData {
        std::string filename;
        std::vector<char> data;
        uint32_t checksum = 0;  // Инициализация по умолчанию
    };
    std::vector<FileData> files;
    std::string installPath;
    size_t totalSize = 0;
    size_t currentProgress = 0;
    bool createDesktopShortcut = true;
    bool createStartMenuShortcut = true;
    bool autoRunAfterInstall = true;
    std::string version = "1.0.0";
    std::string iconPath;

    // Вычисление CRC32 для проверки целостности файлов
    uint32_t calculateCRC32(const std::vector<char>& data) {
        uint32_t crc = 0xFFFFFFFF;
        for (char byte : data) {
            crc ^= static_cast<uint8_t>(byte);
            for (int i = 0; i < 8; i++) {
                crc = (crc >> 1) ^ (0xEDB88320 & (-(static_cast<int32_t>(crc & 1))));
            }
        }
        return ~crc;
    }

    void createShortcut(const std::string& shortcutPath, const std::string& targetPath) {
        HRESULT hres;
        IShellLinkW* psl;
        
        hres = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER,
                              IID_IShellLink, (LPVOID*)&psl);
        if (SUCCEEDED(hres)) {
            IPersistFile* ppf;
            
            std::wstring wideTargetPath = AnsiToWide(targetPath);
            std::wstring wideWorkingDir = AnsiToWide(fs::path(targetPath).parent_path().string());
            
            psl->SetPath(wideTargetPath.c_str());
            psl->SetWorkingDirectory(wideWorkingDir.c_str());
            
            hres = psl->QueryInterface(IID_IPersistFile, (LPVOID*)&ppf);
            if (SUCCEEDED(hres)) {
                std::wstring wideShortcutPath = AnsiToWide(shortcutPath);
                ppf->Save(wideShortcutPath.c_str(), TRUE);
                ppf->Release();
            }
            psl->Release();
        }
    }

public:
    void setShortcutOptions(bool desktop, bool startMenu, bool autoRun) {
        createDesktopShortcut = desktop;
        createStartMenuShortcut = startMenu;
        autoRunAfterInstall = autoRun;
    }

    void setProgress(size_t current, size_t total) {
        if (hProgress != nullptr) {
            SendMessage(hProgress, PBM_SETRANGE32, 0, static_cast<LPARAM>(total));
            SendMessage(hProgress, PBM_SETPOS, static_cast<WPARAM>(current), 0);
        }
    }

    void setStatus(const std::string& status) {
        if (hStatus != nullptr) {
            std::wstring wideStatus = AnsiToWide(status);
            SetWindowTextW(hStatus, wideStatus.c_str());
        }
    }

    void addFile(const std::string& filepath) {
        std::ifstream file(filepath, std::ios::binary);
        if (!file) {
            std::wstring wideError = L"Failed to open file: " + AnsiToWide(filepath);
            MessageBoxW(hMainWindow, wideError.c_str(), L"Error", MB_ICONERROR);
            return;
        }

        FileData fileData;
        fileData.filename = fs::path(filepath).filename().string();
        
        file.seekg(0, std::ios::end);
        size_t size = file.tellg();
        file.seekg(0, std::ios::beg);
        
        fileData.data.resize(size);
        file.read(fileData.data.data(), size);
        
        // Вычисляем контрольную сумму
        fileData.checksum = calculateCRC32(fileData.data);
        
        files.push_back(fileData);
        totalSize += size;
        
        setStatus("Added file: " + fileData.filename);
    }

    void setInstallPath(const std::string& path) {
        installPath = path;
        if (!fs::exists(installPath)) {
            fs::create_directories(installPath);
        }
    }

    void setVersion(const std::string& ver) {
        version = ver;
    }

    void setIcon(const std::string& path) {
        iconPath = path;
    }

    void createInstaller(const std::string& outputPath) {
        std::ofstream installer(outputPath, std::ios::binary);
        if (!installer) {
            MessageBoxW(hMainWindow, L"Failed to create installer file", L"Error", MB_ICONERROR);
            return;
        }

        // Записываем версию
        size_t versionLength = version.length();
        installer.write(reinterpret_cast<const char*>(&versionLength), sizeof(versionLength));
        installer.write(version.c_str(), versionLength);

        // Записываем путь к иконке
        size_t iconPathLength = iconPath.length();
        installer.write(reinterpret_cast<const char*>(&iconPathLength), sizeof(iconPathLength));
        if (!iconPath.empty()) {
            installer.write(iconPath.c_str(), iconPathLength);
        }

        size_t numFiles = files.size();
        installer.write(reinterpret_cast<const char*>(&numFiles), sizeof(numFiles));

        currentProgress = 0;
        for (const auto& file : files) {
            // Записываем имя файла
            size_t nameLength = file.filename.length();
            installer.write(reinterpret_cast<const char*>(&nameLength), sizeof(nameLength));
            installer.write(file.filename.c_str(), nameLength);
            
            // Записываем контрольную сумму
            installer.write(reinterpret_cast<const char*>(&file.checksum), sizeof(file.checksum));
            
            // Записываем данные файла
            size_t dataSize = file.data.size();
            installer.write(reinterpret_cast<const char*>(&dataSize), sizeof(dataSize));
            installer.write(file.data.data(), dataSize);

            currentProgress += dataSize;
            setProgress(currentProgress, totalSize);
            setStatus("Adding: " + file.filename);
        }

        setStatus("Installer created successfully!");
    }

    bool readFromInstaller(const std::string& installerPath) {
        std::ifstream installer(installerPath, std::ios::binary);
        if (!installer) {
            MessageBoxW(hMainWindow, L"Failed to open installer file", L"Error", MB_ICONERROR);
            return false;
        }

        size_t numFiles;
        installer.read(reinterpret_cast<char*>(&numFiles), sizeof(numFiles));

        totalSize = 0;
        files.clear();

        for (size_t i = 0; i < numFiles; i++) {
            FileData fileData;
            
            // Читаем имя файла
            size_t nameLength;
            installer.read(reinterpret_cast<char*>(&nameLength), sizeof(nameLength));
            std::vector<char> nameBuffer(nameLength);
            installer.read(nameBuffer.data(), nameLength);
            fileData.filename = std::string(nameBuffer.data(), nameLength);
            
            // Читаем контрольную сумму
            installer.read(reinterpret_cast<char*>(&fileData.checksum), sizeof(fileData.checksum));
            
            // Читаем данные файла
            size_t dataSize;
            installer.read(reinterpret_cast<char*>(&dataSize), sizeof(dataSize));
            fileData.data.resize(dataSize);
            installer.read(fileData.data.data(), dataSize);
            
            // Проверяем целостность
            uint32_t calculatedChecksum = calculateCRC32(fileData.data);
            if (calculatedChecksum != fileData.checksum) {
                std::wstringstream wss;
                wss << L"File integrity check failed for: " << AnsiToWide(fileData.filename) << L"\n"
                    << L"Expected: " << std::hex << fileData.checksum << L"\n"
                    << L"Got: " << calculatedChecksum;
                MessageBoxW(hMainWindow, wss.str().c_str(), L"Error", MB_ICONERROR);
                return false;
            }
            
            files.push_back(fileData);
            totalSize += dataSize;
        }

        return true;
    }

    void install() {
        if (installPath.empty()) {
            MessageBoxW(hMainWindow, currentTranslation->selectDir.c_str(), currentTranslation->error.c_str(), MB_ICONERROR);
            return;
        }

        currentProgress = 0;
        for (const auto& file : files) {
            std::string fullPath = installPath + "\\" + file.filename;
            std::ofstream outFile(fullPath, std::ios::binary);
            
            if (!outFile) {
                std::wstring errorMsg = L"Failed to create file: " + AnsiToWide(fullPath);
                MessageBoxW(hMainWindow, errorMsg.c_str(), currentTranslation->error.c_str(), MB_ICONERROR);
                continue;
            }

            outFile.write(file.data.data(), file.data.size());
            currentProgress += file.data.size();
            
            setProgress(currentProgress, totalSize);
            setStatus("Installing: " + file.filename);
        }

        // Создаем ярлыки
        std::string mainExePath = installPath + "\\MyApp.exe";
        
        if (createDesktopShortcut) {
            PWSTR desktopPath;
            if (SUCCEEDED(SHGetKnownFolderPath(FOLDERID_Desktop, 0, NULL, &desktopPath))) {
                std::string shortcutPath = fs::path(desktopPath).string() + "\\MyApp.lnk";
                createShortcut(shortcutPath, mainExePath);
                CoTaskMemFree(desktopPath);
            }
        }

        if (createStartMenuShortcut) {
            PWSTR startMenuPath;
            if (SUCCEEDED(SHGetKnownFolderPath(FOLDERID_Programs, 0, NULL, &startMenuPath))) {
                std::string shortcutPath = fs::path(startMenuPath).string() + "\\MyApp.lnk";
                createShortcut(shortcutPath, mainExePath);
                CoTaskMemFree(startMenuPath);
            }
        }

        setStatus("Installation completed successfully!");
        MessageBoxW(hMainWindow, L"Installation completed successfully!", L"Success", MB_ICONINFORMATION);

        // Автозапуск после установки
        if (autoRunAfterInstall) {
            std::wstring wideExePath = AnsiToWide(mainExePath);
            std::wstring wideInstallPath = AnsiToWide(installPath);
            ShellExecuteW(NULL, L"open", wideExePath.c_str(), NULL, wideInstallPath.c_str(), SW_SHOW);
        }
    }
};

static void UpdateUILanguage() {
    const auto& trans = TRANSLATIONS.at(currentLanguage);
    
    SetWindowTextW(hMainWindow, trans.windowTitle.c_str());
    
    // Обновляем тексты всех элементов управления
    HWND hInstallDirLabel = GetDlgItem(hMainWindow, 100);
    SetWindowTextW(hInstallDirLabel, trans.installDir.c_str());
    
    SetWindowTextW(hBrowseButton, trans.browse.c_str());
    
    HWND hLicenseLabel = GetDlgItem(hMainWindow, 101);
    SetWindowTextW(hLicenseLabel, trans.license.c_str());
    
    HWND hLicenseText = GetDlgItem(hMainWindow, 102);
    SetWindowTextW(hLicenseText, trans.licenseText.c_str());
    
    SetWindowTextW(hLicenseCheck, trans.acceptLicense.c_str());
    SetWindowTextW(hDesktopShortcutCheck, trans.desktopShortcut.c_str());
    SetWindowTextW(hStartMenuShortcutCheck, trans.startMenuShortcut.c_str());
    SetWindowTextW(hAutoRunCheck, trans.autoRun.c_str());
    SetWindowTextW(hInstallButton, trans.install.c_str());
    SetWindowTextW(hStatus, trans.ready.c_str());
}

static void UpdateFilesList() {
    ListView_DeleteAllItems(hFilesList);
    int index = 0;
    for (const auto& file : selectedFiles) {
        LVITEMW lvi = { 0 };
        lvi.mask = LVIF_TEXT;
        lvi.iItem = index;
        
        // Имя файла
        lvi.iSubItem = 0;
        std::wstring fileName = file.displayName;
        std::vector<wchar_t> fileNameBuffer(fileName.begin(), fileName.end());
        fileNameBuffer.push_back(0); // Добавляем нулевой символ
        lvi.pszText = fileNameBuffer.data();
        ListView_InsertItem(hFilesList, &lvi);
        
        // Размер файла
        std::wstringstream ss;
        ss << (file.size / 1024) << L" KB";
        std::wstring sizeStr = ss.str();
        std::vector<wchar_t> sizeBuffer(sizeStr.begin(), sizeStr.end());
        sizeBuffer.push_back(0); // Добавляем нулевой символ
        lvi.iSubItem = 1;
        lvi.pszText = sizeBuffer.data();
        ListView_SetItem(hFilesList, &lvi);
        
        index++;
    }
}

static void RemoveSelectedFile() {
    int index = ListView_GetNextItem(hFilesList, -1, LVNI_SELECTED);
    if (index != -1) {
        selectedFiles.erase(selectedFiles.begin() + index);
        UpdateFilesList();
    }
}

// Добавляем структуру для хранения данных окна
struct WindowData {
    std::unique_ptr<WNDCLASSEXW> wc;
    std::vector<WCHAR> className;
    std::vector<WCHAR> windowTitle;
    std::vector<WCHAR> staticTexts;
    std::vector<WCHAR> buttonTexts;
};

// Функция для создания современного шрифта
static HFONT CreateModernFont() {
    NONCLIENTMETRICSW ncm = { sizeof(ncm) };
    SystemParametersInfoW(SPI_GETNONCLIENTMETRICS, sizeof(ncm), &ncm, 0);
    ncm.lfMessageFont.lfHeight = -14;  // Увеличиваем размер шрифта
    wcscpy_s(ncm.lfMessageFont.lfFaceName, L"Segoe UI");
    return CreateFontIndirectW(&ncm.lfMessageFont);
}

// Функция для применения современного стиля к кнопке
static void ModernizeButton(HWND hButton) {
    SetWindowTheme(hButton, L"Explorer", NULL);
    SendMessageW(hButton, WM_SETFONT, (WPARAM)hModernFont, TRUE);
}

// Функция для рекурсивного добавления файлов из папки
static void AddFilesFromFolder(const std::wstring& folderPath) {
    for (const auto& entry : fs::recursive_directory_iterator(folderPath)) {
        if (fs::is_regular_file(entry)) {
            WIN32_FILE_ATTRIBUTE_DATA fad;
            std::wstring path = entry.path().wstring();
            
            if (GetFileAttributesExW(path.c_str(), GetFileExInfoStandard, &fad)) {
                LARGE_INTEGER size;
                size.HighPart = fad.nFileSizeHigh;
                size.LowPart = fad.nFileSizeLow;
                
                // Получаем относительный путь от корневой папки
                std::wstring relativePath = path.substr(folderPath.length() + 1);
                
                selectedFiles.emplace_back(
                    WideToAnsi(path),
                    static_cast<size_t>(size.QuadPart),
                    relativePath
                );
            }
        }
    }
    UpdateFilesList();
}

// Обновляем CreateInstallerWindow
static void CreateInstallerWindow() {
    auto windowData = std::make_unique<WindowData>();
    windowData->wc = std::make_unique<WNDCLASSEXW>();
    
    INITCOMMONCONTROLSEX icex;
    icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
    icex.dwICC = ICC_PROGRESS_CLASS | ICC_STANDARD_CLASSES | ICC_LISTVIEW_CLASSES;
    InitCommonControlsEx(&icex);

    auto& wc = *windowData->wc;
    wc.cbSize = sizeof(WNDCLASSEXW);
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = GetModuleHandle(NULL);
    wc.lpszClassName = L"InstallerWindow";
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    RegisterClassExW(&wc);

    hMainWindow = CreateWindowExW(
        0,
        L"InstallerWindow",
        currentTranslation->windowTitle.c_str(),
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
        CW_USEDEFAULT, CW_USEDEFAULT,
        800, 600,
        NULL,
        NULL,
        GetModuleHandle(NULL),
        NULL
    );

    hLanguageCombo = CreateWindowExW(
        0,
        L"COMBOBOX",
        L"",
        WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | CBS_HASSTRINGS,
        20, 20,
        150, 200,
        hMainWindow,
        (HMENU)7,
        GetModuleHandle(NULL),
        NULL
    );

    SendMessageW(hLanguageCombo, CB_ADDSTRING, 0, (LPARAM)L"English");
    SendMessageW(hLanguageCombo, CB_ADDSTRING, 0, (LPARAM)L"Русский");
    SendMessageW(hLanguageCombo, CB_ADDSTRING, 0, (LPARAM)L"Deutsch");
    SendMessageW(hLanguageCombo, CB_SETCURSEL, 0, 0);

    hFilesList = CreateWindowExW(
        WS_EX_CLIENTEDGE | WS_EX_ACCEPTFILES,  // Добавляем поддержку drag & drop
        WC_LISTVIEW,
        L"",
        WS_CHILD | WS_VISIBLE | WS_VSCROLL | LVS_REPORT | LVS_SINGLESEL,
        20, 60,
        600, 200,
        hMainWindow,
        (HMENU)8,
        GetModuleHandle(NULL),
        NULL
    );

    // Включаем расширенные стили для ListView
    ListView_SetExtendedListViewStyle(hFilesList, 
        LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_INFOTIP);

    // Создаем подсказки
    hTooltip = CreateWindowExW(
        0,
        TOOLTIPS_CLASS,
        NULL,
        WS_POPUP | TTS_ALWAYSTIP | TTS_BALLOON,
        CW_USEDEFAULT, CW_USEDEFAULT,
        CW_USEDEFAULT, CW_USEDEFAULT,
        hMainWindow,
        NULL,
        GetModuleHandle(NULL),
        NULL
    );

    LVCOLUMNW lvc = { 0 };
    lvc.mask = LVCF_TEXT | LVCF_WIDTH;
    lvc.cx = 400;
    lvc.pszText = L"File Name";
    ListView_InsertColumn(hFilesList, 0, &lvc);
    lvc.cx = 100;
    lvc.pszText = L"Size";
    ListView_InsertColumn(hFilesList, 1, &lvc);

    hAddFileButton = CreateWindowW(
        L"BUTTON",
        L"Add Files",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        640, 60,
        120, 30,
        hMainWindow,
        (HMENU)9,
        GetModuleHandle(NULL),
        NULL
    );

    hRemoveFileButton = CreateWindowW(
        L"BUTTON",
        L"Remove File",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        640, 100,
        120, 30,
        hMainWindow,
        (HMENU)11,
        GetModuleHandle(NULL),
        NULL
    );

    CreateWindowExW(
        0,
        L"STATIC",
        L"Version:",
        WS_CHILD | WS_VISIBLE,
        20, 280,
        60, 20,
        hMainWindow,
        NULL,
        GetModuleHandle(NULL),
        NULL
    );

    hVersionEdit = CreateWindowExW(
        WS_EX_CLIENTEDGE,
        L"EDIT",
        L"1.0.0",
        WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL,
        90, 280,
        100, 20,
        hMainWindow,
        NULL,
        GetModuleHandle(NULL),
        NULL
    );

    CreateWindowExW(
        0,
        L"STATIC",
        L"Icon:",
        WS_CHILD | WS_VISIBLE,
        200, 280,
        40, 20,
        hMainWindow,
        NULL,
        GetModuleHandle(NULL),
        NULL
    );

    hIconPathEdit = CreateWindowExW(
        WS_EX_CLIENTEDGE,
        L"EDIT",
        L"",
        WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL | ES_READONLY,
        250, 280,
        300, 20,
        hMainWindow,
        NULL,
        GetModuleHandle(NULL),
        NULL
    );

    hIconBrowseButton = CreateWindowW(
        L"BUTTON",
        L"...",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        560, 280,
        30, 20,
        hMainWindow,
        (HMENU)10,
        GetModuleHandle(NULL),
        NULL
    );

    // Создаем текст лицензии и используем его
    HWND hLicenseTextEdit = CreateWindowExW(
        WS_EX_CLIENTEDGE,
        L"EDIT",
        currentTranslation->licenseText.c_str(),
        WS_CHILD | WS_VISIBLE | ES_MULTILINE | ES_READONLY | WS_VSCROLL,
        20, 320,
        740, 120,
        hMainWindow,
        (HMENU)102,
        GetModuleHandle(NULL),
        NULL
    );

    // Применяем современный шрифт к тексту лицензии
    if (hLicenseTextEdit != NULL) {
        SendMessageW(hLicenseTextEdit, WM_SETFONT, (WPARAM)hModernFont, TRUE);
    }

    hLicenseCheck = CreateWindowW(
        L"BUTTON",
        currentTranslation->acceptLicense.c_str(),
        WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
        20, 450,
        740, 20,
        hMainWindow,
        (HMENU)3,
        GetModuleHandle(NULL),
        NULL
    );

    hDesktopShortcutCheck = CreateWindowW(
        L"BUTTON",
        currentTranslation->desktopShortcut.c_str(),
        WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
        20, 480,
        200, 20,
        hMainWindow,
        (HMENU)4,
        GetModuleHandle(NULL),
        NULL
    );

    hStartMenuShortcutCheck = CreateWindowW(
        L"BUTTON",
        currentTranslation->startMenuShortcut.c_str(),
        WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
        230, 480,
        200, 20,
        hMainWindow,
        (HMENU)5,
        GetModuleHandle(NULL),
        NULL
    );

    hAutoRunCheck = CreateWindowW(
        L"BUTTON",
        currentTranslation->autoRun.c_str(),
        WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
        440, 480,
        200, 20,
        hMainWindow,
        (HMENU)6,
        GetModuleHandle(NULL),
        NULL
    );

    SendMessageW(hDesktopShortcutCheck, BM_SETCHECK, BST_CHECKED, 0);
    SendMessageW(hStartMenuShortcutCheck, BM_SETCHECK, BST_CHECKED, 0);
    SendMessageW(hAutoRunCheck, BM_SETCHECK, BST_CHECKED, 0);

    hProgress = CreateWindowExW(
        0,
        PROGRESS_CLASS,
        NULL,
        WS_CHILD | WS_VISIBLE,
        20, 510,
        740, 20,
        hMainWindow,
        NULL,
        GetModuleHandle(NULL),
        NULL
    );

    hStatus = CreateWindowExW(
        0,
        L"STATIC",
        L"Ready to install...",
        WS_CHILD | WS_VISIBLE | SS_LEFT,
        20, 540,
        440, 20,
        hMainWindow,
        NULL,
        GetModuleHandle(NULL),
        NULL
    );

    hInstallButton = CreateWindowW(
        L"BUTTON",
        L"Create Installer",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        640, 530,
        120, 30,
        hMainWindow,
        (HMENU)2,
        GetModuleHandle(NULL),
        NULL
    );

    // Создаем современный шрифт
    hModernFont = CreateModernFont();
    hBackgroundBrush = CreateSolidBrush(RGB(255, 255, 255));

    // Создаем кнопку для добавления папок
    hAddFolderButton = CreateWindowW(
        L"BUTTON",
        L"Add Folder",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        640, 140,
        120, 30,
        hMainWindow,
        (HMENU)12,
        GetModuleHandle(NULL),
        NULL
    );
    ModernizeButton(hAddFolderButton);

    // Модернизируем существующие кнопки
    ModernizeButton(hAddFileButton);
    ModernizeButton(hRemoveFileButton);
    ModernizeButton(hInstallButton);
    ModernizeButton(hIconBrowseButton);

    // Включаем визуальные стили для ListView
    SetWindowTheme(hFilesList, L"Explorer", NULL);
    SendMessageW(hFilesList, WM_SETFONT, (WPARAM)hModernFont, TRUE);

    // Обновляем стиль окна для поддержки современного вида
    DWORD dwStyle = GetWindowLongW(hMainWindow, GWL_STYLE);
    dwStyle |= WS_CLIPCHILDREN;
    SetWindowLongW(hMainWindow, GWL_STYLE, dwStyle);

    // Включаем темную рамку окна для Windows 10
    BOOL value = TRUE;
    DwmSetWindowAttribute(hMainWindow, DWMWA_USE_IMMERSIVE_DARK_MODE, &value, sizeof(value));

    ShowWindow(hMainWindow, SW_SHOW);
    UpdateWindow(hMainWindow);
    UpdateUILanguage();
}

// Добавляем функцию сортировки
static int CALLBACK CompareListItems(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort) {
    FileInfo* file1 = reinterpret_cast<FileInfo*>(lParam1);
    FileInfo* file2 = reinterpret_cast<FileInfo*>(lParam2);
    
    int column = HIWORD(lParamSort);
    bool ascending = LOWORD(lParamSort);
    
    int result = 0;
    switch (column) {
        case 0: // Имя файла
            result = file1->displayName.compare(file2->displayName);
            break;
        case 1: // Размер
            result = (file1->size < file2->size) ? -1 : (file1->size > file2->size) ? 1 : 0;
            break;
    }
    
    return ascending ? result : -result;
}

// Обновляем обработчик сообщений
static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_CTLCOLORBTN:
            return (LRESULT)hBackgroundBrush;
            
        case WM_CTLCOLORSTATIC:
        case WM_CTLCOLOREDIT: {
            HDC hdc = (HDC)wParam;
            SetBkColor(hdc, RGB(255, 255, 255));
            SetTextColor(hdc, RGB(0, 0, 0));
            return (LRESULT)hBackgroundBrush;
        }

        case WM_COMMAND: {
            if (HIWORD(wParam) == CBN_SELCHANGE && LOWORD(wParam) == 7) {
                // Изменение языка
                int index = SendMessage(hLanguageCombo, CB_GETCURSEL, 0, 0);
                switch (index) {
                    case 0: currentLanguage = "en"; break;
                    case 1: currentLanguage = "ru"; break;
                    case 2: currentLanguage = "de"; break;
                }
                currentTranslation = &TRANSLATIONS.at(currentLanguage);
                UpdateUILanguage();
                
                // Обновляем состояние кнопки Install после смены языка
                bool licenseAccepted = SendMessage(hLicenseCheck, BM_GETCHECK, 0, 0) == BST_CHECKED;
                EnableWindow(hInstallButton, licenseAccepted);
            }
            else if (LOWORD(wParam) == 1) { // Browse button
                BROWSEINFOW bi = { 0 };
                bi.lpszTitle = currentTranslation->selectDir.c_str();
                bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE;
                
                LPITEMIDLIST pidl = SHBrowseForFolderW(&bi);
                if (pidl != NULL) {
                    WCHAR path[MAX_PATH];
                    if (SHGetPathFromIDListW(pidl, path)) {
                        SetWindowTextW(hPathEdit, path);
                        installPath = WideToAnsi(path);
                    }
                    CoTaskMemFree(pidl);
                }
            }
            else if (LOWORD(wParam) == 2) { // Create Installer button
                if (selectedFiles.empty()) {
                    MessageBoxW(hwnd, L"Please add files first", L"Error", MB_ICONERROR);
                    return 0;
                }

                OPENFILENAMEW ofn = { 0 };
                WCHAR szFile[MAX_PATH] = L"setup.exe";
                
                ofn.lStructSize = sizeof(ofn);
                ofn.hwndOwner = hwnd;
                ofn.lpstrFile = szFile;
                ofn.nMaxFile = sizeof(szFile);
                ofn.lpstrFilter = L"Executable Files\0*.exe\0All Files\0*.*\0";
                ofn.nFilterIndex = 1;
                ofn.lpstrDefExt = L"exe";
                ofn.Flags = OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST;

                if (GetSaveFileNameW(&ofn)) {
                    std::vector<WCHAR> version(256);
                    GetWindowTextW(hVersionEdit, version.data(), static_cast<int>(version.size()));

                    Installer installer;
                    for (const auto& fileInfo : selectedFiles) {
                        installer.addFile(fileInfo.path);
                    }

                    installer.setVersion(WideToAnsi(version.data()));
                    if (!iconPath.empty()) {
                        installer.setIcon(WideToAnsi(iconPath));
                    }

                    installer.createInstaller(WideToAnsi(szFile));
                    MessageBoxW(hwnd, L"Installer created successfully!", L"Success", MB_ICONINFORMATION);
                }
            }
            else if (LOWORD(wParam) == 3) { // License checkbox
                bool licenseAccepted = SendMessage(hLicenseCheck, BM_GETCHECK, 0, 0) == BST_CHECKED;
                EnableWindow(hInstallButton, licenseAccepted);
                
                // Обновляем текст кнопки Install
                SetWindowTextW(hInstallButton, currentTranslation->install.c_str());
            }
            else if (LOWORD(wParam) == 9) { // Add Files button
                std::vector<wchar_t> szFile(32768, 0);
                
                OPENFILENAMEW ofn = { 0 };
                ofn.lStructSize = sizeof(ofn);
                ofn.hwndOwner = hwnd;
                ofn.lpstrFile = szFile.data();
                ofn.nMaxFile = static_cast<DWORD>(szFile.size());
                ofn.lpstrFilter = L"All Files\0*.*\0";
                ofn.nFilterIndex = 1;
                ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_ALLOWMULTISELECT | OFN_EXPLORER;

                if (GetOpenFileNameW(&ofn)) {
                    WCHAR* p = szFile.data();
                    std::wstring directory = p;
                    p += directory.length() + 1;

                    if (*p) { // Выбрано несколько файлов
                        while (*p) {
                            std::wstring fullPath = directory + L"\\" + p;
                            
                            WIN32_FILE_ATTRIBUTE_DATA fad;
                            if (GetFileAttributesExW(fullPath.c_str(), GetFileExInfoStandard, &fad)) {
                                LARGE_INTEGER size;
                                size.HighPart = fad.nFileSizeHigh;
                                size.LowPart = fad.nFileSizeLow;
                                
                                selectedFiles.emplace_back(
                                    WideToAnsi(fullPath),
                                    static_cast<size_t>(size.QuadPart),
                                    std::wstring(p)
                                );
                            }
                            
                            p += wcslen(p) + 1;
                        }
                    }
                    else { // Выбран один файл
                        WIN32_FILE_ATTRIBUTE_DATA fad;
                        if (GetFileAttributesExW(directory.c_str(), GetFileExInfoStandard, &fad)) {
                            LARGE_INTEGER size;
                            size.HighPart = fad.nFileSizeHigh;
                            size.LowPart = fad.nFileSizeLow;
                            
                            std::wstring fileName = directory.substr(directory.find_last_of(L"\\") + 1);
                            selectedFiles.emplace_back(
                                WideToAnsi(directory),
                                static_cast<size_t>(size.QuadPart),
                                fileName
                            );
                        }
                    }
                    UpdateFilesList();
                }
            }
            else if (LOWORD(wParam) == 10) { // Icon Browse button
                OPENFILENAMEW ofn = { 0 };
                WCHAR szFile[MAX_PATH] = { 0 };
                
                ofn.lStructSize = sizeof(ofn);
                ofn.hwndOwner = hwnd;
                ofn.lpstrFile = szFile;
                ofn.nMaxFile = sizeof(szFile);
                ofn.lpstrFilter = L"Icon Files\0*.ico\0";
                ofn.nFilterIndex = 1;
                ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

                if (GetOpenFileNameW(&ofn)) {
                    iconPath = ofn.lpstrFile;
                    SetWindowTextW(hIconPathEdit, iconPath.c_str());
                }
            }
            else if (LOWORD(wParam) == 11) { // Remove File button
                RemoveSelectedFile();
            }
            else if (LOWORD(wParam) == 12) { // Add Folder button
                BROWSEINFOW bi = { 0 };
                bi.lpszTitle = L"Select Folder";
                bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE;
                
                LPITEMIDLIST pidl = SHBrowseForFolderW(&bi);
                if (pidl != NULL) {
                    WCHAR path[MAX_PATH];
                    if (SHGetPathFromIDListW(pidl, path)) {
                        AddFilesFromFolder(path);
                    }
                    CoTaskMemFree(pidl);
                }
                return 0;
            }
            break;
        }
        case WM_DESTROY: {
            if (hModernFont) DeleteObject(hModernFont);
            if (hBackgroundBrush) DeleteObject(hBackgroundBrush);
            PostQuitMessage(0);
            return 0;
        }
        case WM_DROPFILES: {
            HDROP hDrop = (HDROP)wParam;
            UINT fileCount = DragQueryFileW(hDrop, 0xFFFFFFFF, NULL, 0);
            
            for (UINT i = 0; i < fileCount; i++) {
                WCHAR filePath[MAX_PATH];
                DragQueryFileW(hDrop, i, filePath, MAX_PATH);
                
                WIN32_FILE_ATTRIBUTE_DATA fad;
                if (GetFileAttributesExW(filePath, GetFileExInfoStandard, &fad)) {
                    LARGE_INTEGER size;
                    size.HighPart = fad.nFileSizeHigh;
                    size.LowPart = fad.nFileSizeLow;
                    
                    std::wstring fileName = std::wstring(filePath);
                    fileName = fileName.substr(fileName.find_last_of(L"\\") + 1);
                    
                    selectedFiles.emplace_back(
                        WideToAnsi(filePath),
                        static_cast<size_t>(size.QuadPart),
                        fileName
                    );
                }
            }
            
            DragFinish(hDrop);
            UpdateFilesList();
            return 0;
        }
        
        case WM_NOTIFY: {
            LPNMHDR nmhdr = (LPNMHDR)lParam;
            if (nmhdr->hwndFrom == hFilesList) {
                switch (nmhdr->code) {
                    case LVN_COLUMNCLICK: {
                        LPNMLISTVIEW pnmv = (LPNMLISTVIEW)lParam;
                        static bool ascending = true;
                        ascending = !ascending;
                        
                        ListView_SortItems(hFilesList, CompareListItems, 
                            MAKELPARAM(ascending, pnmv->iSubItem));
                        return 0;
                    }
                    
                    case LVN_GETINFOTIP: {
                        LPNMLVGETINFOTIPW pGetInfoTip = (LPNMLVGETINFOTIPW)lParam;
                        if (pGetInfoTip->iItem >= 0 && pGetInfoTip->iItem < static_cast<int>(selectedFiles.size())) {
                            const FileInfo& file = selectedFiles[pGetInfoTip->iItem];
                            std::wstring tooltip = AnsiToWide(file.path);
                            wcscpy_s(pGetInfoTip->pszText, pGetInfoTip->cchTextMax, tooltip.c_str());
                        }
                        return 0;
                    }
                }
            }
            break;
        }
        
        case WM_CONTEXTMENU: {
            if ((HWND)wParam == hFilesList) {
                POINT pt = { LOWORD(lParam), HIWORD(lParam) };
                if (pt.x == -1 && pt.y == -1) {
                    // Контекстное меню вызвано с клавиатуры
                    RECT rc;
                    int index = ListView_GetNextItem(hFilesList, -1, LVNI_SELECTED);
                    if (index != -1) {
                        ListView_GetItemRect(hFilesList, index, &rc, LVIR_BOUNDS);
                        pt.x = rc.left;
                        pt.y = rc.bottom;
                        ClientToScreen(hFilesList, &pt);
                    }
                }
                
                HMENU hMenu = CreatePopupMenu();
                AppendMenuW(hMenu, MF_STRING, 1, L"Remove");
                AppendMenuW(hMenu, MF_STRING, 2, L"Show in Explorer");
                AppendMenuW(hMenu, MF_SEPARATOR, 0, NULL);
                AppendMenuW(hMenu, MF_STRING, 3, L"Clear All");
                
                int cmd = TrackPopupMenu(hMenu, 
                    TPM_RETURNCMD | TPM_RIGHTBUTTON | TPM_NONOTIFY,
                    pt.x, pt.y, 0, hwnd, NULL);
                
                DestroyMenu(hMenu);
                
                switch (cmd) {
                    case 1:
                        RemoveSelectedFile();
                        break;
                    case 2: {
                        int index = ListView_GetNextItem(hFilesList, -1, LVNI_SELECTED);
                        if (index != -1) {
                            std::wstring path = AnsiToWide(selectedFiles[index].path);
                            std::wstring param = L"/select,\"" + path + L"\"";
                            ShellExecuteW(NULL, L"open", L"explorer.exe", 
                                param.c_str(), NULL, SW_SHOW);
                        }
                        break;
                    case 3:
                        selectedFiles.clear();
                        UpdateFilesList();
                        break;
                }
                return 0;
            }
            break;
        }
    }
    return DefWindowProcW(hwnd, uMsg, wParam, lParam);
}

static bool CheckSystemCompatibility() {
    SYSTEM_INFO si;
    GetNativeSystemInfo(&si);
    
    #ifdef _WIN64
        if (si.wProcessorArchitecture != PROCESSOR_ARCHITECTURE_AMD64 &&
            si.wProcessorArchitecture != PROCESSOR_ARCHITECTURE_ARM64) {
            MessageBoxW(NULL, L"This application requires a 64-bit processor.", L"Error", MB_ICONERROR);
            return false;
        }
    #endif

    // Проверяем версию Windows используя VersionHelpers API
    if (!IsWindows10OrGreater()) {
        MessageBoxW(NULL, L"This application requires Windows 10 or later.", L"Error", MB_ICONERROR);
        return false;
    }
    
    return true;
}

_Use_decl_annotations_
int WINAPI wWinMain(
    _In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR lpCmdLine,
    _In_ int nCmdShow)
{
    UNREFERENCED_PARAMETER(hInstance);
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(nCmdShow);
    
    if (!CheckSystemCompatibility()) {
        return 1;
    }

    // Инициализация COM для работы с ярлыками
    HRESULT hr = CoInitialize(NULL);
    if (FAILED(hr)) {
        MessageBoxW(NULL, L"Failed to initialize COM", L"Error", MB_ICONERROR);
        return 1;
    }

    INT_PTR result = 0;
    MSG msg = { 0 };  // Инициализируем структуру нулями

    if (wcscmp(lpCmdLine, L"--create") == 0) {
        Installer installer;
        installer.createInstaller("setup.exe");
    }
    else {
        CreateInstallerWindow();

        while (GetMessage(&msg, NULL, 0, 0)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        result = msg.wParam;
    }

    CoUninitialize();
    
    // Безопасное преобразование INT_PTR в int
    #pragma warning(push)
    #pragma warning(disable: 4244)  // Отключаем предупреждение о потере данных
    return static_cast<int>(result);
    #pragma warning(pop)
} 