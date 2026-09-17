# Кросс-сборка ItteC под Windows на macOS

Здесь лежит всё, что нужно, чтобы собрать `ItteC.exe` для Windows 11 x64,
не имея под рукой Windows-машины. Сборка выполняется на этом же макбуке
как обычный кросс-компиляционный процесс.

## Результат

- `win-release/` — готовая к запуску папка: `ItteC.exe` + все DLL и плагины.
- `ittec-win64.zip` (≈13 МБ) — архив этой папки для переноса на Windows-ПК.
  Qt на винде НЕ нужна — все библиотеки рядом с exe.

Запуск на Windows: распаковать архив и запустить `win-release/ItteC.exe`.

## Использованные инструменты

| Инструмент | Назначение | Где |
|---|---|---|
| Homebrew `mingw-w64` (v14, GCC 16.2) | кросс-компилятор x86_64 → Windows | `/opt/homebrew/bin/x86_64-w64-mingw32-g++` |
| macOS-qmake 6.11.0 | генерация Windows-Makefile | `/Users/roman/Qt/6.11.0/macos/bin/qmake` |
| Qt 6.11.0 `win64_mingw` (aqtinstall) | заголовки/либы/плагины для Windows | `qt/6.11.0/mingw_64/` |
| GNU make (из brew) | сборка Makefile Release | `/opt/homebrew/opt/make/libexec/gnubin/make` |
| Wine (опционально) | смок-тест exe прямо на маке | `/opt/homebrew/bin/wine` (`/Applications/Wine Stable.app`) |

## Как это было собрано (действия)

### 1. Кросс-компилятор

```bash
brew install mingw-w64
```

Появляется семейство команд `x86_64-w64-mingw32-*` (gcc, g++, windres, ar, ld...).

### 2. Qt-библиотеки для Windows

aqtinstall (v3.3.0 содержит баг для Qt 6.11 — `Failed to download checksum
Updates.xml`, поэтому ставится dev-версия с GitHub):

```bash
python3 -m venv /tmp/aqt-venv
/tmp/aqt-venv/bin/pip install --upgrade "git+https://github.com/miurahr/aqtinstall.git"
mkdir -p winbuild/qt
/tmp/aqt-venv/bin/aqt install-qt --outputdir winbuild/qt windows desktop 6.11.0 win64_mingw
```

Кит оказывается в `winbuild/qt/6.11.0/mingw_64/` (либы `libQt6*.a`, `.dll`,
mkspecs, plugins). Компилятор для кросса берётся из Homebrew, а не из кита —
`qmake.exe`/`moc.exe` кита являются Windows-бинарниками и на маке не запускаются.

### 3. Нативные инструменты moc/rcc/uic

qmake из Windows-кита ищет эти тулзы в `libexec/` кита, но там их нет —
в ките они лежат в `bin/` как `.exe`. Подкладываем symlink на НАТИВНЫЕ
(macOS) версии из mac-кита, чтобы их можно было реально запускать:

```bash
QTW=winbuild/qt/6.11.0/mingw_64
mkdir -p "$QTW/libexec"
for t in moc uic rcc; do
  ln -sf /Users/roman/Qt/6.11.0/macos/libexec/$t "$QTW/libexec/$t"
done
```

### 4. Обёртка над qmake (winbuild/qwrap/)

mac-qmake при вызове с `-spec win32-g++` берёт пути библиотек из своего
`qt.conf`. Чтобы он видел Windows-кит, сделан каталог-обёртка с symlink на
мак-qmake и своим `qt.conf`, где `Prefix`/`Data` указывают на Windows-кит,
а `Binaries`/`LibraryExecutables` — на macOS-кит (нативные тулзы):

```
winbuild/qwrap/qt.conf:

[Paths]
Prefix = <abs>/winbuild/qt/6.11.0/mingw_64
Binaries = /Users/roman/Qt/6.11.0/macos/bin
LibraryExecutables = /Users/roman/Qt/6.11.0/macos/libexec
Data = <abs>/winbuild/qt/6.11.0/mingw_64
```

Настроить проверкой: `winbuild/qwrap/qmake -query QT_INSTALL_*`.

### 5. Shim-каталог компиляторов (winbuild/tools/)

qmake (и сгенерированный Makefile) зовут `g++`, `gcc`, `windres`, `ar` и т.д.
по коротким именам — но в PATH первыми стоят Xcode-тулчейны (Apple clang).
Сделан каталог с symlink на кросс-версии, он первым в PATH при сборке:

```bash
mkdir -p winbuild/tools
for t in gcc g++ cpp ar ranlib nm strip as ld windres objdump dlltool rc windmc; do
  ln -sf /opt/homebrew/bin/x86_64-w64-mingw32-$t winbuild/tools/$t
done
```

### 6. Генерация Makefile

```bash
cd winbuild/build
export PATH="$PWD/../tools:/opt/homebrew/opt/make/libexec/gnubin:$PATH"
export QMAKEPATH=../qt/6.11.0/mingw_64
../qwrap/qmake -o Makefile ../../lab1_code/ItteC.pro -spec win32-g++ CONFIG+=release
```

