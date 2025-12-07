
@echo off
REM %1= Check if commit message provided , %2= push repostiory

set "COMMIT_MSG=%~1"
set "PUSHN=%~2"
if "%COMMIT_MSG%"=="" set "COMMIT_MSG=Deploy site"


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