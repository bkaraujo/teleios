param(
    [Parameter(Mandatory=$true)]
    [string]$ProjectPath,

    [Parameter(Mandatory=$false)]
    [string[]]$CompileFlags = @(),

    [Parameter(Mandatory=$false)]
    [string[]]$LinkFlags = @(),

    [Parameter(Mandatory=$false)]
    [string[]]$Defines = @()
)


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
$ClangCMD="C:\PROGRA~1\LLVM\bin\clang.exe"
try { $null = & $ClangCMD --version 2>&1 } 
catch {
    Write-Error "Clang não encontrado. Certifique-se de que está instalado e no PATH."
    exit 1
}
# ============================================================
# Configuração do comando de compilação
# ============================================================
# Flags de linkagem (combinando as padrões com as passadas por parâmetro)
$LDFLAGS = $LinkFlags

# Extrair nome do executável do diretório do projeto
$ProjectName = Split-Path $ProjectPath -Leaf
$ExeName = Join-Path $ProjectPath "$ProjectName.exe"

Write-Host "========================================"       -ForegroundColor Cyan
Write-Host "Compilação Incremental - Projeto: $ProjectName" -ForegroundColor Cyan
Write-Host "========================================"       -ForegroundColor Cyan
Write-Host "Diretório build: $BuildDir"                     -ForegroundColor Gray
Write-Host "Diretório fonte: $SrcDir"                       -ForegroundColor Gray
Write-Host ""

# Buscar todos os arquivos .c recursivamente
$SourceFiles = Get-ChildItem -Path $SrcDir -Recurse -Filter "*.c"

if ($SourceFiles.Count -eq 0) {
    Write-Error "Nenhum arquivo .c encontrado em $SrcDir"
    exit 1
}

Write-Host "Encontrados $($SourceFiles.Count) arquivo(s) .c" -ForegroundColor Yellow

# Detectar número de cores físicos da máquina
$PhysicalCores = (Get-CimInstance -ClassName Win32_Processor | Measure-Object -Property NumberOfCores -Sum).Sum
Write-Host "Cores físicos detectados: $PhysicalCores" -ForegroundColor Gray

# Verificar se ThreadJob está disponível (mais rápido que Start-Job)
$UseThreadJobs = $null -ne (Get-Command Start-ThreadJob -ErrorAction SilentlyContinue)
if ($UseThreadJobs) {
    Write-Host "Usando ThreadJobs para compilação paralela" -ForegroundColor Gray
} else {
    Write-Host "ThreadJobs não disponível, usando Jobs tradicionais" -ForegroundColor Gray
}
Write-Host ""

# Verificar quais arquivos precisam ser recompilados
$FilesToCompile = @()
$ObjectFiles = @()
$FileDependencyMap = @{}  # Mapa de dependências entre arquivos .c

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

    # Construir mapa de dependências entre arquivos .c
    # Extrair includes de outros arquivos .c do mesmo projeto
    $sourceContent = Get-Content $srcFile.FullName -Raw
    $cIncludes = [regex]::Matches($sourceContent, '#include\s+"([^"]+\.c)"')

    $dependencies = @()
    foreach ($match in $cIncludes) {
        $includedFile = $match.Groups[1].Value
        $fullIncludePath = Join-Path (Split-Path $srcFile.FullName) $includedFile

        if (Test-Path $fullIncludePath) {
            $dependencies += $fullIncludePath
        }
    }

    $FileDependencyMap[$srcFile.FullName] = $dependencies
}

# ============================================================
# Compilação Paralela com Runspace Pool
# ============================================================

$CompileSuccess = $true
$CompiledCount = 0

