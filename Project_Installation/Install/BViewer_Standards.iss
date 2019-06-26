; -- BViewer_Standards.iss --
; Installs ILO Standard images.

[Setup]
AppName=ILO Standards
AppVerName=ILO Standards 1.1w
DefaultDirName={code:ProgramData}\BViewer
DisableDirPage=no
DefaultGroupName=BViewer
Compression=lzma
SolidCompression=no
SourceDir=C:\Tom\BViewer\Install\Files
SetupIconFile=BViewer.ico
OutputDir=C:\Tom\BViewer\Install\Output
OutputBaseFilename=InstallStandards
;LicenseFile=License.rtf


[Dirs]
Name: "{code:ProgramData}\BViewer\Standards"; Flags: uninsneveruninstall; Permissions: users-modify
Name: "{code:ProgramData}"; Permissions: users-modify; Check: IsInstalledOnExternalDrive
 
[Files]
Source: "Standards\*.png"; DestDir: "{code:ProgramData}\BViewer\Standards"; Attribs: readonly; Flags: overwritereadonly

[Code]

function IsInstalledOnExternalDrive(): Boolean;
begin
  Result := ( CompareText( ExtractFileDrive( ExpandConstant( '{code:ProgramData}' ) ), 'C:' ) <> 0 );
end;


// Reference a local {code:ProgramData} directory if installed on an external drive.
function ProgramData( Param: String ): String;
begin
  if ( CompareText( ExtractFileDrive( ExpandConstant( '{commonappdata}' ) ), 'C:' ) = 0 ) then
    begin
      Result := ExpandConstant( '{commonappdata}' );
    end
  else
    begin
      Result := ExtractFileDrive( ExpandConstant( '{commonappdata}' ) ) + '\ProgramData';
    end
end;


