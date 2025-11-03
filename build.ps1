# TELEIOS CMake Build Script
# Usage: .\build.ps1 [-Clean] [-Release] [-EngineOnly] [-SandboxOnly] [-Generator <name>]

param(
    [switch]$Clean,
    [switch]$Release,
    [switch]$EngineOnly,
    [switch]$SandboxOnly,
    [string]$Generator = ""
)

$ErrorActionPreference = "Stop"

# Determine build type
$BuildType = if ($Release) { "Release" } else { "Debug" }

# Build directory
$BuildDir = "build"

Write-Host "========================================" -ForegroundColor Cyan
Write-Host "TELEIOS CMake Build System" -ForegroundColor Cyan
Write-Host "Build Type: $BuildType" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan

# Function to detect available compilers
function Find-AvailableGenerator {
    Write-Host "`nDetecting available build tools..." -ForegroundColor Yellow

    # Check for Visual Studio
    $vsWhere = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"
    if (Test-Path $vsWhere) {
        $vsInstall = & $vsWhere -latest -property installationPath 2>$null
        if ($vsInstall) {
            $vsYear = & $vsWhere -latest -property catalog_productLineVersion 2>$null
            Write-Host "  [+] Found Visual Studio $vsYear" -ForegroundColor Green
            return "Visual Studio 17 2022"
        }
    }

    # Check for MinGW (via gcc)
    $gcc = Get-Command gcc -ErrorAction SilentlyContinue
    if ($gcc) {
        $mingwPath = Split-Path (Split-Path $gcc.Source)
        Write-Host "  [+] Found MinGW/GCC at: $mingwPath" -ForegroundColor Green

        # Check for mingw32-make
        $make = Get-Command mingw32-make -ErrorAction SilentlyContinue
        if ($make) {
            Write-Host "  [+] Found mingw32-make" -ForegroundColor Green
            return "MinGW Makefiles"
        }

        # Check for make
        $make = Get-Command make -ErrorAction SilentlyContinue
        if ($make) {
            Write-Host "  [+] Found make" -ForegroundColor Green
            return "Unix Makefiles"
        }
    }

    # Check for Ninja
    $ninja = Get-Command ninja -ErrorAction SilentlyContinue
    if ($ninja) {
        Write-Host "  [+] Found Ninja build system" -ForegroundColor Green
        return "Ninja"
    }

    # Check for NMake (Visual Studio)
    $nmake = Get-Command nmake -ErrorAction SilentlyContinue
    if ($nmake) {
        Write-Host "  [+] Found NMake (Visual Studio)" -ForegroundColor Green
        return "NMake Makefiles"
    }

    Write-Host "  [!] No suitable build tools found!" -ForegroundColor Red
    return $null
}

# Function to show installation instructions
function Show-SetupInstructions {
    Write-Host "`n========================================" -ForegroundColor Red
    Write-Host "BUILD TOOLS NOT FOUND" -ForegroundColor Red
    Write-Host "========================================" -ForegroundColor Red
    Write-Host "`nYou need to install a C compiler and build tools."
    Write-Host "`nRecommended options:"
    Write-Host "`n1. Visual Studio (RECOMMENDED FOR WINDOWS):" -ForegroundColor Cyan
    Write-Host "   Download: https://visualstudio.microsoft.com/downloads/"
    Write-Host "   - Install 'Desktop development with C++' workload"
    Write-Host "   - Includes MSVC compiler and build tools"

    Write-Host "`n2. MinGW-w64 via MSYS2:" -ForegroundColor Cyan
    Write-Host "   a. Download MSYS2: https://www.msys2.org/"
    Write-Host "   b. Install and run MSYS2"
    Write-Host "   c. Run: pacman -S mingw-w64-x86_64-gcc mingw-w64-x86_64-make"
    Write-Host "   d. Add to PATH: C:\msys64\mingw64\bin"

    Write-Host "`n3. Clang (via LLVM):" -ForegroundColor Cyan
    Write-Host "   Download: https://releases.llvm.org/download.html"
    Write-Host "   - Add to PATH during installation"

    Write-Host "`nAlso required:" -ForegroundColor Yellow
    Write-Host "   - CMake 3.20+: https://cmake.org/download/"
    Write-Host "   - Git: https://git-scm.com/downloads"

    Write-Host "`nAfter installation, restart PowerShell and try again."
    Write-Host "========================================`n" -ForegroundColor Red
}

# Detect or use specified generator
if ($Generator -eq "") {
    $Generator = Find-AvailableGenerator
    if ($null -eq $Generator) {
        Show-SetupInstructions
        exit 1
    }
    Write-Host "`nUsing generator: $Generator" -ForegroundColor Green
} else {
    Write-Host "`nUsing specified generator: $Generator" -ForegroundColor Green
}

# Clean build if requested
if ($Clean -and (Test-Path $BuildDir)) {
    Write-Host "`nCleaning build directory..." -ForegroundColor Yellow
    Remove-Item -Recurse -Force $BuildDir
}

# Create build directory
if (-not (Test-Path $BuildDir)) {
    Write-Host "Creating build directory..." -ForegroundColor Green
    New-Item -ItemType Directory -Path $BuildDir | Out-Null
}

# Configure CMake
Write-Host "`nConfiguring CMake..." -ForegroundColor Green
Push-Location $BuildDir
try {
    $cmakeArgs = @("-G", $Generator, "-DCMAKE_BUILD_TYPE=$BuildType", "..")

    Write-Host "Running: cmake $($cmakeArgs -join ' ')" -ForegroundColor Gray
    & cmake $cmakeArgs

    if ($LASTEXITCODE -ne 0) {
        Write-Host "`nCMake configuration failed!" -ForegroundColor Red
        Write-Host "Try running with -Clean flag to start fresh." -ForegroundColor Yellow
        throw "CMake configuration failed"
    }

    # Build
    Write-Host "`nBuilding project..." -ForegroundColor Green

    if ($EngineOnly) {
        Write-Host "Building target: engine" -ForegroundColor Cyan
        cmake --build . --target engine --config $BuildType
    } elseif ($SandboxOnly) {
        Write-Host "Building target: sandbox" -ForegroundColor Cyan
        cmake --build . --target sandbox --config $BuildType
    } else {
        Write-Host "Building all targets" -ForegroundColor Cyan
        cmake --build . --config $BuildType
    }

    if ($LASTEXITCODE -ne 0) {
        throw "Build failed"
    }

    Write-Host "`n========================================" -ForegroundColor Green
    Write-Host "BUILD COMPLETED SUCCESSFULLY!" -ForegroundColor Green
    Write-Host "========================================" -ForegroundColor Green

    # Show output location
    if (Test-Path "$BuildDir\bin") {
        Write-Host "`nExecutables:" -ForegroundColor Cyan
        Get-ChildItem "$BuildDir\bin" -File | ForEach-Object {
            Write-Host "  - $($_.Name)" -ForegroundColor White
        }
    }

    Write-Host "`nOutput directory: $BuildDir\bin" -ForegroundColor Cyan

} catch {
    Write-Host "`n========================================" -ForegroundColor Red
    Write-Host "BUILD FAILED" -ForegroundColor Red
    Write-Host "========================================" -ForegroundColor Red
    Write-Host "Error: $_" -ForegroundColor Red
    exit 1
} finally {
    Pop-Location
}
