
@echo off
REM Check if commit message provided
if "%~1"=="" (
    set "COMMIT_MSG=Deploy site"
) else (
    set "COMMIT_MSG=%~1"
)



echo Building site...
site_generator.exe
if errorlevel 1 (
    echo Build failed
    exit /b 1
)

echo Committing changes...
git add .
git commit -m "%COMMIT_MSG%"
git push "%PUSHN%" main

echo Deployed! Site will be live in 1-2 minutes