cd vendor
call InstallPyPackages.bat
call install_gems.bat
cd ..
call EnvironmentSetup.exe
@echo off
call Engine\ThirdParty\Physx\physx\generate_projects.bat vc16win64
echo Please now build debug and release from ThirdParty/Physx/physx/compiler/vc16win64 for debug and release
pause