qmake генерирует `Makefile.Release` для целевого приложения. ВАЖНО: передавать
компиляторы аргументами (`QMAKE_CXX=...`) бесполезно — mkspec перезаписывает их
и при валидации qmake всё равно зовёт `g++` из PATH. Поэтому и нужен shim-каталог.

### 7. Правка Makefile (сборка завершается линковкой)

Qt-кит собран с MinGW 13, который ожидается, а кросс-gcc на маке — 16.
Поэтому `libQt6EntryPoint.a` несовместим: он тянет старый символ
`__imp___argc`, которого в новом CRT больше нет (ошибка `undefined reference
to __imp___argc`). Лечится заменой точки входа:

1. Из `Makefile.Release` убрать `libQt6EntryPoint.a` из `LIBS` и define
   `-DQT_NEEDS_QMAIN` из `DEFINES`.
2. Добавить в список `OBJECTS` свой `release/winmain.o`.
3. Не задавать `-Wl,-e,mainCRTStartup` самому (иначе CRT ругнётся, что нет
   `WinMain`) — вместо этого добавить `.o` с реализацией `WinMain`.

`winmain.cpp` (winbuild/) воспроизводит поведение штатного Qt-энтрипоинта
вручную: парсит широкую командную строку в UTF-8 через
`CommandLineToArgvW`/`WideCharToMultiByte` и вызывает `main(argc, argv)`.
Файл специально лежит в `winbuild/`, а не в `lab1_code/`, чтобы не трогать
сдаваемые исходники.

```bash
x86_64-w64-mingw32-g++ -c -O2 -std=gnu++1z -mthreads -DUNICODE -D_UNICODE \
  -DWIN32 -DMINGW_HAS_SECURE_API=1 -o build/release/winmain.o winmain.cpp
```

После правки `Makefile.Release` собрать:

```bash
cd winbuild/build
export PATH=".../tools:/opt/homebrew/opt/make/libexec/gnubin:$PATH"
make -j8
```

Результат: `build/release/ItteC.exe` —
`PE32+ executable (GUI) x86-64, for MS Windows`.

### 8. Деплой (win-release/)

`windeployqt` — тоже Windows-бинарник, на маке не запускается, поэтому папка
собирается копированием вручную (см. состав в начале):

- `ItteC.exe`
- `Qt6Core.dll`, `Qt6Gui.dll`, `Qt6Widgets.dll`, `Qt6Svg.dll`
- компиляторный рантайм из кита: `libgcc_s_seh-1.dll`, `libstdc++-6.dll`,
  `libwinpthread-1.dll`
- `platforms/qwindows.dll`
- `imageformats/qgif.dll`, `qico.dll`, `qjpeg.dll`, `qsvg.dll`

```bash
cd winbuild
rm -rf win-release && mkdir -p win-release/platforms win-release/imageformats
cp build/release/ItteC.exe win-release/
cp qt/6.11.0/mingw_64/bin/{Qt6Core,Qt6Gui,Qt6Widgets,Qt6Svg}.dll win-release/
cp qt/6.11.0/mingw_64/bin/libgcc_s_seh-1.dll qt/6.11.0/mingw_64/bin/libstdc++-6.dll qt/6.11.0/mingw_64/bin/libwinpthread-1.dll win-release/
cp qt/6.11.0/mingw_64/plugins/platforms/qwindows.dll win-release/platforms/
cp qt/6.11.0/mingw_64/plugins/imageformats/qgif.dll qt/6.11.0/mingw_64/plugins/imageformats/qico.dll qt/6.11.0/mingw_64/plugins/imageformats/qjpeg.dll qt/6.11.0/mingw_64/plugins/imageformats/qsvg.dll win-release/imageformats/
# упаковка
ditto -c -k --sequesterRsrc --keepParent win-release ittec-win64.zip
```

## Проверка на маке (Wine)

Запуск под Wine показывает, что DLL/плагины/символы подгружаются корректно:

```bash
cd winbuild/win-release
wine ./ItteC.exe &   # окно должно появиться, в логе нет err:module
pkill -f 'ItteC.exe'
```

Нужен Rosetta 2 (`/usr/bin/arch -x86_64 true` должен проходить).

## Подводные камни

- `aqtinstall` <=3.3.0 падает на Qt 6.11 (`Updates.xml`); нужна dev-версия.
- Нельзя запускать `qmake.exe`/`moc.exe` из Windows-кита — это PE-бинарники.
- Замену компиляторов для qmake делайте через PATH-шимы, не аргументами.
- После каждого перезапуска `qmake` правки в `Makefile.Release` (вин-энтрипоинт)
  сгорают — правки нужно накладывать заново после генерации.
- При расхождении GCC-версий Qt-кита и кросс-компилятора exe линкуется со
  старым MinGW-рантаймом из кита (совместимость вперёд соблюдается, тест под
  Wine это подтверждает).

## Ограничения

- Целевая платформа: Windows 11 x64 (тулчейн MinGW-w64, не MSVC).
- Финальный запуск на настоящем Windows-ПК не заменяет смок-тест на маке.
- Сборка пересоздаётся из `lab1_code/` — исходники сдачи не модифицируются.