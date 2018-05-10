@echo off

SET VISUAL_STUDIO_BUILD=""
SET VISUAL_STUDIO_PLATFORM=%2

set PLATFORM="win"
set BUILD=%1
set TASK=%3

if "%2"=="x64" (
   set PLATFORM="win"
)

if "%TASK%"=="" (
  call rake platform=%PLATFORM% build=%BUILD%
) else (
  call rake platform=%PLATFORM% build=%BUILD% %TASK%
)
