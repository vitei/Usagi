@echo off
setlocal enabledelayedexpansion

REM Path to your requirements file
set REQUIREMENTS=..\Tools\python\pypackages.txt

REM Loop through each line in the requirements file
for /f "usebackq delims=" %%i in ("%REQUIREMENTS%") do (
    set "line=%%i"
    
    REM Skip empty lines or comment lines
    if not "!line!"=="" if "!line:~0,1!" neq "#" (
        echo Installing !line!...
        easy_install !line!
    )
)

echo Done!
pause