; ===========================================
; ParseMindStudio - Inno Setup Installer
; ===========================================

[Setup]
AppName=ParseMind Studio
AppVersion=1.0.0
DefaultDirName={localappdata}\ParseMindStudio
DisableProgramGroupPage=yes
OutputDir=.
OutputBaseFilename=ParseMindStudioInstaller
Compression=lzma
SolidCompression=yes
WizardStyle=modern
ArchitecturesAllowed=x64
ArchitecturesInstallIn64BitMode=x64

[Files]
; Copy Qt app
Source: "build\ui_shell\ParseMindShell.exe"; DestDir: "{app}"; Flags: ignoreversion

; Copy Nuitka compiled module(s)
Source: "embedded_py\main*.pyd"; DestDir: "{app}"; Flags: ignoreversion

; Copy requirements file
Source: "backend_py\requirements.txt"; DestDir: "{app}"; Flags: ignoreversion

[Icons]
Name: "{group}\ParseMind Studio"; Filename: "{app}\ParseMindShell.exe"
Name: "{userdesktop}\ParseMind Studio"; Filename: "{app}\ParseMindShell.exe"

[Run]
Filename: "{app}\ParseMindShell.exe"; Description: "Launch ParseMind Studio"; Flags: nowait postinstall skipifsilent

[Code]
var
  PythonExe: string;

function PythonInstalled(): Boolean;
var
  ResultCode: Integer;
begin
  Result := Exec('python', '--version', '', SW_HIDE, ewWaitUntilTerminated, ResultCode)
    or Exec('py', '--version', '', SW_HIDE, ewWaitUntilTerminated, ResultCode);
end;

// ==========================================================
// Install Python if missing
// ==========================================================
function InstallPython(): Boolean;
var
  Bytes: Int64;
  PythonInstallerPath: string;
  ResultCode: Integer;
begin
  MsgBox('Python not found. The installer will download and install Python 3.11 automatically.',
         mbInformation, MB_OK);

  Bytes := DownloadTemporaryFile(
        'https://www.python.org/ftp/python/3.11.7/python-3.11.7-amd64.exe',
        PythonInstallerPath,
        '',
        'Failed to download Python installer.'
     );
  if Bytes <= 0 then
  begin
    MsgBox('Could not download Python.', mbError, MB_OK);
    Result := False;
    exit;
  end;

  Result := Exec(
      PythonInstallerPath,
      '/quiet InstallAllUsers=0 PrependPath=1 Include_test=0',
      '', SW_HIDE, ewWaitUntilTerminated, ResultCode
  );
end;

// ==========================================================
// Create VENV + Install dependencies silently (no freezing)
// ==========================================================
procedure CreateVenv();
var
  ResultCode: Integer;
  VenvPath, PythonEXEPath: string;
begin
  VenvPath := ExpandConstant('{localappdata}\ParseMindStudio\python');
  PythonExe := 'python';

  { Create venv folder }
  Exec(PythonExe,
       '-m venv "' + VenvPath + '"',
       '', SW_HIDE, ewWaitUntilTerminated, ResultCode);

  PythonEXEPath := VenvPath + '\Scripts\python.exe';

  { Upgrade pip }
  Exec(PythonEXEPath,
       '-m pip install --upgrade pip',
       '', SW_HIDE, ewWaitUntilTerminated, ResultCode);

  { Install dependencies }
  Exec(PythonEXEPath,
       '-m pip install -r "' + ExpandConstant('{app}\requirements.txt') + '"',
       '', SW_SHOW, ewWaitUntilTerminated, ResultCode);
end;

procedure InitializeWizard();
begin
  { Nothing extra }
end;

function NextButtonClick(CurPageID: Integer): Boolean;
begin
  Result := True;
end;

// ==========================================================
// Main install step
// ==========================================================
procedure CurStepChanged(CurStep: TSetupStep);
begin
  if CurStep = ssInstall then
  begin
    if not PythonInstalled() then
    begin
      if not InstallPython() then
      begin
        MsgBox('Python installation failed. Cannot continue.', mbError, MB_OK);
        WizardForm.Close;
      end;
    end;

    CreateVenv();
  end;
end;


[UninstallDelete]
Type: filesandordirs; Name: "{app}"

[Code]
var
  ResultCode: Integer;

procedure CurUninstallStepChanged(CurUninstallStep: TUninstallStep);
begin
  if CurUninstallStep = usUninstall then
  begin
    Exec('taskkill.exe', '/F /IM ParseMindShell.exe', '', SW_HIDE, ewWaitUntilTerminated, ResultCode);
    Exec('taskkill.exe', '/F /IM python.exe', '', SW_HIDE, ewWaitUntilTerminated, ResultCode);
    Exec('taskkill.exe', '/F /IM pythonw.exe', '', SW_HIDE, ewWaitUntilTerminated, ResultCode);
  end;
end;

