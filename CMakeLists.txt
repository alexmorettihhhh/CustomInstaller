cmake_minimum_required(VERSION 3.10)

# Указываем имя проекта и язык
project(CustomInstaller CXX)

# Устанавливаем стандарт C++
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Включаем строгие предупреждения
if(MSVC)
    # Статическая линковка рантайма
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
    
    # Отключаем оптимизацию для отладки
    set(CMAKE_CXX_FLAGS_DEBUG "/Zi /Ob0 /Od /RTC1")
    # Включаем оптимизацию для релизной версии
    set(CMAKE_CXX_FLAGS_RELEASE "/O2 /Ob2 /DNDEBUG")
    
    add_compile_options(
        /W4     # Уровень предупреждений
        /wd4005 # Игнорировать предупреждение о переопределении макросов
        /EHsc   # Обработка исключений
        /utf-8  # Поддержка UTF-8
    )
    add_compile_definitions(
        WIN32
        _WINDOWS
        UNICODE
        _UNICODE
        _CRT_SECURE_NO_WARNINGS
    )
endif()

# Определяем архитектуру
if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(ARCH_SUFFIX "x64")
    add_compile_definitions(_WIN64)
else()
    set(ARCH_SUFFIX "x86")
    add_compile_definitions(_WIN32)
endif()

# Настройка выходных путей
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/${ARCH_SUFFIX})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib/${ARCH_SUFFIX})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib/${ARCH_SUFFIX})

# Установка типа приложения как GUI для Windows
if(WIN32)
    set(CMAKE_WIN32_EXECUTABLE TRUE)
    
    # Добавляем манифест
    set(APP_MANIFEST "${CMAKE_CURRENT_SOURCE_DIR}/app.manifest")
    if(NOT EXISTS ${APP_MANIFEST})
        file(WRITE ${APP_MANIFEST}
            "<?xml version='1.0' encoding='UTF-8' standalone='yes'?>\n"
            "<assembly xmlns='urn:schemas-microsoft-com:asm.v1' manifestVersion='1.0'>\n"
            "  <assemblyIdentity type='win32' name='CustomInstaller' version='1.0.0.0'/>\n"
            "  <trustInfo xmlns='urn:schemas-microsoft-com:asm.v3'>\n"
            "    <security>\n"
            "      <requestedPrivileges>\n"
            "        <requestedExecutionLevel level='requireAdministrator' uiAccess='false'/>\n"
            "      </requestedPrivileges>\n"
            "    </security>\n"
            "  </trustInfo>\n"
            "  <compatibility xmlns='urn:schemas-microsoft-com:compatibility.v1'>\n"
            "    <application>\n"
            "      <!-- Windows Vista and Windows Server 2008 -->\n"
            "      <supportedOS Id='{e2011457-1546-43c5-a5fe-008deee3d3f0}'/>\n"
            "      <!-- Windows 7 and Windows Server 2008 R2 -->\n"
            "      <supportedOS Id='{35138b9a-5d96-4fbd-8e2d-a2440225f93a}'/>\n"
            "      <!-- Windows 8 and Windows Server 2012 -->\n"
            "      <supportedOS Id='{4a2f28e3-53b9-4441-ba9c-d69d4a4a6e38}'/>\n"
            "      <!-- Windows 8.1 and Windows Server 2012 R2 -->\n"
            "      <supportedOS Id='{1f676c76-80e1-4239-95bb-83d0f6d0da78}'/>\n"
            "      <!-- Windows 10, Windows 11, Windows Server 2016, Windows Server 2019 and Windows Server 2022 -->\n"
            "      <supportedOS Id='{8e0f7a12-bfb3-4fe8-b9a5-48fd50a15a9a}'/>\n"
            "    </application>\n"
            "  </compatibility>\n"
            "  <application xmlns='urn:schemas-microsoft-com:asm.v3'>\n"
            "    <windowsSettings>\n"
            "      <dpiAware xmlns='http://schemas.microsoft.com/SMI/2005/WindowsSettings'>true</dpiAware>\n"
            "      <dpiAwareness xmlns='http://schemas.microsoft.com/SMI/2016/WindowsSettings'>PerMonitorV2, PerMonitor</dpiAwareness>\n"
            "      <longPathAware xmlns='http://schemas.microsoft.com/SMI/2016/WindowsSettings'>true</longPathAware>\n"
            "    </windowsSettings>\n"
            "  </application>\n"
            "  <dependency>\n"
            "    <dependentAssembly>\n"
            "      <assemblyIdentity type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'/>\n"
            "    </dependentAssembly>\n"
            "  </dependency>\n"
            "</assembly>\n"
        )
    endif()
endif()

# Добавление ресурсов
set(RESOURCE_RC "${CMAKE_CURRENT_SOURCE_DIR}/resource.rc")
if(NOT EXISTS ${RESOURCE_RC})
    file(WRITE ${RESOURCE_RC}
        "#include <windows.h>\n"
        "\n"
        "VS_VERSION_INFO VERSIONINFO\n"
        "FILEVERSION 1,0,0,0\n"
        "PRODUCTVERSION 1,0,0,0\n"
        "FILEFLAGSMASK 0x3fL\n"
        "FILEFLAGS 0x0L\n"
        "FILEOS 0x40004L\n"
        "FILETYPE 0x1L\n"
        "FILESUBTYPE 0x0L\n"
        "BEGIN\n"
        "    BLOCK \"StringFileInfo\"\n"
        "    BEGIN\n"
        "        BLOCK \"040904b0\"\n"
        "        BEGIN\n"
        "            VALUE \"CompanyName\", \"Your Company\"\n"
        "            VALUE \"FileDescription\", \"Custom Installer\"\n"
        "            VALUE \"FileVersion\", \"1.0.0.0\"\n"
        "            VALUE \"InternalName\", \"installer\"\n"
        "            VALUE \"LegalCopyright\", \"Copyright (C) 2024\"\n"
        "            VALUE \"OriginalFilename\", \"installer.exe\"\n"
        "            VALUE \"ProductName\", \"Custom Installer\"\n"
        "            VALUE \"ProductVersion\", \"1.0.0.0\"\n"
        "        END\n"
        "    END\n"
        "    BLOCK \"VarFileInfo\"\n"
        "    BEGIN\n"
        "        VALUE \"Translation\", 0x409, 1200\n"
        "    END\n"
        "END\n"
    )
endif()

# Добавление исполняемого файла
add_executable(installer 
    installer.cpp
    ${APP_MANIFEST}
    ${RESOURCE_RC}
)

# Линковка с необходимыми Windows библиотеками
if(WIN32)
    target_link_libraries(installer PRIVATE
        comctl32
        ole32
        uuid
        shell32
        advapi32
        comdlg32
        shlwapi
        version
        gdi32
        user32
        uxtheme
        dwmapi
    )
    
    # Устанавливаем версию Windows SDK и подсистему
    target_compile_definitions(installer PRIVATE
        WINVER=0x0A00
        _WIN32_WINNT=0x0A00
    )
    
    # Устанавливаем флаги линковщика
    set_target_properties(installer PROPERTIES
        LINK_FLAGS "/SUBSYSTEM:WINDOWS /MANIFEST:NO"
    )
endif() 