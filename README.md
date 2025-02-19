# Custom Installer

Простой установщик приложений на C++, который позволяет создавать установочные файлы из набора файлов.

## Сборка проекта

1. Убедитесь, что у вас установлены:
   - CMake (версия 3.10 или выше)
   - Компилятор C++ с поддержкой C++17
   - Visual Studio (для Windows)

2. Создайте директорию для сборки и выполните команды:
```bash
mkdir build
cd build
cmake ..
cmake --build . --config Release
```

## Использование

### Создание установщика:
```bash
installer.exe --create <имя_установщика.exe> <файл1> <файл2> ...
```

### Установка:
```bash
installer.exe --install <установщик.exe> <путь_для_установки>
```

## Примеры

Создание установщика:
```bash
installer.exe --create myapp_installer.exe myapp.exe config.ini resources/
```

Установка:
```bash
installer.exe --install myapp_installer.exe "C:\Program Files\MyApp"
``` 