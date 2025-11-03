# Sistema de Build TELEIOS

Este diretório contém o sistema de build modular para o projeto TELEIOS.

## Estrutura

- **compile.ps1** - Script principal de compilação (modular e reutilizável)
- **build_all.ps1** (na raiz) - Script para compilar todos os projetos
- **engine/build.ps1** - Script de build específico do engine
- **sandbox/build.ps1** - Script de build específico do sandbox

## Uso

### Compilar tudo

```powershell
# Compilar engine e sandbox
.\build_all.ps1

# Limpar e recompilar tudo
.\build_all.ps1 -Clean

# Compilar apenas o engine
.\build_all.ps1 -EngineOnly

# Compilar apenas o sandbox
.\build_all.ps1 -SandboxOnly
```

### Compilar projetos individuais

```powershell
# Engine
.\engine\build.ps1
.\engine\build.ps1 -Clean

# Sandbox
.\sandbox\build.ps1
.\sandbox\build.ps1 -Clean
```

### Script Principal (compile.ps1)

O script principal aceita os seguintes parâmetros:

```powershell
.\build\compile.ps1 `
    -ProjectPath "C:\caminho\para\projeto" `
    -CompileFlags @("-O2", "-g") `
    -LinkFlags @("-luser32", "-lgdi32") `
    -Defines @("DEBUG", "WINDOWS")
```

**Parâmetros:**

- **ProjectPath** (obrigatório): Caminho para o diretório do projeto
- **CompileFlags** (opcional): Array de flags de compilação
- **LinkFlags** (opcional): Array de flags de linkagem
- **Defines** (opcional): Array de defines/macros (sem o prefixo -D)

## Configuração dos Projetos

### Engine

O script `engine\build.ps1` configura:

**Flags de Compilação:**
- `-O0` - Sem otimização (debug)
- `-g` - Informações de debug
- `-gcodeview` - Debug info para Windows

**Defines:**
- `TELEIOS_EXPORT` - Flag de export para DLL
- `TELEIOS_DEBUG` - Modo debug

**Flags de Linkagem:**
- `-luser32` - Windows User API
- `-lgdi32` - Windows GDI
- `-lkernel32` - Windows Kernel
- `-lopengl32` - OpenGL

### Sandbox

O script `sandbox\build.ps1` configura:

**Flags de Compilação:**
- `-O0` - Sem otimização (debug)
- `-g` - Informações de debug
- `-gcodeview` - Debug info para Windows
- `-I<engine>/src/main` - Incluir headers do engine

**Defines:**
- `TELEIOS_DEBUG` - Modo debug

**Flags de Linkagem:**
- `-luser32` - Windows User API
- `-lgdi32` - Windows GDI
- `-lkernel32` - Windows Kernel

## Compilação Incremental e Paralela

O sistema suporta compilação incremental e paralela:

- **Compilação incremental**: Apenas arquivos modificados são recompilados
- **Compilação paralela**: Utiliza todos os cores físicos da CPU automaticamente
- **Rastreamento de dependências**: Dependências de headers (.h) são rastreadas automaticamente
- **Análise de dependências entre arquivos .c**: O sistema detecta includes de arquivos .c e garante ordem de compilação correta
- **Linkagem inteligente**: Ocorre apenas quando necessário

### Desempenho

O sistema de compilação paralela usa PowerShell RunspacePool para máxima eficiência:
- Detecta automaticamente o número de cores físicos da máquina
- Compila múltiplos arquivos simultaneamente
- Reduz significativamente o tempo de build em projetos grandes

## Estrutura de Diretórios de Build

```
projeto/
├── build/
│   ├── obj/           # Arquivos objeto (.o)
│   ├── deps/          # Arquivos de dependência (.d)
│   └── .build_timestamp
└── projeto.exe        # Executável final
```

## Personalização

Para adicionar ou modificar flags de um projeto:

1. Edite o script `build.ps1` do projeto específico
2. Modifique os arrays:
   - `$CompileFlags` - Flags de compilação
   - `$Defines` - Macros/defines
   - `$LinkFlags` - Flags de linkagem

Exemplo:

```powershell
$EngineCompileFlags = @(
    "-O0",
    "-g",
    "-gcodeview",
    "-Wall",          # Adicionar warnings
    "-Werror"         # Tratar warnings como erros
)
```
