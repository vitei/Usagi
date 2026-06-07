$ErrorActionPreference = 'Stop'

$TestDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$UsagiRoot = (Resolve-Path (Join-Path $TestDir '..\..\..')).Path

$SearchRoot = $UsagiRoot
$EnvScript = $null
while ($SearchRoot) {
    $Candidate = Join-Path $SearchRoot 'tools\usagi-dev-env.ps1'
    if (Test-Path $Candidate) {
        $EnvScript = $Candidate
        break
    }

    $Parent = Split-Path -Parent $SearchRoot
    if ($Parent -eq $SearchRoot) {
        break
    }
    $SearchRoot = $Parent
}

if (-not $EnvScript) {
    throw 'Unable to find tools\usagi-dev-env.ps1 from the test worktree.'
}

. $EnvScript

$EnvUsagiRoot = $env:USAGI_DIR
$env:USAGI_DIR = $UsagiRoot
$ToolsRoot = Split-Path -Parent $EnvScript

if (-not $env:MSBUILD_DIR) {
    throw 'MSBUILD_DIR is not set. Install Visual Studio Build Tools or update tools\usagi-dev-env.ps1.'
}

$BuildDir = Join-Path $ToolsRoot 'test-build\ResourcePakExporter'
New-Item -ItemType Directory -Force -Path $BuildDir | Out-Null

$Source = Join-Path $TestDir 'ResourcePakExporterTests.cpp'
$Exe = Join-Path $BuildDir 'ResourcePakExporterTests.exe'
Remove-Item -LiteralPath $Exe -Force -ErrorAction SilentlyContinue

$VcVarsCandidates = @(
    'C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat',
    'C:\Program Files\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvars64.bat',
    'C:\Program Files\Microsoft Visual Studio\2022\Professional\VC\Auxiliary\Build\vcvars64.bat',
    'C:\Program Files\Microsoft Visual Studio\2022\Enterprise\VC\Auxiliary\Build\vcvars64.bat'
)
$VcVars = $VcVarsCandidates | Where-Object { Test-Path $_ } | Select-Object -First 1
if (-not $VcVars) {
    throw 'Unable to find Visual Studio 2022 vcvars64.bat.'
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

function Add-IncludeIfExists {
    param(
        [System.Collections.Generic.List[string]]$Includes,
        [string]$Path
    )

    if ($Path -and (Test-Path $Path)) {
        $Includes.Add("/I$Path")
    }
}

$IncludeList = [System.Collections.Generic.List[string]]::new()
$IncludeList.Add("/I$(Join-Path $TestDir 'Stubs')")
$IncludeList.Add("/I$UsagiRoot")
Add-IncludeIfExists $IncludeList (Join-Path $UsagiRoot '_includes')
Add-IncludeIfExists $IncludeList (Join-Path $UsagiRoot '_build\win\debug')
Add-IncludeIfExists $IncludeList (Join-Path $UsagiRoot 'Engine\ThirdParty\nanopb')
Add-IncludeIfExists $IncludeList (Join-Path $UsagiRoot 'Engine\ThirdParty\EASTL\include')
Add-IncludeIfExists $IncludeList (Join-Path $UsagiRoot 'Engine\ThirdParty\EASTL\test\packages\EABase\include\Common')
Add-IncludeIfExists $IncludeList (Join-Path $UsagiRoot 'Engine\ThirdParty\lua-5.3.2\src')
Add-IncludeIfExists $IncludeList (Join-Path $UsagiRoot 'Engine\ThirdParty\yaml-cpp\include')
Add-IncludeIfExists $IncludeList (Join-Path $UsagiRoot 'Engine\ThirdParty\gli')
Add-IncludeIfExists $IncludeList (Join-Path $UsagiRoot 'Engine\ThirdParty\gli\external')

if ($EnvUsagiRoot -and $EnvUsagiRoot -ne $UsagiRoot) {
    Add-IncludeIfExists $IncludeList (Join-Path $EnvUsagiRoot '_build\win\debug')
    Add-IncludeIfExists $IncludeList (Join-Path $EnvUsagiRoot 'Engine\ThirdParty\nanopb')
    Add-IncludeIfExists $IncludeList (Join-Path $EnvUsagiRoot 'Engine\ThirdParty\EASTL\include')
    Add-IncludeIfExists $IncludeList (Join-Path $EnvUsagiRoot 'Engine\ThirdParty\EASTL\test\packages\EABase\include\Common')
    Add-IncludeIfExists $IncludeList (Join-Path $EnvUsagiRoot 'Engine\ThirdParty\lua-5.3.2\src')
    Add-IncludeIfExists $IncludeList (Join-Path $EnvUsagiRoot 'Engine\ThirdParty\yaml-cpp\include')
    Add-IncludeIfExists $IncludeList (Join-Path $EnvUsagiRoot 'Engine\ThirdParty\gli')
    Add-IncludeIfExists $IncludeList (Join-Path $EnvUsagiRoot 'Engine\ThirdParty\gli\external')
}

$Includes = $IncludeList.ToArray()

if ($env:VK_SDK_PATH) {
    $Includes += "/I$env:VK_SDK_PATH\Include"
}

$CommandParts = @(
    "`"$VcVars`" >nul &&",
    'cl /nologo /std:c++17 /EHsc /MTd /Zi'
) + $Defines + $Includes + @(
    "/Fo$BuildDir\",
    "/Fe$Exe",
    $Source
)

$Command = $CommandParts -join ' '

cmd /c $Command
if ($LASTEXITCODE -ne 0) {
    throw "ResourcePakExporter test compile failed with exit code $LASTEXITCODE."
}

& $Exe $BuildDir
