<#
.SYNOPSIS
	Validates a built CozyChorus Suite VST3 with Tracktion pluginval.

.DESCRIPTION
	Downloads pluginval into tools/pluginval on first use (the folder is
	gitignored), then runs it against the VST3 in the build tree for the
	requested configuration and returns pluginval's own exit code:
	0 = every test passed, 1 = something failed.

	Two Windows quirks are handled here so callers don't have to:

	  * pluginval.exe is a JUCE GUI-subsystem binary, so PowerShell does NOT
	    block on it by default -- a bare `& pluginval.exe ...` returns instantly
	    and looks like it did nothing. Piping the output forces the wait, since
	    the stdout handle only closes when the process exits.
	  * For the same reason console text can come back empty, so --output-dir is
	    always passed and the resulting log is echoed if nothing reached stdout.

	Windows only: it fetches the pluginval_Windows.zip release asset.

.PARAMETER Config
	Build configuration to validate. Default Release -- Debug is slow enough to
	trip timeouts, and a JUCE jassert there opens a modal dialog that hangs the
	run. Validate Debug only when attached to a debugger.

.PARAMETER Strictness
	pluginval strictness, 1-10. 5 is pluginval's default and the floor for host
	compatibility; 10 adds parameter fuzzing and repeated state restoration and
	is the release gate.

.PARAMETER TimeoutMs
	Abort if no test produces output for this long. pluginval's own default is
	30s, which 96 kHz x 1024-sample runs blow through; -1 disables the timeout.

.EXAMPLE
	./scripts/Run-Pluginval.ps1
	Validate the Release VST3 at strictness 5.

.EXAMPLE
	./scripts/Run-Pluginval.ps1 -Strictness 10 -Repeat 3 -Randomise
	The release gate: full strictness, three randomised passes.

.EXAMPLE
	./scripts/Run-Pluginval.ps1 -SampleRates 48000 -BlockSizes 256
	Fast iteration loop while chasing a single failure.
#>
[CmdletBinding()]
param(
	[ValidateSet('Debug', 'Release')]
	[string] $Config = 'Release',

	[ValidateRange(1, 10)]
	[int] $Strictness = 5,

	[int] $TimeoutMs = 300000,

	# Override the plugin under test; defaults to the VST3 in the build tree.
	[string] $PluginPath = '',

	# Comma-separated overrides; pluginval defaults to 44100,48000,96000 and
	# 64,128,256,512,1024, i.e. 15 combinations of the two.
	[string] $SampleRates = '',
	[string] $BlockSizes = '',

	[int] $Repeat = 0,
	[switch] $Randomise,

	# For headless CI only -- locally the editor tests are worth running.
	[switch] $SkipGuiTests,

	# Re-download pluginval even if tools/pluginval/pluginval.exe exists.
	[switch] $ForceDownload
)

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

# PowerShell 7.3+ can turn a non-zero native exit code into a terminating error.
# We want to inspect pluginval's exit code ourselves, so opt out where supported.
if (Test-Path Variable:\PSNativeCommandUseErrorActionPreference)
{
	$PSNativeCommandUseErrorActionPreference = $false
}

if ($env:OS -ne 'Windows_NT')
{
	throw 'Run-Pluginval.ps1 is Windows-only (it fetches the pluginval_Windows.zip asset).'
}

$RepoRoot = Split-Path -Parent $PSScriptRoot
$ToolsDir = Join-Path $RepoRoot 'tools\pluginval'
$Exe      = Join-Path $ToolsDir 'pluginval.exe'
$LogDir   = Join-Path $RepoRoot "build\pluginval-logs\$Config"

# --- Acquire pluginval -----------------------------------------------------

if ($ForceDownload -and (Test-Path $ToolsDir))
{
	Remove-Item -Recurse -Force $ToolsDir
}

