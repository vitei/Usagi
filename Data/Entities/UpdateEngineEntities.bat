@echo off
SETLOCAL DisableDelayedExpansion
SET "r=%__CD__%"
(
echo Resources:
FOR /R . %%F IN (*.yml) DO (
  SET "p=%%F"
  SETLOCAL EnableDelayedExpansion
  ECHO(  - File: Entities\!p:%r%=!
  ENDLOCAL
) ) >../EngineEntities.yml

popd