if ($FilesToCompile.Count -eq 0) {
    Write-Host "Nenhum arquivo precisa ser recompilado." -ForegroundColor Green
} else {
    Write-Host ""
    Write-Host "Compilando $($FilesToCompile.Count) arquivo(s) em paralelo (máx: $PhysicalCores threads)..." -ForegroundColor Cyan
    Write-Host ""

    # Criar RunspacePool para paralelismo eficiente
    $RunspacePool = [runspacefactory]::CreateRunspacePool(1, $PhysicalCores)
    $RunspacePool.Open()

    # Script block para compilação
    $ScriptBlock = {
        param($ClangPath, $CompileArgs, $RelativePath)

        try {
            $ErrorActionPreference = "Continue"
            $output = & $ClangPath @CompileArgs 2>&1
            $exitCode = $LASTEXITCODE
            $ErrorActionPreference = "Stop"
        } catch {
            $output = @("Exception: $($_.Exception.Message)")
            $exitCode = 1
        }

        return @{
            RelativePath = $RelativePath
            Output = $output
            ExitCode = $exitCode
        }
    }

    # Criar e iniciar todas as tarefas
    $Tasks = @()
    foreach ($file in $FilesToCompile) {
        Write-Host "Iniciando: $($file.RelativePath)" -ForegroundColor White

        # Construir comando de compilação
        $compileArgs = @(
            "-std=c11",                          # Padrão C11
            "-Wall",                             # Ativar todos os warnings comuns
            "-Wextra",                           # Ativar warnings extras
            # "-Wpedantic",                      # Warnings de conformidade estrita com o padrão
            # "-fno-ms-compatibility",           # Desabilitar compatibilidade MS
            "-Xclang", "-fcxx-exceptions",       # Permitir exceções C++ para _complex
            "-D_CRT_USE_BUILTIN_OFFSETOF",       # Usar offsetof builtin do compilador
            "-I$SrcDir"                          # Diretório de includes
        )

        # Adicionar defines customizados (passados via parâmetro)
        foreach ($define in $Defines) {
            $compileArgs += "-D$define"
        }

        # Adicionar flags de compilação customizadas (passadas via parâmetro)
        $compileArgs += $CompileFlags

        # Adicionar flags de dependência e output
        $compileArgs += @(
            "-MMD",                              # Gerar arquivo de dependências
            "-MP",                               # Adicionar targets phony para cada dependência
            "-MF", $file.DepFile,                # Especificar arquivo de saída de dependências
            "-c", $file.Source,                  # Compilar sem linkar
            "-o", $file.Object                   # Especificar arquivo objeto de saída
        )

        # Criar PowerShell instance e configurar runspace
        $PowerShell = [powershell]::Create()
        $PowerShell.RunspacePool = $RunspacePool
        $PowerShell.AddScript($ScriptBlock).AddArgument($ClangCMD).AddArgument($compileArgs).AddArgument($file.RelativePath) | Out-Null

        # Adicionar à lista de tarefas
        $Tasks += @{
            PowerShell = $PowerShell
            Handle = $PowerShell.BeginInvoke()
            File = $file
        }
    }

    Write-Host ""
    Write-Host "Aguardando conclusão das compilações..." -ForegroundColor Cyan

    # Aguardar conclusão de todas as tarefas
    foreach ($task in $Tasks) {
        try {
            $result = $task.PowerShell.EndInvoke($task.Handle)
            $file = $task.File

            # Processar output
            if ($null -ne $result.Output -and $result.Output.Count -gt 0) {
                $result.Output | ForEach-Object {
                    if ($null -ne $_) {
                        Write-Host "  [$($result.RelativePath)] $_" -ForegroundColor Gray
                    }
                }
            }

            # Verificar resultado
            if ($result.ExitCode -ne 0) {
                Write-Host ""
                Write-Host "ERRO: Compilação falhou para $($result.RelativePath) (Exit Code: $($result.ExitCode))" -ForegroundColor Red
                $CompileSuccess = $false
            } else {
                $CompiledCount++
            }
        } catch {
            Write-Host ""
            Write-Host "ERRO: Exceção ao compilar $($task.File.RelativePath): $_" -ForegroundColor Red
            $CompileSuccess = $false
        } finally {
            $task.PowerShell.Dispose()
        }
    }

    # Fechar RunspacePool
    $RunspacePool.Close()
    $RunspacePool.Dispose()
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
    Write-Host "Build concluído com sucesso!"             -ForegroundColor Green
    Write-Host "========================================" -ForegroundColor Green
    Write-Host "Executável: $ExeName"                     -ForegroundColor Green
} else {
    Write-Host ""
    Write-Host "========================================" -ForegroundColor Green
    Write-Host "Build está atualizado!"                   -ForegroundColor Green
    Write-Host "========================================" -ForegroundColor Green
}

Write-Host ""
