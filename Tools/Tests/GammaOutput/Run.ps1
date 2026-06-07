$ErrorActionPreference = 'Stop'

$TestDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$UsagiRoot = (Resolve-Path (Join-Path $TestDir '..\..\..')).Path
$BuildDir = Join-Path $UsagiRoot 'Tools\test-build\GammaOutput'
$PackageDir = Join-Path $BuildDir 'packages'
$TempDir = Join-Path $BuildDir 'tmp'
$Ramp = Join-Path $BuildDir 'srgb_expected.ppm'
New-Item -ItemType Directory -Force -Path $BuildDir, $PackageDir, $TempDir | Out-Null

function Read-RequiredText($Path) {
    if (-not (Test-Path $Path)) {
        throw "Required file not found: $Path"
    }
    Get-Content -Raw -Path $Path
}

function Convert-LinearToSrgbByte([double]$Value) {
    $Value = [Math]::Max($Value, 0.0)
    if ($Value -le 0.0031308) {
        $Srgb = $Value * 12.92
    } else {
        $Srgb = (1.055 * [Math]::Pow($Value, 1.0 / 2.4)) - 0.055
    }
    $Srgb = [Math]::Min([Math]::Max($Srgb, 0.0), 1.0)
    [byte][Math]::Floor(($Srgb * 255.0) + 0.5)
}

function Assert-Byte($Name, [double]$Linear, [byte]$Expected) {
    $Actual = Convert-LinearToSrgbByte $Linear
    if ($Actual -ne $Expected) {
        throw "$Name expected $Expected, got $Actual"
    }
}

$ColorSpace = Read-RequiredText (Join-Path $UsagiRoot 'Data\GLSL\shaders\includes\colorspace.inc')
$PostProcess = Read-RequiredText (Join-Path $UsagiRoot 'Data\GLSL\effects\PostProcess.yml')
$LinearToHDR = Read-RequiredText (Join-Path $UsagiRoot 'Data\GLSL\shaders\PostFX\LinearToHDR.frag')
$LinearToST2084 = Read-RequiredText (Join-Path $UsagiRoot 'Data\GLSL\shaders\PostFX\LinearToST2084.frag')

foreach ($Needle in @('0.0031308', '12.92', '1.0/2.4', '0.04045', '2.4')) {
    if (-not $ColorSpace.Contains($Needle)) {
        throw "colorspace.inc no longer contains expected sRGB transfer token: $Needle"
    }
}

foreach ($Needle in @('LinearToHDR', 'LinearToST2084', 'LinearToExtended')) {
    if (-not $PostProcess.Contains($Needle)) {
        throw "PostProcess.yml no longer packages HDR output effect: $Needle"
    }
}

foreach ($Needle in @('InverseTonemap', 'Hdr10', 'LinearToST2084')) {
    if (-not $LinearToHDR.Contains($Needle)) {
        throw "LinearToHDR.frag no longer contains expected HDR conversion token: $Needle"
    }
}

foreach ($Needle in @('k709to2020', 'kExpanded709to2020', 'LinearToST2084')) {
    if (-not $LinearToST2084.Contains($Needle)) {
        throw "LinearToST2084.frag no longer contains expected HDR10 token: $Needle"
    }
}

Assert-Byte 'black' -0.25 0
Assert-Byte 'linear zero' 0.0 0
Assert-Byte 'srgb toe boundary' 0.0031308 10
Assert-Byte 'middle gray' 0.18 118
Assert-Byte 'half linear' 0.5 188
Assert-Byte 'white' 1.0 255

$Last = 0
for ($X = 0; $X -lt 256; $X++) {
    $Current = Convert-LinearToSrgbByte ([double]$X / 255.0)
    if (($X -gt 0) -and ($Current -lt $Last)) {
        throw "sRGB ramp is not monotonic at $X"
    }
    $Last = $Current
}

$Header = [Text.Encoding]::ASCII.GetBytes("P6`n256 16`n255`n")
$Pixels = New-Object byte[] (256 * 16 * 3)
$Index = 0
for ($Y = 0; $Y -lt 16; $Y++) {
    for ($X = 0; $X -lt 256; $X++) {
        $Byte = Convert-LinearToSrgbByte ([double]$X / 255.0)
        $Pixels[$Index++] = $Byte
        $Pixels[$Index++] = $Byte
        $Pixels[$Index++] = $Byte
    }
}
[IO.File]::WriteAllBytes($Ramp, $Header + $Pixels)

$ShaderPackage = Join-Path $UsagiRoot 'Tools\bin\ShaderPackage.exe'
$PostProcessEffect = Join-Path $UsagiRoot 'Data\GLSL\effects\PostProcess.yml'
$ShaderDir = Join-Path $UsagiRoot 'Data\GLSL\shaders'
$PostProcessPackage = Join-Path $PackageDir 'PostProcess.pak'

if (-not (Test-Path $ShaderPackage)) {
    throw "ShaderPackage.exe not found: $ShaderPackage"
}

& $ShaderPackage "-avulkan" "-o$PostProcessPackage" "-t$TempDir" "-s$ShaderDir" $PostProcessEffect
if ($LASTEXITCODE -ne 0) {
    exit $LASTEXITCODE
}
if (-not (Test-Path $PostProcessPackage)) {
    throw "Shader package was not produced: $PostProcessPackage"
}

Write-Host 'Gamma output smoke test passed'
