[Setup]
AppName=Space Wargames
AppVersion=1.0
DefaultDirName={autopf}\Space Wargames
DefaultGroupName=Space Wargames
; --- CHANGEMENT ICI : Dossier de sortie de l'installeur ---
OutputDir=E:\space-wargames\windows
OutputBaseFilename=SpaceWargames_Setup
Compression=lzma
SolidCompression=yes

[Files]
; 1. L'exécutable principal (Vérifie qu'il est bien à la racine de SpaceWargames_Windows comme sur ton image)
Source: "E:\space-wargames\windows\SpaceWargames_Windows\main.exe"; DestDir: "{app}"; Flags: ignoreversion

; 2. Les DLLs
Source: "E:\space-wargames\windows\SpaceWargames_Windows\*.dll"; DestDir: "{app}"; Flags: ignoreversion

; 3. Les dossiers de données
Source: "E:\space-wargames\windows\SpaceWargames_Windows\assets\*"; DestDir: "{app}\assets"; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "E:\space-wargames\windows\SpaceWargames_Windows\configs\*"; DestDir: "{app}\configs"; Flags: ignoreversion recursesubdirs createallsubdirs

[Icons]
Name: "{group}\Space Wargames"; Filename: "{app}\main.exe"
Name: "{autodesktop}\Space Wargames"; Filename: "{app}\main.exe"; Tasks: desktopicon

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked

[UninstallDelete]
; Supprime le fichier de configuration ImGui
Type: files; Name: "{app}\imgui.ini"
; Supprime aussi les sauvegardes si tu en as créé dans le dossier
Type: files; Name: "{app}\save.json"