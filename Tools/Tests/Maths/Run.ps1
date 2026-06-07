$ErrorActionPreference = 'Stop'

$TestDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$UsagiRoot = (Resolve-Path (Join-Path $TestDir '..\..\..')).Path
$ExternalToolsRoot = Join-Path (Split-Path -Parent (Split-Path -Parent $UsagiRoot)) 'tools'
$EnvScript = Join-Path $ExternalToolsRoot 'usagi-dev-env.ps1'

if (Test-Path $EnvScript) {
    & { . $EnvScript } *> $null
}

$env:USAGI_DIR = $UsagiRoot

$GeneratedInclude = Join-Path $UsagiRoot '_build\win\debug'
if (-not (Test-Path $GeneratedInclude)) {
    throw "Generated include directory not found: $GeneratedInclude. Run the Usagi build generation step before native tests."
}

$BuildDir = Join-Path $UsagiRoot 'Tools\test-build\Maths'
New-Item -ItemType Directory -Force -Path $BuildDir | Out-Null

$Source = Join-Path $TestDir 'MathsTests.cpp'
$Exe = Join-Path $BuildDir 'MathsTests.exe'

$VcVarsCandidates = @(
    $env:VCVARS64,
    'C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat',
    'C:\Program Files\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvars64.bat',
    'C:\Program Files\Microsoft Visual Studio\2022\Professional\VC\Auxiliary\Build\vcvars64.bat',
    'C:\Program Files\Microsoft Visual Studio\2022\Enterprise\VC\Auxiliary\Build\vcvars64.bat'
) | Where-Object { $_ -and (Test-Path $_) }

if (-not $VcVarsCandidates) {
    throw 'Unable to find vcvars64.bat. Install Visual Studio C++ tools or set VCVARS64.'
}

$Defines = @(
    '/DPLATFORM_PC',
    '/D_DEBUG',
    '/DFINAL_BUILD',
    '/DVK_USE_PLATFORM_WIN32_KHR',
    '/D"LUA_USER_H=<Engine/Framework/Script/LuaConf.h>"',
    '/DPB_FIELD_32BIT',
    '/DNN_SWITCH_ENABLE_HOST_IO',
    '/DEASTL_USER_DEFINED_ALLOCATOR',
    '/DEASTL_CUSTOM_FLOAT_CONSTANTS_REQUIRED=1',
    '/D_CRT_SECURE_NO_WARNINGS'
)

$Includes = @(
    "/I$UsagiRoot",
    "/I$GeneratedInclude",
    "/I$UsagiRoot\Engine\ThirdParty\nanopb",
    "/I$UsagiRoot\Engine\ThirdParty\EASTL\include",
    "/I$UsagiRoot\Engine\ThirdParty\EASTL\test\packages\EABase\include\Common",
    "/I$UsagiRoot\Engine\ThirdParty\lua-5.3.2\src",
    "/I$UsagiRoot\Engine\ThirdParty\yaml-cpp\include",
    "/I$UsagiRoot\Engine\ThirdParty\gli",
    "/I$UsagiRoot\Engine\ThirdParty\gli\external",
    "/I$env:VK_SDK_PATH\Include"
)

$Sources = @(
    $Source,
    (Join-Path $UsagiRoot 'Engine\Maths\AABB.cpp'),
    (Join-Path $UsagiRoot 'Engine\Maths\MathUtil.cpp'),
    (Join-Path $UsagiRoot 'Engine\Maths\Matrix3x3.cpp'),
    (Join-Path $UsagiRoot 'Engine\Maths\Matrix4x3.cpp'),
    (Join-Path $UsagiRoot 'Engine\Maths\Matrix4x4.cpp'),
    (Join-Path $UsagiRoot 'Engine\Maths\Plane.cpp'),
    (Join-Path $UsagiRoot 'Engine\Maths\Quaternionf.cpp'),
    (Join-Path $UsagiRoot 'Engine\Maths\Sphere.cpp'),
    (Join-Path $UsagiRoot 'Engine\Maths\Vector2f.cpp'),
    (Join-Path $UsagiRoot 'Engine\Maths\Vector3f.cpp'),
    (Join-Path $UsagiRoot 'Engine\Maths\Vector4f.cpp'),
    (Join-Path $UsagiRoot 'Engine\Maths\_vulkan\Matrix4x4_ps.cpp'),
    (Join-Path $UsagiRoot 'Engine\Core\Timer\_win\TimeTracker.cpp')
)

$CommandParts = @(
    "`"$($VcVarsCandidates[0])`" >nul &&",
    'cl /nologo /std:c++17 /EHsc /MTd /Zi /wd4291'
) + $Defines + $Includes + @(
    "/Fo$BuildDir\",
    "/Fe$Exe"
) + $Sources

cmd /c ($CommandParts -join ' ')
if ($LASTEXITCODE -ne 0) {
    throw "Maths test build failed with exit code $LASTEXITCODE"
}

& $Exe
if ($LASTEXITCODE -ne 0) {
    throw "Maths test executable failed with exit code $LASTEXITCODE"
}
