; -- BViewer.iss --
; Installs BViewer.

[Setup]
AppName=BViewer
AppVerName=BViewer version 1.2p
DefaultDirName={pf}\BViewer
DisableDirPage=no
DefaultGroupName=BViewer
UninstallDisplayIcon={app}\BViewer.exe
Compression=lzma
SetupLogging=yes
SolidCompression=no
SourceDir=F:\Tom\BViewer\Install\Files
SetupIconFile=BViewer.ico
OutputDir=F:\Tom\BViewer\Install\Output
OutputBaseFilename=Setup
;LicenseFile=License.rtf

[Tasks]
Name: desktopicon; Description: "Create a &desktop icon"
Name: quicklaunchicon; Description: "Create a &Quick Launch icon"


[Dirs]
Name: "{app}\"; Permissions: users-readexec
Name: "{app}\BRetriever"
Name: "{code:ProgramData}"; Permissions: users-modify; Check: IsInstalledOnExternalDrive
Name: "{code:ProgramData}\BViewer\BRetriever\Inbox"; Permissions: users-modify
Name: "{code:ProgramData}\BViewer\BRetriever\Watch Folder"; Permissions: users-modify
Name: "{code:ProgramData}\BViewer\BRetriever\Images"; Flags: uninsneveruninstall; Permissions: users-modify
Name: "{code:ProgramData}\BViewer\ImageArchive"; Flags: uninsneveruninstall; Permissions: users-modify
Name: "{code:ProgramData}\BViewer\BRetriever\Log"; Flags: uninsneveruninstall; Permissions: users-modify
Name: "{code:ProgramData}\BViewer\BRetriever\Config"; Flags: uninsneveruninstall; Permissions: users-modify
Name: "{code:ProgramData}\BViewer\BRetriever\Service"; Flags: uninsneveruninstall; Permissions: users-modify
Name: "{code:ProgramData}\BViewer\BRetriever\Queued Files"; Permissions: users-modify
Name: "{code:ProgramData}\BViewer\BRetriever\Abstracts"; Flags: uninsneveruninstall; Permissions: users-modify
Name: "{code:ProgramData}\BViewer\BRetriever\Abstracts\Local"; Flags: uninsneveruninstall; Permissions: users-modify
Name: "{code:ProgramData}\BViewer\BRetriever\Abstracts\Local\Archive"; Flags: uninsneveruninstall; Permissions: users-modify
Name: "{code:ProgramData}\BViewer\BRetriever\Abstracts\Export"; Flags: uninsneveruninstall; Permissions: users-modify
Name: "{code:ProgramData}\BViewer\Clients"; Flags: uninsneveruninstall; Permissions: users-modify
Name: "{app}\Docs"
Name: "{code:ProgramData}\BViewer\Log"; Flags: uninsneveruninstall; Permissions: users-modify
Name: "{code:ProgramData}\BViewer\Data"; Flags: uninsneveruninstall; Permissions: users-modify
Name: "{code:ProgramData}\BViewer\DataArchive"; Flags: uninsneveruninstall; Permissions: users-modify
Name: "{code:ProgramData}\BViewer\Reports"; Flags: uninsneveruninstall; Permissions: users-modify
Name: "{code:ProgramData}\BViewer\ReportArchive"; Flags: uninsneveruninstall; Permissions: users-modify
Name: "{code:ProgramData}\BViewer\Signatures"; Flags: uninsneveruninstall; Permissions: users-modify
Name: "{app}\Config"; Flags: uninsneveruninstall
Name: "{code:ProgramData}\BViewer\Config"; Flags: uninsneveruninstall; Permissions: users-modify
Name: "{code:ProgramData}\BViewer\Standards"; Flags: uninsneveruninstall; Permissions: users-modify


