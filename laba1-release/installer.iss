; Скрипт сборки установщика ItteC для Inno Setup 6.
; Пакет берётся из ..\winbuild\win-release (готовый exe + все DLL и плагины).
; Результат: dist\Setup-ItteC.exe — один файл установщика.
;
; Сборка: открыть этот файл в Inno Setup Compiler и нажать Compile (F9).

#define MyAppName "ItteC"
#define MyAppVersion "1.0.0"
#define MyAppPublisher "Roman Severyn"
#define MyAppExeName "ItteC.exe"

[Setup]
AppId={{8DCCC586-95DD-4205-88E9-B8694D381FD2}}
AppName={#MyAppName}
AppVersion={#MyAppVersion}
AppVerName={#MyAppName} {#MyAppVersion}
AppPublisher={#MyAppPublisher}
DefaultDirName={autopf}\{#MyAppName}
DefaultGroupName={#MyAppName}
DisableProgramGroupPage=yes
UninstallDisplayIcon={app}\{#MyAppExeName}
Compression=lzma2/max
SolidCompression=yes
WizardStyle=modern
PrivilegesRequired=lowest
ArchitecturesAllowed=x64compatible
ArchitecturesInstallIn64BitMode=x64compatible
OutputDir=dist
OutputBaseFilename=Setup-ItteC
SetupLogging=yes

[Languages]
Name: "russian"; MessagesFile: "compiler:Languages\Russian.isl"

[Tasks]
Name: "desktopicon"; Description: "Создать ярлык на рабочем столе"; GroupDescription: "Дополнительно:"; Flags: unchecked

[Files]
Source: "..\winbuild\win-release\*"; DestDir: "{app}"; Flags: ignoreversion recursesubdirs createallsubdirs

[Icons]
Name: "{group}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"
Name: "{group}\Удалить {#MyAppName}"; Filename: "{uninstallexe}"
Name: "{autodesktop}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"; Tasks: desktopicon

[Run]
Filename: "{app}\{#MyAppExeName}"; Description: "Запустить {#MyAppName}"; Flags: nowait postinstall skipifsilent
