REM %1 is the output dir where the executable exists
REM %2 is build platform, that is x64/Win32
REM %3 is build type, Debug/Release
REM %4 is the full path to the solution dir

copy %USAGI_DIR%\Engine\ThirdParty\glew\bin\release\%2\glew32.dll %1

if "%2" == "x64" (
	if "%3" == "Debug" (
		copy %USAGI_PHYSICS%\Bin\vc15win64\PhysX3CommonDEBUG_x64.dll %1
		copy %USAGI_PHYSICS%\Bin\vc15win64\PhysX3CookingDEBUG_x64.dll %1
		copy %USAGI_PHYSICS%\Bin\vc15win64\PhysX3DEBUG_x64.dll %1
		copy %USAGI_PHYSICS%\..\PxShared\bin\vc15win64\PxFoundationDEBUG_x64.dll %1
		copy %USAGI_PHYSICS%\..\PxShared\bin\vc15win64\PxPvdSDKDEBUG_x64.dll %1
	)
	if "%3" == "Release" (
		copy %USAGI_PHYSICS%\Bin\vc15win64\PhysX3Common_x64.dll %1
		copy %USAGI_PHYSICS%\Bin\vc15win64\PhysX3Cooking_x64.dll %1
		copy %USAGI_PHYSICS%\Bin\vc15win64\PhysX3_x64.dll %1
		copy %USAGI_PHYSICS%\..\PxShared\bin\vc15win64\PxFoundation_x64.dll %1
		copy %USAGI_PHYSICS%\..\PxShared\bin\vc15win64\PxPvdSDK_x64.dll %1
	)
	copy %USAGI_PHYSICS%\Bin\vc15win64\nvToolsExt64_1.dll %1
)