[Files]
; Note:  The default method for uninstall is to process the following list of files in reverse order, deleting as appropriate.
; Files marked as "uninsneveruninstall" are selectively deleted in the code section's uninstall event handlers below.
; Files marked as "onlyifdoesntexist" will need to be forceably installed when necessary for compatibility with the
;     new version of the executable programs.  This will require removing this flag for one version, then replacing it.
Source: "BViewer.exe"; DestDir: "{app}"; Attribs: readonly; Flags: overwritereadonly replacesameversion
Source: "BViewer.cfg"; DestDir: "{code:ProgramData}\BViewer\Config"
Source: "BViewerSplash.bmp"; DestDir: "{code:ProgramData}\BViewer\Config"
; Install the default reader information file.
Source: "CriticalData1.sav"; DestDir: "{code:ProgramData}\BViewer\Config"; Flags: onlyifdoesntexist  uninsneveruninstall
; Install the default BViewer setup screen customization data.
Source: "CriticalData2.sav"; DestDir:  "{code:ProgramData}\BViewer\Config"; Flags: onlyifdoesntexist uninsneveruninstall
Source: "BViewer.ico"; DestDir: "{app}\Config"; Attribs: readonly; Flags: ignoreversion overwritereadonly
Source: "BStandards.ico"; DestDir: "{app}\Config"; Attribs: readonly; Flags: ignoreversion overwritereadonly
Source: "DicomDictionary.txt"; DestDir: "{code:ProgramData}\BViewer\Config"; Attribs: readonly; Flags: overwritereadonly
Source: "DicomDictionary.txt"; DestDir: "{code:ProgramData}\BViewer\BRetriever\Config"; Attribs: readonly; Flags: overwritereadonly
Source: "DicomDictionaryPrivate.txt"; DestDir: "{code:ProgramData}\BViewer\Config"; Attribs: readonly; Flags: overwritereadonly
Source: "DicomDictionaryPrivate.txt"; DestDir: "{code:ProgramData}\BViewer\BRetriever\Config"; Attribs: readonly; Flags: overwritereadonly
Source: "AboutBViewer.txt"; DestDir: "{app}\Docs"; Attribs: readonly; Flags: overwritereadonly
Source: "BViewerTechnicalRequirements.txt"; DestDir: "{app}\Docs"; Attribs: readonly; Flags: overwritereadonly
Source: "AbstractImport.cfg"; DestDir: "{code:ProgramData}\BViewer\BRetriever\Config"; Attribs: readonly; Flags: overwritereadonly uninsneveruninstall
; BViewer user manual, NIOSH version, used in NIOSH and TEST modes.
Source: "BViewer.chm"; DestDir: "{app}\Docs"; Attribs: readonly; Flags: overwritereadonly
; BViewer user manual, general-purpose version.
Source: "BViewerGP.chm"; DestDir: "{app}\Docs"; Attribs: readonly; Flags: overwritereadonly
Source: "ServiceController.exe"; DestDir: "{app}\BRetriever"; Attribs: readonly; Flags: overwritereadonly replacesameversion
Source: "ServiceController.cfg"; DestDir: "{code:ProgramData}\BViewer\BRetriever\Config"; Attribs: readonly; Flags: overwritereadonly
Source: "BRetriever.exe"; DestDir: "{app}\BRetriever"; Attribs: readonly; Flags: overwritereadonly replacesameversion
Source: "BRetriever.cfg"; DestDir: "{code:ProgramData}\BViewer\BRetriever\Config"; Attribs: readonly; Flags: overwritereadonly
Source: "Shared.cfg"; DestDir: "{code:ProgramData}\BViewer\BRetriever\Service"; Flags: onlyifdoesntexist uninsneveruninstall
;Source: "Selection.axt"; DestDir: "{app}\BRetriever\Abstracts\Local"
Source: "BViewerReleaseNotes.doc"; DestDir: "{app}\Docs"; Attribs: readonly; Flags: overwritereadonly
; Install report page templates for NIOSH and GP modes.
Source: "CWHSPReportPage1.png"; DestDir: "{code:ProgramData}\BViewer\Config"; Attribs: readonly; Flags: overwritereadonly
Source: "CWHSPReportPage2.png"; DestDir: "{code:ProgramData}\BViewer\Config"; Attribs: readonly; Flags: overwritereadonly
;Source: "CWHSPReportPage1.png"; DestDir: "{app}\Reports"; Attribs: readonly; Flags: overwritereadonly
;Source: "CWHSPReportPage2.png"; DestDir: "{app}\Reports"; Attribs: readonly; Flags: overwritereadonly
Source: "GPReportPage1.png"; DestDir: "{code:ProgramData}\BViewer\Config"; Attribs: readonly; Flags: overwritereadonly
Source: "GPReportPage2.png"; DestDir: "{code:ProgramData}\BViewer\Config"; Attribs: readonly; Flags: overwritereadonly
;Source: "GPReportPage1.png"; DestDir: "{app}\Reports"; Attribs: readonly; Flags: overwritereadonly
;Source: "GPReportPage2.png"; DestDir: "{app}\Reports"; Attribs: readonly; Flags: overwritereadonly
; Install default list of clients.
Source: "Client.cfg"; DestDir: "{code:ProgramData}\BViewer\Clients"; Flags: onlyifdoesntexist uninsneveruninstall
Source: "ClientSoutheastXRayInc.cfg"; DestDir: "{code:ProgramData}\BViewer\Clients"; Flags: onlyifdoesntexist uninsneveruninstall
; Install default signature image.
Source: "Signature.bmp"; DestDir: "{code:ProgramData}\BViewer\Signatures"; Flags: onlyifdoesntexist
; Install list of image files to be imported during first BViewer session.
Source: "Import.axt"; DestDir: "{code:ProgramData}\BViewer\BRetriever\Abstracts\Local"; Flags: onlyifdoesntexist uninsneveruninstall
; Install the three display calibration images.
Source: "2.16.124.113543.6004.101.103.20021117.185552.1.001.001.png"; DestDir: "{code:ProgramData}\BViewer\BRetriever\Images"
Source: "2.16.124.113543.6004.101.103.20021117.185552.1.003.001.png"; DestDir: "{code:ProgramData}\BViewer\BRetriever\Images"
Source: "2.16.124.113543.6004.101.103.20021117.162333.1.006.001.png"; DestDir: "{code:ProgramData}\BViewer\BRetriever\Images"
; Install a set of fake standards files, so that BViewer will function in the absence of an installed set of ILO standards.
Source: "FakeStandard.png"; DestDir: "{code:ProgramData}\BViewer\Standards"; DestName: "0example1.png"; Attribs: readonly; Flags: onlyifdoesntexist uninsneveruninstall
Source: "FakeStandard.png"; DestDir: "{code:ProgramData}\BViewer\Standards"; DestName: "0example2.png"; Attribs: readonly; Flags: onlyifdoesntexist uninsneveruninstall
Source: "FakeStandard.png"; DestDir: "{code:ProgramData}\BViewer\Standards"; DestName: "1p.png"; Attribs: readonly; Flags: onlyifdoesntexist uninsneveruninstall overwritereadonly
Source: "FakeStandard.png"; DestDir: "{code:ProgramData}\BViewer\Standards"; DestName: "1q.png"; Attribs: readonly; Flags: onlyifdoesntexist uninsneveruninstall overwritereadonly
Source: "FakeStandard.png"; DestDir: "{code:ProgramData}\BViewer\Standards"; DestName: "1r.png"; Attribs: readonly; Flags: onlyifdoesntexist uninsneveruninstall overwritereadonly
Source: "FakeStandard.png"; DestDir: "{code:ProgramData}\BViewer\Standards"; DestName: "1s.png"; Attribs: readonly; Flags: onlyifdoesntexist uninsneveruninstall overwritereadonly
Source: "FakeStandard.png"; DestDir: "{code:ProgramData}\BViewer\Standards"; DestName: "1t.png"; Attribs: readonly; Flags: onlyifdoesntexist uninsneveruninstall overwritereadonly
Source: "FakeStandard.png"; DestDir: "{code:ProgramData}\BViewer\Standards"; DestName: "2p.png"; Attribs: readonly; Flags: onlyifdoesntexist uninsneveruninstall overwritereadonly
Source: "FakeStandard.png"; DestDir: "{code:ProgramData}\BViewer\Standards"; DestName: "2q.png"; Attribs: readonly; Flags: onlyifdoesntexist uninsneveruninstall overwritereadonly
Source: "FakeStandard.png"; DestDir: "{code:ProgramData}\BViewer\Standards"; DestName: "2r.png"; Attribs: readonly; Flags: onlyifdoesntexist uninsneveruninstall overwritereadonly
Source: "FakeStandard.png"; DestDir: "{code:ProgramData}\BViewer\Standards"; DestName: "2s.png"; Attribs: readonly; Flags: onlyifdoesntexist uninsneveruninstall overwritereadonly
Source: "FakeStandard.png"; DestDir: "{code:ProgramData}\BViewer\Standards"; DestName: "2t.png"; Attribs: readonly; Flags: onlyifdoesntexist uninsneveruninstall overwritereadonly
Source: "FakeStandard.png"; DestDir: "{code:ProgramData}\BViewer\Standards"; DestName: "3p.png"; Attribs: readonly; Flags: onlyifdoesntexist uninsneveruninstall overwritereadonly
Source: "FakeStandard.png"; DestDir: "{code:ProgramData}\BViewer\Standards"; DestName: "3q.png"; Attribs: readonly; Flags: onlyifdoesntexist uninsneveruninstall overwritereadonly
Source: "FakeStandard.png"; DestDir: "{code:ProgramData}\BViewer\Standards"; DestName: "3r.png"; Attribs: readonly; Flags: onlyifdoesntexist uninsneveruninstall overwritereadonly
Source: "FakeStandard.png"; DestDir: "{code:ProgramData}\BViewer\Standards"; DestName: "3s.png"; Attribs: readonly; Flags: onlyifdoesntexist uninsneveruninstall overwritereadonly
Source: "FakeStandard.png"; DestDir: "{code:ProgramData}\BViewer\Standards"; DestName: "3t.png"; Attribs: readonly; Flags: onlyifdoesntexist uninsneveruninstall overwritereadonly
Source: "FakeStandard.png"; DestDir: "{code:ProgramData}\BViewer\Standards"; DestName: "A.png"; Attribs: readonly; Flags: onlyifdoesntexist uninsneveruninstall overwritereadonly
Source: "FakeStandard.png"; DestDir: "{code:ProgramData}\BViewer\Standards"; DestName: "B.png"; Attribs: readonly; Flags: onlyifdoesntexist uninsneveruninstall overwritereadonly
Source: "FakeStandard.png"; DestDir: "{code:ProgramData}\BViewer\Standards"; DestName: "C.png"; Attribs: readonly; Flags: onlyifdoesntexist uninsneveruninstall overwritereadonly
Source: "FakeStandard.png"; DestDir: "{code:ProgramData}\BViewer\Standards"; DestName: "quad_calcification_thickening.png"; Attribs: readonly; Flags: onlyifdoesntexist uninsneveruninstall overwritereadonly
Source: "FakeStandard.png"; DestDir: "{code:ProgramData}\BViewer\Standards"; DestName: "quad_u.png"; Attribs: readonly; Flags: onlyifdoesntexist uninsneveruninstall overwritereadonly


