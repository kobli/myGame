set branch=<BRANCH>
set arch=<ARCH>
set remote_addr=http://kobl.eu/dl/mygame/packages/%branch%
set tag
FOR /F "tokens=*" %%a in ('curl %remote_addr%/latest') do SET tag=%%a
set current_tag
for /f "delims=" %%x in (version) do set current_tag=%%x
set file_name=myGame-client-%tag%-%arch%.zip
set remote_file_addr=%remote_addr%/%tag%/%file_name%
set config_backup_dir=config_backup\%current_tag%
echo %tag%
echo %current_tag%

IF "%tag%"=="%current_tag%" (
    ECHO "Already up to date"
) ELSE (
    mkdir %config_backup_dir%
    move config\* %config_backup_dir%
  
    for %%i in (*) do if not %%i == upgrade.bat ( if not %%i == curl.exe ( if not %%i == 7za.exe del %%i))
    for /d %%x in (*) do if not %%x == config_backup @rd /s /q "%%x"
    curl -O %remote_file_addr%
    7za x -y %file_name%
)           
