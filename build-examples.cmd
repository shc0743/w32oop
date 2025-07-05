@echo off
setlocal enabledelayedexpansion
chcp 936

title 我的构建系统 v2025.7.5
echo 我的构建系统 v2025.7.5 正在构建...

cd /d "%~dp0" || exit /b 1

where cl >nul
if %errorlevel% neq 0 (
    REM Call the Visual Studio Developer Command Prompt
    call "C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\Tools\VsDevCmd.bat"  -startdir=none -arch=x64 -host_arch=x64
)
where cl >nul
if %errorlevel% neq 0 (
    REM Call the Visual Studio Developer Command Prompt
    call "D:\software\VisualStudio\ms\versions\2022\Community\Common7\Tools\VsDevCmd.bat"  -startdir=none -arch=x64 -host_arch=x64
)

set INCLUDE=%~dp0;%INCLUDE%


python ./build-examples.py
if NOT "%errorlevel%" == "0" (
    echo 构建示例程序失败
    pause
    exit /b
)

timeout 5 > NUL