; Create shortcuts.
[Icons]
; Start menu:
Name: "{group}\BViewer"; Filename: "{app}\BVIEWER.EXE"; WorkingDir: "{app}"; Comment: "BViewer"; IconFilename: "{app}\Config\BViewer.ico";
; Desktop:
Name: "{commondesktop}\BViewer"; Filename: "{app}\BVIEWER.EXE"; WorkingDir: "{app}"; Comment: "BViewer"; IconFilename: "{app}\Config\BViewer.ico";

Name: "{group}\Uninstall BViewer"; Filename: "{uninstallexe}"


[Run]
Filename: "{app}\BRetriever\ServiceController.exe";  Parameters: "/install /start";  Description: "Install the BRetriever service.";  Flags: nowait


[UninstallRun]
Filename: "{app}\BRetriever\ServiceController.exe";  Parameters: "/stop /remove"


[UninstallDelete]
; Files and directories created by the install program are automatically deleted.
; This section is for any additional file or directories created by the program.
Type: filesandordirs; Name: "{code:ProgramData}\BViewer\BRetriever\Inbox"
Type: filesandordirs; Name: "{code:ProgramData}\BViewer\BRetriever\Queued Files"
Type: filesandordirs; Name: "{code:ProgramData}\BViewer\BRetriever\Watch Folder"
Type: files; Name: "{app}\BViewer.exe"
;Type: files; Name: "{app}\Docs\AboutBViewer.txt"
;Type: files; Name: "{app}\Docs\BViewerTechnicalRequirements.txt"
Type: files; Name: "{app}\BRetriever\*.exe"
Type: files; Name: "{code:ProgramData}\BViewer\Config\*.png"
;Type: files; Name: "{app}\Config\*.ico"
Type: files; Name: "{code:ProgramData}\BViewer\BRetriever\Config\DicomDictionary.txt"
Type: files; Name: "{code:ProgramData}\BViewer\Config\DicomDictionary.txt"
Type: files; Name: "{code:ProgramData}\BViewer\BRetriever\Service\BRetrieverStatus.dat"
Type: files; Name: "{code:ProgramData}\BViewer\BRetriever\Service\ServiceController.cfg"
Type: filesandordirs; Name: "{app}\Docs"
Type: filesandordirs; Name: "{app}\Config"


