echo off
setlocal

set MAYAPY="C:\Program Files\Autodesk\Maya2015\bin\mayapy.exe"

pushd \\horace\nas3\python
%MAYAPY% get-pip.py
popd

%MAYAPY% -m easy_install pyyaml==3.11

endlocal

pause