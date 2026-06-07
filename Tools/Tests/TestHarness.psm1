$script:HarnessName = $null
$script:Passed = 0
$script:Skipped = 0
$script:Failed = 0

function Initialize-TestHarness {
    param([Parameter(Mandatory=$true)][string]$Name)

    $script:HarnessName = $Name
    $script:Passed = 0
    $script:Skipped = 0
    $script:Failed = 0
    Write-Host "== $Name =="
}

function Get-UsagiRoot {
    return [System.IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..\..'))
}

function Get-TestsRoot {
    return $PSScriptRoot
}

function New-TestWorkspace {
    param([Parameter(Mandatory=$true)][string]$Name)

    $root = Join-Path $PSScriptRoot '.work'
    $path = Join-Path $root $Name
    Remove-Item -LiteralPath $path -Recurse -Force -ErrorAction SilentlyContinue
    New-Item -ItemType Directory -Force -Path $path | Out-Null
    return $path
}

function Assert-PathExists {
    param([Parameter(Mandatory=$true)][string]$Path)

    if (-not (Test-Path -LiteralPath $Path)) {
        throw "Required path not found: $Path"
    }
}

function Add-TestPass {
    param([Parameter(Mandatory=$true)][string]$Message)

    $script:Passed++
    Write-Host "[PASS] $Message"
}

function Add-TestSkip {
    param([Parameter(Mandatory=$true)][string]$Message)

    $script:Skipped++
    Write-Host "[SKIP] $Message"
}

function Invoke-TestStep {
    param(
        [Parameter(Mandatory=$true)][string]$Name,
        [Parameter(Mandatory=$true)][scriptblock]$ScriptBlock
    )

    try {
        & $ScriptBlock
        Add-TestPass $Name
    }
    catch {
        $script:Failed++
        Write-Host "[FAIL] $Name"
        Write-Host $_.Exception.Message
    }
}

function Invoke-ExternalCommand {
    param(
        [Parameter(Mandatory=$true)][string]$FilePath,
        [string[]]$Arguments = @(),
        [string]$WorkingDirectory = (Get-Location).Path,
        [int[]]$ExpectedExitCodes = @(0),
        [int]$TimeoutSeconds = 30
    )

    $workspace = New-TestWorkspace "process-$([Guid]::NewGuid().ToString('N'))"
    $stdout = Join-Path $workspace 'stdout.log'
    $stderr = Join-Path $workspace 'stderr.log'

    $escapedArgs = @()
    foreach ($arg in $Arguments) {
        if ($arg -match '[\s"]') {
            $escapedArgs += '"' + ($arg -replace '"', '\"') + '"'
        }
        else {
            $escapedArgs += $arg
        }
    }

    $startInfo = New-Object System.Diagnostics.ProcessStartInfo
    $startInfo.FileName = $FilePath
    $startInfo.Arguments = ($escapedArgs -join ' ')
    $startInfo.WorkingDirectory = $WorkingDirectory
    $startInfo.UseShellExecute = $false
    $startInfo.CreateNoWindow = $true
    $startInfo.RedirectStandardOutput = $true
    $startInfo.RedirectStandardError = $true

    $process = New-Object System.Diagnostics.Process
    $process.StartInfo = $startInfo

    if (-not $process.Start()) {
        throw "Failed to start command: $FilePath $($Arguments -join ' ')"
    }

    if (-not $process.WaitForExit($TimeoutSeconds * 1000)) {
        try {
            $process.Kill()
        }
        catch {
        }
        throw "Command timed out after ${TimeoutSeconds}s: $FilePath $($Arguments -join ' ')"
    }

    $outText = $process.StandardOutput.ReadToEnd()
    $errText = $process.StandardError.ReadToEnd()
    Set-Content -Encoding ASCII -Path $stdout -Value $outText
    Set-Content -Encoding ASCII -Path $stderr -Value $errText

    $exitCode = $process.ExitCode
    if ($ExpectedExitCodes -notcontains $exitCode) {
        throw "Unexpected exit code $exitCode from $FilePath $($Arguments -join ' ')`nSTDOUT:`n$outText`nSTDERR:`n$errText"
    }

    return [PSCustomObject]@{
        ExitCode = $exitCode
        StdOut = $stdout
        StdErr = $stderr
    }
}

function Complete-TestHarness {
    Write-Host "== $script:HarnessName summary: $script:Passed passed, $script:Skipped skipped, $script:Failed failed =="
    if ($script:Failed -gt 0) {
        exit 1
    }
}

Export-ModuleMember -Function `
    Initialize-TestHarness, `
    Get-UsagiRoot, `
    Get-TestsRoot, `
    New-TestWorkspace, `
    Assert-PathExists, `
    Add-TestPass, `
    Add-TestSkip, `
    Invoke-TestStep, `
    Invoke-ExternalCommand, `
    Complete-TestHarness
