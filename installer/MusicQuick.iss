; MusicQuick Inno Setup 安裝腳本
; 編譯前請先執行：  .\scripts\deploy.ps1
; 再用 Inno Setup Compiler 打開本檔案並 Compile

#define MyAppName "MusicQuick"
#define MyAppVersion "0.1.0"
#define MyAppPublisher "MusicQuick"
#define MyAppExeName "MusicQuick.exe"
; 相對於本 .iss 檔案所在目錄
#define DistDir "..\dist\MusicQuick"

[Setup]
AppId={{A3B8C2D1-4E5F-6789-ABCD-EF0123456789}
AppName={#MyAppName}
AppVersion={#MyAppVersion}
AppPublisher={#MyAppPublisher}
DefaultDirName={autopf}\{#MyAppName}
DefaultGroupName={#MyAppName}
DisableProgramGroupPage=yes
OutputDir=output
OutputBaseFilename=MusicQuick_Setup_{#MyAppVersion}
SetupIconFile=
Compression=lzma2
SolidCompression=yes
WizardStyle=modern
ArchitecturesInstallIn64BitMode=x64compatible
PrivilegesRequired=lowest

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked

[Files]
; 將 deploy.ps1 準備好的整個 dist 目錄安裝到 {app}
Source: "{#DistDir}\*"; DestDir: "{app}"; Flags: ignoreversion recursesubdirs createallsubdirs

[Icons]
Name: "{group}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"
Name: "{autodesktop}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"; Tasks: desktopicon

[Run]
Filename: "{app}\{#MyAppExeName}"; Description: "{cm:LaunchProgram,{#StringChange(MyAppName, '&', '&&')}}"; Flags: nowait postinstall skipifsilent