[Messages]
ConfirmUninstall=Are you sure you want to remove %1?


[Code]

function IsInstalledOnExternalDrive(): Boolean;
begin
  Result := ( CompareText( ExtractFileDrive( ExpandConstant( '{app}' ) ), 'C:' ) <> 0 );
end;


// Reference a local {code:ProgramData} directory if installed on an external drive.
function ProgramData( Param: String ): String;
begin
  if ( CompareText( ExtractFileDrive( ExpandConstant( '{app}' ) ), 'C:' ) = 0 ) then
    begin
      Result := ExpandConstant( '{commonappdata}' );
    end
  else
    begin
      Result := ExtractFileDrive( ExpandConstant( '{app}' ) ) + '\ProgramData';
    end
end;



var
  RemovePatientInfoCheckResult: Boolean;
  RemoveUserInfoCheckResult: Boolean;
  RemoveStandardsCheckResult: Boolean;


procedure InitializeUninstallProgressForm();
begin
  RemoveStandardsCheckResult :=  MsgBox('THINK!  Do you wish to remove the installed standard reference images?', mbConfirmation, MB_YESNO or MB_DEFBUTTON2) = IDYES;
  RemoveUserInfoCheckResult :=  MsgBox('THINK!  Do you wish to remove all of the existing readers'' information?', mbConfirmation, MB_YESNO or MB_DEFBUTTON2) = IDYES;
  if ( RemoveUserInfoCheckResult ) then
    begin
      RemovePatientInfoCheckResult := true;
    end
  else
    begin
      RemovePatientInfoCheckResult :=  MsgBox('THINK!  Do you wish to remove any existing patient information?', mbConfirmation, MB_YESNO or MB_DEFBUTTON2) = IDYES;
    end;
