# Установщик ItteC

В этой папке лежит `installer.iss` для [Inno Setup 6](https://jrsoftware.org/isinfo.php).
Скрипт упаковывает содержимое `winbuild/win-release/` (сам `ItteC.exe` и все
нужные DLL/плагины) в один установочный файл `Setup-ItteC.exe`.

## Как собрать

1. Установите Inno Setup 6 на Windows.
2. Откройте `installer.iss` в Inno Setup Compiler.
3. Нажмите **Compile** (или F9).
4. Готовый установщик появится в `laba1-release/dist/Setup-ItteC.exe`.

Установщик кладёт программу в папку пользователя (`%LocalAppData%\Programs\ItteC`),
не требует прав администратора, создаёт ярлык в «Пуск» и (по желанию) на рабочем
столе, корректно удаляется через «Установка и удаление программ».

Важно: `installer.iss` ссылается на `..\winbuild\win-release\*`. Если перенести
`installer.iss` в другую папку, поправьте путь в секции `[Files]`.

## Портативный вариант без установки

Если нужен просто один exe без инсталляции — возьмите готовый
`winbuild/ittec-win64.zip`. Если хочется всё в одном файле, который
сам распаковывается и запускается, можно собрать 7-Zip SFX: упаковать
содержимое `win-release/` в архив и приклеить к нему модуль `7z.sfx`
(в `7zSD.sfx` можно добавить `RunProgram="ItteC.exe"` в `config.txt`).

## Подпись

Сертификат и скрипты лежат в `winbuild/sign/`. Подписать установщик тем же
сертификатом:

На Windows (signtool, входит в Windows SDK):

```
signtool sign /fd SHA256 /f ..\winbuild\sign\ittec-selfsign.pfx /p ittec dist\Setup-ItteC.exe
```

На macOS (osslsigncode):

```
brew install osslsigncode
osslsigncode sign -certs winbuild/sign/ittec-selfsign.crt -key winbuild/sign/ittec-selfsign.key \
  -n "ItteC" -h sha256 -ts http://timestamp.digicert.com \
  -in dist/Setup-ItteC.exe -out dist/Setup-ItteC-signed.exe
```

Сертификат самоподписанный, поэтому Windows покажет издателя «Roman Severyn»
с пометкой «не подтверждено». Чтобы убрать предупреждения SmartScreen
полностью, нужен платный сертификат code signing — для лабораторной работы
это не требуется.