if (-not (Test-Path $Exe))
{
	$url = 'https://github.com/Tracktion/pluginval/releases/latest/download/pluginval_Windows.zip'
	$zip = Join-Path $env:TEMP 'pluginval_Windows.zip'

	Write-Host "pluginval not found -- downloading from $url"

	# Windows PowerShell 5.1 defaults to TLS 1.0, which GitHub refuses.
	[Net.ServicePointManager]::SecurityProtocol = [Net.SecurityProtocolType]::Tls12

	New-Item -ItemType Directory -Force -Path $ToolsDir | Out-Null
	Invoke-WebRequest -Uri $url -OutFile $zip -UseBasicParsing
	Expand-Archive -Path $zip -DestinationPath $ToolsDir -Force
	Remove-Item $zip -Force

	if (-not (Test-Path $Exe))
	{
		throw "Download succeeded but $Exe is missing -- the release asset layout may have changed."
	}
}

# --- Locate the plugin -----------------------------------------------------

if ([string]::IsNullOrWhiteSpace($PluginPath))
{
	$PluginPath = Join-Path $RepoRoot "build\CozyChorusSuite_artefacts\$Config\VST3\CozyChorus Suite.vst3"
}

if (-not (Test-Path $PluginPath))
{
	throw "No plugin at '$PluginPath'. Build it first: cmake --build build --config $Config"
}

New-Item -ItemType Directory -Force -Path $LogDir | Out-Null

# --- Build the argument list -----------------------------------------------

$pvArgs = @(
	'--strictness-level', $Strictness
	'--timeout-ms', $TimeoutMs
	'--output-dir', $LogDir
)

if (-not [string]::IsNullOrWhiteSpace($SampleRates)) { $pvArgs += @('--sample-rates', $SampleRates) }
if (-not [string]::IsNullOrWhiteSpace($BlockSizes))  { $pvArgs += @('--block-sizes', $BlockSizes) }
if ($Repeat -gt 0)                                   { $pvArgs += @('--repeat', $Repeat) }
if ($Randomise)                                      { $pvArgs += '--randomise' }
if ($SkipGuiTests)                                   { $pvArgs += '--skip-gui-tests' }

# Map the common -Verbose switch onto pluginval's own flag.
if ($PSBoundParameters.ContainsKey('Verbose')) { $pvArgs += '--verbose' }

# --validate must come last, immediately before the plugin path.
$pvArgs += @('--validate', $PluginPath)

Write-Host ''
Write-Host "Validating : $PluginPath"
Write-Host "Strictness : $Strictness"
Write-Host "Logs       : $LogDir"
Write-Host ''

# --- Run -------------------------------------------------------------------

$before = @(Get-ChildItem -Path $LogDir -File -ErrorAction SilentlyContinue)

# Piping is what makes PowerShell wait for this GUI-subsystem process (see the
# .DESCRIPTION note). Relax ErrorActionPreference across the call so that text
# pluginval writes to stderr becomes output rather than a terminating error.
$sawOutput = $false
try
{
	$ErrorActionPreference = 'Continue'
	& $Exe @pvArgs 2>&1 | ForEach-Object { $script:sawOutput = $true; Write-Host $_ }
}
finally
{
	$ErrorActionPreference = 'Stop'
}

$exitCode = $LASTEXITCODE

# --- Report ----------------------------------------------------------------

$after   = @(Get-ChildItem -Path $LogDir -File -ErrorAction SilentlyContinue)
$beforeNames = $before | ForEach-Object { $_.Name }
$newLogs = @($after | Where-Object { $beforeNames -notcontains $_.Name } | Sort-Object LastWriteTime)
$log     = if ($newLogs.Count -gt 0) { $newLogs[-1] } else { $null }

# GUI-subsystem builds can produce no console text at all; fall back to the log.
if (-not $sawOutput -and $null -ne $log)
{
	Get-Content $log.FullName | ForEach-Object { Write-Host $_ }
}

Write-Host ''
if ($null -ne $log)
{
	Write-Host "Log: $($log.FullName)"
}

if ($exitCode -eq 0)
{
	Write-Host "pluginval PASSED ($Config, strictness $Strictness)." -ForegroundColor Green
}
else
{
	Write-Host "pluginval FAILED ($Config, strictness $Strictness) -- exit code $exitCode." -ForegroundColor Red
}

exit $exitCode
