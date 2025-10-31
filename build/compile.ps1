param(
    [Parameter(Mandatory=$true)]
    [string]$ProjectPath
)

$ClangCMD="C:\PROGRA~1\LLVM\bin\clang.exe"

# Configurações
$ErrorActionPreference = "Stop"
$SrcDir = Join-Path $ProjectPath "src\main"
$BuildDir = Join-Path $ProjectPath "build"
$ObjDir = Join-Path $BuildDir "obj"
$DepDir = Join-Path $BuildDir "deps"
$TimestampFile = Join-Path $BuildDir ".build_timestamp"
# ============================================================
# Verificação de Ambiente
# ============================================================
# Verificar se o diretório de origem existe
if (-not (Test-Path $SrcDir)) {
    Write-Error "Diretório de origem não encontrado: $SrcDir"
    exit 1
}

# Criar diretórios de build se não existirem
@($ObjDir, $DepDir) | ForEach-Object {
    if (-not (Test-Path $_)) {
        New-Item -ItemType Directory -Path $_ -Force | Out-Null
    }
}

# Verificar se o Clang está instalível
try { $null = & $ClangCMD --version 2>&1 } 
catch {
    Write-Error "Clang não encontrado. Certifique-se de que está instalado e no PATH."
    exit 1
}
# ============================================================
# Configuração do comando de compilação
# ============================================================
# Flags de linkagem
$LDFLAGS = @()

# Extrair nome do executável do diretório do projeto
$ProjectName = Split-Path $ProjectPath -Leaf
$ExeName = Join-Path $ProjectPath "$ProjectName.exe"

Write-Host "========================================" -ForegroundColor Cyan
Write-Host "Compilação Incremental - Projeto: $ProjectName" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan
Write-Host "Diretório build: $BuildDir" -ForegroundColor Gray
Write-Host "Diretório fonte: $SrcDir" -ForegroundColor Gray
Write-Host ""

# Buscar todos os arquivos .c recursivamente
$SourceFiles = Get-ChildItem -Path $SrcDir -Recurse -Filter "*.c"

if ($SourceFiles.Count -eq 0) {
    Write-Error "Nenhum arquivo .c encontrado em $SrcDir"
    exit 1
}

Write-Host "Encontrados $($SourceFiles.Count) arquivo(s) .c" -ForegroundColor Yellow
Write-Host ""

# Verificar quais arquivos precisam ser recompilados
$FilesToCompile = @()
$ObjectFiles = @()

foreach ($srcFile in $SourceFiles) {
    # Criar caminho relativo preservando a estrutura de diretórios
    $relativePath = $srcFile.FullName.Substring($SrcDir.Length + 1)
    $objName = $relativePath -replace "\.c$", ".o" -replace "\\", "_"
    $objFile = Join-Path $ObjDir $objName
    $depFile = Join-Path $DepDir ($objName -replace "\.o$", ".d")

    $ObjectFiles += $objFile

    $needsCompile = $false

    # Verificar se o arquivo objeto existe
    if (-not (Test-Path $objFile)) {
        $needsCompile = $true
        Write-Host "[NOVO] $relativePath" -ForegroundColor Green
    }
    # Verificar se o arquivo fonte foi modificado
    elseif ((Get-Item $srcFile.FullName).LastWriteTime -gt (Get-Item $objFile).LastWriteTime) {
        $needsCompile = $true
        Write-Host "[MODIFICADO] $relativePath" -ForegroundColor Yellow
    }
    # Verificar dependências (headers)
    elseif (Test-Path $depFile) {
        $depContent = Get-Content $depFile -Raw
        # Extrair lista de dependências do arquivo .d
        $deps = $depContent -split '\s+' | Where-Object { $_ -match '\.(h|hpp)$' }

        foreach ($dep in $deps) {
            # Remover escape de espaços do makefile
            $dep = $dep -replace '\\', ''
            if (Test-Path $dep) {
                if ((Get-Item $dep).LastWriteTime -gt (Get-Item $objFile).LastWriteTime) {
                    $needsCompile = $true
                    Write-Host "[DEPENDÊNCIA] $relativePath (header modificado)" -ForegroundColor Cyan
                    break
                }
            }
        }
    }

    if ($needsCompile) {
        $FilesToCompile += @{
            Source = $srcFile.FullName
            Object = $objFile
            DepFile = $depFile
            RelativePath = $relativePath
        }
    }
}

