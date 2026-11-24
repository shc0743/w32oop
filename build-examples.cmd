@echo off
setlocal enabledelayedexpansion
chcp 936

title 我的构建系统 v2025.7.5
echo 我的构建系统 v2025.7.5 正在构建...

cd /d "%~dp0" || exit /b 1

set INCLUDE=%~dp0;%INCLUDE%


python ./build-examples.py
if NOT "%errorlevel%" == "0" (
    echo 构建示例程序失败
    pause
    exit /b
)

timeout 5 > NUL