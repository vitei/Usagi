param(
    [ValidateSet("Debug", "Release")]
    [string]$Configuration = "Debug",
    [ValidateSet("x64")]
    [string]$Platform = "x64",
    [int]$TimeoutSeconds = 15,
    [switch]$SkipBuild
)

$ErrorActionPreference = "Stop"

function Get-RepoRoot {
    $root = (Resolve-Path $PSScriptRoot).Path
    while ($root -and !(Test-Path (Join-Path $root "Engine\CommonProps.props"))) {
        $parent = Split-Path $root -Parent
        if ($parent -eq $root) {
            break
        }

        $root = $parent
    }

    if (!$root -or !(Test-Path (Join-Path $root "Engine\CommonProps.props"))) {
        throw "Could not locate Usagi repository root from $PSScriptRoot"
    }

    return $root
}

function Get-MSBuild {
    $fromPath = Get-Command msbuild.exe -ErrorAction SilentlyContinue
    if ($fromPath) {
        return $fromPath.Source
    }

    $vswhere = Join-Path ${env:ProgramFiles(x86)} "Microsoft Visual Studio\Installer\vswhere.exe"
    if (Test-Path $vswhere) {
        $found = & $vswhere -latest -products * -requires Microsoft.Component.MSBuild -find "MSBuild\**\Bin\MSBuild.exe" | Select-Object -First 1
        if ($found) {
            return $found
        }
    }

    throw "MSBuild.exe was not found. Install Visual Studio Build Tools with C++ workload or run from a Developer PowerShell."
}

function Send-Json([System.Diagnostics.Process]$Process, [string]$Json) {
    $Process.StandardInput.WriteLine($Json)
    $Process.StandardInput.Flush()
}

function Wait-ForLine {
    param(
        [System.Diagnostics.Process]$Process,
        [scriptblock]$Predicate,
        [string]$Description,
        [int]$TimeoutSeconds
    )

    $deadline = [DateTime]::UtcNow.AddSeconds($TimeoutSeconds)
    $seen = New-Object System.Collections.Generic.List[string]
    $readTask = $null

    while ([DateTime]::UtcNow -lt $deadline) {
        if ($null -eq $readTask) {
            $readTask = $Process.StandardOutput.ReadLineAsync()
        }

        if ($readTask.Wait(50)) {
            $line = $readTask.Result
            $readTask = $null
            if ($null -eq $line) {
                break
            }

            $seen.Add($line)
            if (& $Predicate $line) {
                return $line
            }
        }

        if ($Process.HasExited) {
            break
        }
    }

    $exitDetails = ""
    if ($Process.HasExited) {
        $exitDetails = "Process exited with code $($Process.ExitCode)."
    }

    $stderr = ""
    if ($Process.HasExited) {
        $stderr = $Process.StandardError.ReadToEnd()
    }

    throw "Timed out waiting for $Description. $exitDetails`nstdout:`n$($seen -join "`n")`nstderr:`n$stderr"
}

$repoRoot = Get-RepoRoot
$env:USAGI_DIR = $repoRoot

$projectPath = Join-Path $repoRoot "Tools\Source\UsagiPreviewHost\project\UsagiPreviewHost.vcxproj"
$hostPath = Join-Path $repoRoot "Tools\bin\UsagiPreviewHost.exe"

if (!$SkipBuild) {
    $msbuild = Get-MSBuild
    & $msbuild $projectPath /m /p:Configuration=$Configuration /p:Platform=$Platform /v:minimal
    if ($LASTEXITCODE -ne 0) {
        exit $LASTEXITCODE
    }
}

if (!(Test-Path $hostPath)) {
    throw "Preview host executable was not found: $hostPath"
}

$startInfo = [System.Diagnostics.ProcessStartInfo]::new()
$startInfo.FileName = $hostPath
$startInfo.WorkingDirectory = $repoRoot
$startInfo.UseShellExecute = $false
$startInfo.CreateNoWindow = $true
$startInfo.RedirectStandardInput = $true
$startInfo.RedirectStandardOutput = $true
$startInfo.RedirectStandardError = $true

$process = [System.Diagnostics.Process]::new()
$process.StartInfo = $startInfo

try {
    [void]$process.Start()

    Send-Json $process '{"type":"pick","x":1,"y":2}'
    [void](Wait-ForLine $process { param($line) $line -match '"type":"error"' -and $line -match 'not been initialized' } "pre-init protocol error" $TimeoutSeconds)

    $dataPath = (Join-Path $repoRoot "Data").Replace("\", "\\")
    $romfilesPath = (Join-Path $repoRoot "_romfiles\win").Replace("\", "\\")
    Send-Json $process "{`"type`":`"init`",`"protocolVersion`":1,`"dataPath`":`"$dataPath`",`"romfilesPath`":`"$romfilesPath`"}"
    [void](Wait-ForLine $process { param($line) $line -match '"type":"ready"' -and $line -match '"protocolVersion":1' } "ready response" $TimeoutSeconds)

    Send-Json $process '{"type":"loadEntity","path":"Data/Entities/PreviewSmoke.yml"}'
    [void](Wait-ForLine $process { param($line) $line -match '"type":"loaded"' -and $line -match '"resourceType":"entity"' -and $line -match '"success":false' } "stub entity load response" $TimeoutSeconds)

    Send-Json $process '{"type":"loadParticle","emitterPath":"Data/Particle/Emitters/PreviewSmoke.yml","effectPath":null}'
    [void](Wait-ForLine $process { param($line) $line -match '"type":"loaded"' -and $line -match '"resourceType":"particle"' -and $line -match '"success":false' } "stub particle load response" $TimeoutSeconds)

    Send-Json $process '{"type":"pick","x":4,"y":8}'
    [void](Wait-ForLine $process { param($line) $line -match '"type":"picked"' -and $line -match '"entityId":null' } "empty pick response" $TimeoutSeconds)

    Send-Json $process '{"type":"shutdown"}'
    if (!$process.WaitForExit($TimeoutSeconds * 1000)) {
        $process.Kill()
        throw "Preview host did not exit after shutdown."
    }

    if ($process.ExitCode -ne 0) {
        $stderr = $process.StandardError.ReadToEnd()
        throw "Preview host exited with code $($process.ExitCode).`nstderr:`n$stderr"
    }

    Write-Host "Preview host headless smoke passed."
}
finally {
    if (!$process.HasExited) {
        try {
            Send-Json $process '{"type":"shutdown"}'
            if (!$process.WaitForExit(1000)) {
                $process.Kill()
            }
        }
        catch {
            try { $process.Kill() } catch { }
        }
    }

    $process.Dispose()
}