# Compilar arquivos que precisam ser recompilados
$CompileSuccess = $true
$CompiledCount = 0

if ($FilesToCompile.Count -eq 0) {
    Write-Host "Nenhum arquivo precisa ser recompilado." -ForegroundColor Green
} else {
    Write-Host ""
    Write-Host "Compilando $($FilesToCompile.Count) arquivo(s)..." -ForegroundColor Cyan
    Write-Host ""

    foreach ($file in $FilesToCompile) {
        Write-Host "Compilando: $($file.RelativePath)" -ForegroundColor White

        # Construir comando de compilação
        $compileArgs = @(
            "-std=c11",
            "-Wall",
            "-Wextra",
            # "-Wpedantic",
            "-fno-ms-compatibility",  # Desabilitar compatibilidade MS
            "-Xclang", "-fcxx-exceptions",  # Permitir exceções para complex
            "-D_CRT_USE_BUILTIN_OFFSETOF",
            "-I$SrcDir",
            "-MMD",
            "-MP",
            "-MF", $file.DepFile,
            "-c", $file.Source,
            "-o", $file.Object
        )

        # Executar clang diretamente (não via Invoke-Expression)
        try {
            $ErrorActionPreference = "Continue"
            $output = & $ClangCMD $compileArgs 2>&1
            $ErrorActionPreference = "Stop"

            if ($output) {
                $output | ForEach-Object {
                    Write-Host $_ -ForegroundColor Gray
                }
            }
        } catch {
            # Ignorar exceções do stderr
        }

        if ($LASTEXITCODE -ne 0) {
            Write-Host ""
            Write-Host "ERRO: Compilação falhou para $($file.RelativePath)" -ForegroundColor Red
            $CompileSuccess = $false
            break
        } else {
            $CompiledCount++
        }
    }
}

if (-not $CompileSuccess) {
    Write-Host ""
    Write-Host "Compilação falhou!" -ForegroundColor Red
    exit 1
}

# Verificar se precisa linkar
$needsLink = $false

if ($FilesToCompile.Count -gt 0) {
    $needsLink = $true
    Write-Host ""
    Write-Host "Arquivos compilados com sucesso: $CompiledCount" -ForegroundColor Green
} elseif (-not (Test-Path $ExeName)) {
    $needsLink = $true
    Write-Host "Executável não existe, linkando..." -ForegroundColor Yellow
}

# Linkar
if ($needsLink) {
    Write-Host ""
    Write-Host "Linkando executável: $ProjectName.exe" -ForegroundColor Cyan

    $objFilesStr = ($ObjectFiles | ForEach-Object { "`"$_`"" }) -join " "
    $ldflagsStr = $LDFLAGS -join " "

    $linkCmd = "$ClangCMD $objFilesStr $ldflagsStr -o `"$ExeName`""

    try {
        $output = Invoke-Expression $linkCmd 2>&1
        if ($LASTEXITCODE -ne 0) {
            Write-Host "ERRO ao linkar:" -ForegroundColor Red
            Write-Host $output -ForegroundColor Red
            exit 1
        } else {
            if ($output) {
                Write-Host $output -ForegroundColor Yellow
            }
        }
    } catch {
        Write-Host "ERRO ao linkar: $_" -ForegroundColor Red
        exit 1
    }

    # Atualizar timestamp
    Get-Date | Out-File $TimestampFile

    Write-Host ""
    Write-Host "========================================" -ForegroundColor Green
    Write-Host "Build concluído com sucesso!" -ForegroundColor Green
    Write-Host "========================================" -ForegroundColor Green
    Write-Host "Executável: $ExeName" -ForegroundColor Green
} else {
    Write-Host ""
    Write-Host "========================================" -ForegroundColor Green
    Write-Host "Build está atualizado!" -ForegroundColor Green
    Write-Host "========================================" -ForegroundColor Green
}

Write-Host ""
