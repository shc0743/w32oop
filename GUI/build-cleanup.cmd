@echo off
setlocal enabledelayedexpansion

cd /d "%~dp0" || exit /b 1

for /d %%d in ("examples\*") do (
    del /q "%%d\*.exe" "%%d\*.ilk" "%%d\*.obj" "%%d\*.pdb" 2>nul
)