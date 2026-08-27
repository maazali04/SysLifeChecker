[Setup]
AppName=SysLifeChecker
AppVersion=1.0.0
AppPublisher=Maaz Ali
DefaultDirName={autopf}\SysLifeChecker
DefaultGroupName=SysLifeChecker
OutputDir=Output_Installer
OutputBaseFilename=SysLifeChecker_Setup
Compression=lzma2/ultra
SolidCompression=yes
PrivilegesRequired=admin

[Files]
; 1. Main Application Executable from the root folder
Source: "SysLifeChecker.exe"; DestDir: "{app}"; Flags: ignoreversion

; 2. smartctl Helper from third_party/smartmontools (placed right next to your main exe)
Source: "third_party\smartmontools\smartctl.exe"; DestDir: "{app}"; Flags: ignoreversion

; 3. Include asset subfolders (fonts, icons, themes, UI scripts)
Source: "resources\*"; DestDir: "{app}\resources"; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "ui\*"; DestDir: "{app}\ui"; Flags: ignoreversion recursesubdirs createallsubdirs

[Icons]
Name: "{group}\SysLifeChecker"; Filename: "{app}\SysLifeChecker.exe"
Name: "{autodesktop}\SysLifeChecker"; Filename: "{app}\SysLifeChecker.exe"

[Run]
Filename: "{app}\SysLifeChecker.exe"; Description: "Launch SysLifeChecker"; Flags: nowait postinstall skipifsilent