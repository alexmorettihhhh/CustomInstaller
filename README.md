# Custom Installer

Современный установщик приложений на C++20 с поддержкой Windows 10/11.

## Возможности

- Современный пользовательский интерфейс в стиле Windows 11
- Поддержка тёмной и светлой темы
- Drag & Drop файлов и папок
- Многоязычный интерфейс
- Создание ярлыков на рабочем столе и в меню "Пуск"
- Поддержка длинных путей
- Проверка целостности файлов
- Автоматическое обновление
- Поддержка Unicode

## Системные требования

- Windows 10 1809 или новее
- Visual Studio 2019/2022 с поддержкой C++20
- CMake 3.20 или новее

## Сборка проекта

### Через Visual Studio

1. Откройте папку проекта в Visual Studio
2. Выберите конфигурацию (Debug/Release) и архитектуру (x64/x86)
3. Нажмите "Сборка" -> "Собрать решение"

### Через командную строку

```powershell
# Клонирование репозитория
git clone https://github.com/your-username/custom-installer
cd custom-installer

# Создание и переход в папку сборки
mkdir build
cd build

# Генерация проекта
cmake -G "Visual Studio 17 2022" -A x64 ..

# Сборка
cmake --build . --config Release
```

## Использование

### Создание установщика

```powershell
installer.exe --create <output_installer.exe> [options] <files...>

Опции:
  --name "App Name"           # Имя приложения
  --version "1.0.0"          # Версия
  --company "Company Name"    # Название компании
  --icon path/to/icon.ico    # Путь к иконке
  --license license.txt      # Файл лицензии
```

### Установка

```powershell
installer.exe --install <installer.exe> [options]

Опции:
  --silent                   # Тихая установка
  --dir "C:\Program Files"   # Папка установки
  --no-desktop              # Без ярлыка на рабочем столе
  --no-startmenu           # Без ярлыка в меню "Пуск"
```

## Примеры

### Базовое использование

```powershell
# Создание установщика
installer.exe --create MyApp.exe --name "My Application" app.exe resources/* 

# Установка
installer.exe --install MyApp.exe --dir "C:\Program Files\MyApp"
```

### Расширенное использование

```powershell
# Создание установщика с дополнительными опциями
installer.exe --create MyApp.exe ^
  --name "My Application" ^
  --version "2.0.0" ^
  --company "My Company" ^
  --icon app.ico ^
  --license LICENSE.txt ^
  app.exe ^
  resources/* ^
  docs/*

# Тихая установка
installer.exe --install MyApp.exe --silent --dir "C:\Program Files\MyApp"
```

## Лицензия

MIT License. См. файл `LICENSE` для подробностей.

## Поддержка

При возникновении проблем создайте issue в репозитории проекта. 