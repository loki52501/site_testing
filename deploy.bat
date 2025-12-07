@echo off
echo Building site...
site_generator.exe
if errorlevel 1 (
    echo Build failed
    exit /b 1
)

echo Committing changes...
git add .
git commit -m "Deploy site"
git push

echo Deployed! Site will be live in 1-2 minutes
