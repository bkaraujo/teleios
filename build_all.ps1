# ============================================================
# Script de Build Completo - TELEIOS
# Compila o Engine e o Sandbox
# ============================================================

param(
    [Parameter(Mandatory=$false)]
    [switch]$Clean,

    [Parameter(Mandatory=$false)]
    [switch]$EngineOnly,

    [Parameter(Mandatory=$false)]
    [switch]$SandboxOnly
)

$ErrorActionPreference = "Stop"

$RootDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$EngineScript = Join-Path $RootDir "engine\build.ps1"
$SandboxScript = Join-Path $RootDir "sandbox\build.ps1"

Write-Host ""
Write-Host "===============================================" -ForegroundColor Green
Write-Host "  TELEIOS - Build System" -ForegroundColor Green
Write-Host "===============================================" -ForegroundColor Green
Write-Host ""

$BuildEngine = $true
$BuildSandbox = $true

if ($EngineOnly) {
    $BuildSandbox = $false
}

if ($SandboxOnly) {
    $BuildEngine = $false
}

$Success = $true

# Build do Engine
if ($BuildEngine) {
    Write-Host ">>> Iniciando build do Engine..." -ForegroundColor Yellow
    Write-Host ""

    try {
        if ($Clean) {
            & $EngineScript -Clean
        } else {
            & $EngineScript
        }

        if ($LASTEXITCODE -ne 0) {
            $Success = $false
            Write-Host ""
            Write-Host "ERRO: Build do Engine falhou!" -ForegroundColor Red
        }
    } catch {
        $Success = $false
        Write-Host ""
        Write-Host "ERRO ao executar build do Engine: $_" -ForegroundColor Red
    }

    Write-Host ""
}

# Build do Sandbox (apenas se o engine foi bem sucedido ou não está sendo compilado)
if ($BuildSandbox -and $Success) {
    Write-Host ">>> Iniciando build do Sandbox..." -ForegroundColor Yellow
    Write-Host ""

    try {
        if ($Clean) {
            & $SandboxScript -Clean
        } else {
            & $SandboxScript
        }

        if ($LASTEXITCODE -ne 0) {
            $Success = $false
            Write-Host ""
            Write-Host "ERRO: Build do Sandbox falhou!" -ForegroundColor Red
        }
    } catch {
        $Success = $false
        Write-Host ""
        Write-Host "ERRO ao executar build do Sandbox: $_" -ForegroundColor Red
    }

    Write-Host ""
}

# Resultado final
Write-Host ""
Write-Host "===============================================" -ForegroundColor Green

if ($Success) {
    Write-Host "  Build concluído com SUCESSO!" -ForegroundColor Green
    Write-Host "===============================================" -ForegroundColor Green

    if ($BuildEngine) {
        $EngineExe = Join-Path $RootDir "engine\engine.exe"
        if (Test-Path $EngineExe) {
            Write-Host "  Engine: " -NoNewline -ForegroundColor Gray
            Write-Host "$EngineExe" -ForegroundColor Cyan
        }
    }

    if ($BuildSandbox) {
        $SandboxExe = Join-Path $RootDir "sandbox\sandbox.exe"
        if (Test-Path $SandboxExe) {
            Write-Host "  Sandbox: " -NoNewline -ForegroundColor Gray
            Write-Host "$SandboxExe" -ForegroundColor Cyan
        }
    }

    Write-Host "===============================================" -ForegroundColor Green
    exit 0
} else {
    Write-Host "  Build FALHOU!" -ForegroundColor Red
    Write-Host "===============================================" -ForegroundColor Red
    exit 1
}
