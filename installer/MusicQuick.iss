; MusicQuick Inno Setup install script
; Before compile: .\scripts\deploy.ps1
; Then open this file in Inno Setup Compiler and Build

#define MyAppName "MusicQuick"
#define MyAppVersion "0.1.0"
#define MyAppPublisher "MusicQuick"
#define MyAppExeName "MusicQuick.exe"
; Relative to this .iss file
#define DistDir "..\dist\MusicQuick"
; Must match QGuiApplication organization/application name
#define MyOrgName "WingSound"
#define MySettingsAppName "WingSound"

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
; Pack deploy output, but exclude portable marker so install mode uses AppData/Registry
Source: "{#DistDir}\*"; DestDir: "{app}"; Flags: ignoreversion recursesubdirs createallsubdirs; Excludes: "portable"

[Icons]
Name: "{group}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"
Name: "{autodesktop}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"; Tasks: desktopicon

[Run]
Filename: "{app}\{#MyAppExeName}"; Description: "{cm:LaunchProgram,{#StringChange(MyAppName, '&', '&&')}}"; Flags: nowait postinstall skipifsilent

[Code]
var
  DeleteUserData: Boolean;

function InitializeUninstall(): Boolean;
begin
  Result := True;
  DeleteUserData := False;
  { Default button = No (MB_DEFBUTTON2): keep settings unless user chooses Yes }
  if MsgBox(
       '是否同時刪除個人設定與快取？' + #13#10 + #13#10 +
       '選「否」可在重新安裝後保留壁紙、播放列表與偏好設定。',
       mbConfirmation, MB_YESNO or MB_DEFBUTTON2) = IDYES then
    DeleteUserData := True;
end;

procedure CurUninstallStepChanged(CurUninstallStep: TUninstallStep);
begin
  if (CurUninstallStep = usPostUninstall) and DeleteUserData then
  begin
    DelTree(ExpandConstant('{userappdata}\{#MyOrgName}\{#MySettingsAppName}'), True, True, True);
    DelTree(ExpandConstant('{localappdata}\{#MyOrgName}\{#MySettingsAppName}'), True, True, True);
    RegDeleteKeyIncludingSubkeys(HKEY_CURRENT_USER, 'Software\{#MyOrgName}\{#MySettingsAppName}');
  end;
end;
