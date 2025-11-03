# ============================================================
# Script de Build do Sandbox TELEIOS
# ============================================================

param(
    [Parameter(Mandatory=$false)]
    [switch]$Clean
)

$ErrorActionPreference = "Stop"

# Obter diretório do sandbox
$SandboxDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$RootDir = Split-Path -Parent $SandboxDir
$CompileScript = Join-Path $RootDir "build\compile.ps1"
$EngineDir = Join-Path $RootDir "engine"

Write-Host "========================================" -ForegroundColor Cyan
Write-Host "Build do Sandbox TELEIOS" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""

# Limpar build se solicitado
if ($Clean) {
    $BuildDir = Join-Path $SandboxDir "build"
    if (Test-Path $BuildDir) {
        Write-Host "Limpando diretório de build..." -ForegroundColor Yellow
        Remove-Item -Path $BuildDir -Recurse -Force
    }
    $ExeFile = Join-Path $SandboxDir "sandbox.exe"
    if (Test-Path $ExeFile) {
        Remove-Item -Path $ExeFile -Force
    }
    Write-Host "Build limpo!" -ForegroundColor Green
    Write-Host ""
}

# Configurar flags de compilação do sandbox
$SandboxCompileFlags = @(
    "-O0",           # Sem otimização para debug
    "-g",            # Informações de debug
    "-gcodeview",    # Debug info para Windows
    "-I$EngineDir\src\main"  # Incluir headers do engine
)

# Configurar defines do sandbox
$SandboxDefines = @(
    "TELEIOS_DEBUG"  # Flag de debug
)

# Configurar flags de linkagem do sandbox
$SandboxLinkFlags = @(
    "-luser32",      # Windows User API
    "-lgdi32",       # Windows GDI
    "-lkernel32"     # Windows Kernel
)

# Verificar se o engine existe
$EngineExe = Join-Path $EngineDir "engine.exe"
if (-not (Test-Path $EngineExe)) {
    Write-Host "AVISO: Engine não encontrado em $EngineExe" -ForegroundColor Yellow
    Write-Host "Você pode precisar compilar o engine primeiro." -ForegroundColor Yellow
    Write-Host ""
}

Write-Host "Flags de compilação:" -ForegroundColor Cyan
$SandboxCompileFlags | ForEach-Object { Write-Host "  $_" -ForegroundColor Gray }
Write-Host ""

Write-Host "Defines:" -ForegroundColor Cyan
$SandboxDefines | ForEach-Object { Write-Host "  -D$_" -ForegroundColor Gray }
Write-Host ""

Write-Host "Flags de linkagem:" -ForegroundColor Cyan
$SandboxLinkFlags | ForEach-Object { Write-Host "  $_" -ForegroundColor Gray }
Write-Host ""

# Executar o script de compilação principal
try {
    & $CompileScript `
        -ProjectPath $SandboxDir `
        -CompileFlags $SandboxCompileFlags `
        -LinkFlags $SandboxLinkFlags `
        -Defines $SandboxDefines

    if ($LASTEXITCODE -eq 0) {
        Write-Host ""
        Write-Host "========================================" -ForegroundColor Cyan
        Write-Host "Sandbox compilado com sucesso!" -ForegroundColor Cyan
        Write-Host "========================================" -ForegroundColor Cyan
    } else {
        Write-Error "Falha ao compilar o sandbox"
        exit 1
    }
} catch {
    Write-Error "Erro ao executar script de compilação: $_"
    exit 1
}
