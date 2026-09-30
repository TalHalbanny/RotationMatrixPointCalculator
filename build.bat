@echo off
setlocal
cd /d "%~dp0"

set "CXX="
where clang++ >nul 2>nul && set "CXX=clang++"
if not defined CXX where g++ >nul 2>nul && set "CXX=g++"
if not defined CXX if exist "%LOCALAPPDATA%\Microsoft\WinGet\Packages\MartinStorsjo.LLVM-MinGW.UCRT_Microsoft.Winget.Source_8wekyb3d8bbwe\llvm-mingw-20260616-ucrt-x86_64\bin\clang++.exe" (
  set "CXX=%LOCALAPPDATA%\Microsoft\WinGet\Packages\MartinStorsjo.LLVM-MinGW.UCRT_Microsoft.Winget.Source_8wekyb3d8bbwe\llvm-mingw-20260616-ucrt-x86_64\bin\clang++.exe"
)

if not defined CXX (
  echo No C++ compiler found. Install LLVM MinGW or MinGW-w64, then rerun build.bat
  exit /b 1
)

if not exist build mkdir build
echo Building with %CXX%
"%CXX%" -std=c++17 -O2 -municode -static -o build\PointRotation.exe src\main.cpp -lopengl32 -lglu32 -luser32 -lgdi32
if errorlevel 1 exit /b 1
echo Built build\PointRotation.exe
endlocal
