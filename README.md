# ItteC

Лабораторная работа 1 по инженерной и компьютерной графике (вариант 9).
Приложение для работы с цветом: редактор с поддержкой цветовых моделей
RGB, XYZ и LAB, интерактивный выбор цвета, градиентные слайдеры, кисть
для рисования.

Стек: C++17, Qt Widgets, сборка через qmake.

## Структура репозитория

- `src/` — исходники приложения (проект `ItteC.pro`).
- `tests/` — юнит-тесты цветовых преобразований (Qt Test).
- `docs/` — отчёты по лабораторной работе.
- `laba1-release/` — сборка Release под Windows: `build_release.bat`,
  раскладка с `windeployqt`.
- `winbuild/` — кросс-сборка Windows-версии с macOS. Готовая папка
  `winbuild/win-release/` и архив `ittec-win64.zip` уже собраны,
  подробности в `winbuild/README.md`.

## Сборка и запуск (macOS)

Нужен Qt 6. Сборку лучше делать в отдельной папке, чтобы не засорять
исходники:

```
mkdir build && cd build
qmake ../src/ItteC.pro
make
open ItteC.app
```

## Тесты

```
mkdir build-tests && cd build-tests
qmake ../tests/tests.pro
make
./runtests
```

## Windows

- На Windows: `laba1-release/build_release.bat` (требует Qt MinGW).
- Готовый exe и DLL уже лежат в `winbuild/win-release/` и `ittec-win64.zip` —
  можно просто распаковать и запустить.