@echo off
chcp 65001

where cl >nul
if %errorlevel% neq 0 (
    REM Call the Visual Studio Developer Command Prompt
    call "C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\Tools\VsDevCmd.bat"  -startdir=none -arch=x64 -host_arch=x64
)

cd /d %~dp0
for %%I in ("%cd%") do set "p=%%~nxI"
cl /std:c++20 /EHsc /Zi /Fe:"%p%.exe" /D_UNICODE /DUNICODE  ../../Window.cpp demo.cpp /link /MANIFEST:EMBED
cl /std:c++20 /EHsc /Zi /Fe:"%p%-better.exe" /D_UNICODE /DUNICODE  ../../Window.cpp better-demo.cpp /link /MANIFEST:EMBED
