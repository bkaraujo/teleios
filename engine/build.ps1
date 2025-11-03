# ============================================================
# Script de Build do Engine TELEIOS
# ============================================================

param(
    [Parameter(Mandatory=$false)]
    [switch]$Clean
)

$ErrorActionPreference = "Stop"

# Obter diretório do engine
$EngineDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$RootDir = Split-Path -Parent $EngineDir
$CompileScript = Join-Path $RootDir "build\compile.ps1"

Write-Host "========================================" -ForegroundColor Magenta
Write-Host "Build do Engine TELEIOS" -ForegroundColor Magenta
Write-Host "========================================" -ForegroundColor Magenta
Write-Host ""

# Limpar build se solicitado
if ($Clean) {
    $BuildDir = Join-Path $EngineDir "build"
    if (Test-Path $BuildDir) {
        Write-Host "Limpando diretório de build..." -ForegroundColor Yellow
        Remove-Item -Path $BuildDir -Recurse -Force
    }
    $ExeFile = Join-Path $EngineDir "engine.exe"
    if (Test-Path $ExeFile) {
        Remove-Item -Path $ExeFile -Force
    }
    Write-Host "Build limpo!" -ForegroundColor Green
    Write-Host ""
}

# Configurar flags de compilação do engine
$EngineCompileFlags = @(
    "-O0",           # Sem otimização para debug
    "-g",            # Informações de debug
    "-gcodeview"     # Debug info para Windows
)

# Configurar defines do engine
$EngineDefines = @(
    "TELEIOS_BUILD_DEBUG"    # Flag de debug
)

# Configurar flags de linkagem do engine
$EngineLinkFlags = @(
    "-luser32",      # Windows User API
    "-lgdi32",       # Windows GDI
    "-lkernel32",    # Windows Kernel
    "-lopengl32"     # OpenGL
)

Write-Host "Flags de compilação:" -ForegroundColor Cyan
$EngineCompileFlags | ForEach-Object { Write-Host "  $_" -ForegroundColor Gray }
Write-Host ""

Write-Host "Defines:" -ForegroundColor Cyan
$EngineDefines | ForEach-Object { Write-Host "  -D$_" -ForegroundColor Gray }
Write-Host ""

Write-Host "Flags de linkagem:" -ForegroundColor Cyan
$EngineLinkFlags | ForEach-Object { Write-Host "  $_" -ForegroundColor Gray }
Write-Host ""

# Executar o script de compilação principal
try {
    & $CompileScript `
        -ProjectPath $EngineDir `
        -CompileFlags $EngineCompileFlags `
        -LinkFlags $EngineLinkFlags `
        -Defines $EngineDefines

    if ($LASTEXITCODE -eq 0) {
        Write-Host ""
        Write-Host "========================================" -ForegroundColor Magenta
        Write-Host "Engine compilado com sucesso!" -ForegroundColor Magenta
        Write-Host "========================================" -ForegroundColor Magenta
    } else {
        Write-Error "Falha ao compilar o engine"
        exit 1
    }
} catch {
    Write-Error "Erro ao executar script de compilação: $_"
    exit 1
}
