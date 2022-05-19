REM %1 is the output dir where the executable exists
REM %2 is build platform, that is x64/Win32
REM %3 is build type, Debug/Release
REM %4 is the full path to the solution dir

copy %USAGI_DIR%\Engine\ThirdParty\glew\bin\release\%2\glew32.dll %1

if "%2" == "x64" (
	if "%3" == "Debug" (
		copy %USAGI_DIR%\Engine\ThirdParty\PhysX\physx\bin\win.x86_64.vc142.mt\Debug\PhysX_64.dll %1
		copy %USAGI_DIR%\Engine\ThirdParty\PhysX\physx\bin\win.x86_64.vc142.mt\Debug\PhysXCommon_64.dll %1
		copy %USAGI_DIR%\Engine\ThirdParty\PhysX\physx\bin\win.x86_64.vc142.mt\Debug\PhysXCooking_64.dll %1
		copy %USAGI_DIR%\Engine\ThirdParty\PhysX\physx\bin\win.x86_64.vc142.mt\Debug\PhysXDevice64.dll %1
		copy %USAGI_DIR%\Engine\ThirdParty\PhysX\physx\bin\win.x86_64.vc142.mt\Debug\PhysXFoundation_64.dll %1
		copy %USAGI_DIR%\Engine\ThirdParty\PhysX\physx\bin\win.x86_64.vc142.mt\Debug\PhysXGpu_64.dll %1
	)
	else (
		copy %USAGI_DIR%\Engine\ThirdParty\PhysX\physx\bin\win.x86_64.vc142.mt\Release\PhysX_64.dll %1
		copy %USAGI_DIR%\Engine\ThirdParty\PhysX\physx\bin\win.x86_64.vc142.mt\Release\PhysXCommon_64.dll %1
		copy %USAGI_DIR%\Engine\ThirdParty\PhysX\physx\bin\win.x86_64.vc142.mt\Release\PhysXCooking_64.dll %1
		copy %USAGI_DIR%\Engine\ThirdParty\PhysX\physx\bin\win.x86_64.vc142.mt\Release\PhysXDevice64.dll %1
		copy %USAGI_DIR%\Engine\ThirdParty\PhysX\physx\bin\win.x86_64.vc142.mt\Release\PhysXFoundation_64.dll %1
		copy %USAGI_DIR%\Engine\ThirdParty\PhysX\physx\bin\win.x86_64.vc142.mt\Release\PhysXGpu_64.dll %1
	)
)