end;

  
procedure CurUninstallStepChanged(CurUninstallStep: TUninstallStep);
begin
  case CurUninstallStep of
    usPostUninstall:
      begin
        if ( RemoveStandardsCheckResult ) then
          begin
            DelTree(ExpandConstant('{code:ProgramData}\BViewer\Standards'), True, True, True);
          end;
        if ( RemovePatientInfoCheckResult ) then
          begin
            DelTree(ExpandConstant('{code:ProgramData}\BViewer\Reports'), True, True, True);
            DelTree(ExpandConstant('{code:ProgramData}\BViewer\Data'), True, True, True);
            DelTree(ExpandConstant('{code:ProgramData}\BViewer\BRetriever\Images'), True, True, True);
            DelTree(ExpandConstant('{code:ProgramData}\BViewer\BRetriever\Abstracts'), True, True, True);
          end;
        if ( RemoveUserInfoCheckResult ) then
          begin
            DeleteFile(ExpandConstant('{code:ProgramData}\BViewer\Config\CriticalData1.sav'));
//            DeleteFile(ExpandConstant('{app}\BRetriever\Shared.cfg'));
//            DeleteFile(ExpandConstant('{app}\BRetriever\Abstracts\Local\Import.axt'));
            DelTree(ExpandConstant('{code:ProgramData}\BViewer\Signatures'), True, True, True);
            DelTree(ExpandConstant('{code:ProgramData}\BViewer\Clients'), True, True, True);
            DelTree(ExpandConstant('{code:ProgramData}\BViewer\Config'), True, True, True);
            DelTree(ExpandConstant('{code:ProgramData}\BViewer\Log'), True, True, True);
            DelTree(ExpandConstant('{code:ProgramData}\BViewer\BRetriever\Log'), True, True, True);
            DelTree(ExpandConstant('{code:ProgramData}\BViewer\BRetriever\Config'), True, True, True);
            DelTree(ExpandConstant('{code:ProgramData}\BViewer\BRetriever\Service'), True, True, True);
            DelTree(ExpandConstant('{code:ProgramData}\BViewer\BRetriever'), True, True, True);
            DelTree(ExpandConstant('{app}\BRetriever'), True, True, True);
            // Note:  Abstract files should be deleted with the patient data below, after all
            // the directories are sorted out.
//            RemoveDir(ExpandConstant('{app}\BRetriever\Abstracts'));
          end;
        if ( RemoveStandardsCheckResult ) and ( RemoveUserInfoCheckResult ) then
          begin
            RemoveDir(ExpandConstant('{app}\BRetriever'));
            RemoveDir(ExpandConstant('{app}'));
          end;
      end;
  end;
end;


