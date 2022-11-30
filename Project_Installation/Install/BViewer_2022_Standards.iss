; -- BViewer_Standards.iss --
; Installs ILO 2022 Digital Standard images.

[Setup]
AppName=ILO Standards
AppVerName=ILO Standards 1.2q
DefaultDirName={code:ProgramData}\BViewer
DisableDirPage=no
DefaultGroupName=BViewer
Compression=lzma
SolidCompression=no
SourceDir=F:\Tom\BViewer\Install\Files
SetupIconFile=BViewer.ico
OutputDir=F:\Tom\BViewer\Install\Output
OutputBaseFilename=Install2022Standards
;LicenseFile=License.rtf


[Dirs]
Name: "{code:ProgramData}\BViewer\Standards"; Flags: uninsneveruninstall; Permissions: users-modify
Name: "{code:ProgramData}"; Permissions: users-modify; Check: IsInstalledOnExternalDrive
 
[Files]
Source: "Standards2022\*.png"; DestDir: "{code:ProgramData}\BViewer\Standards"; Attribs: readonly; Flags: overwritereadonly

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


