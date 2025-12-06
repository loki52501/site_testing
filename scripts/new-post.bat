@echo off
REM Script to create a new blog post on Windows

echo === Create New Blog Post ===
echo.

set /p title="Enter post title: "

REM Generate filename (simple version for Windows)
set filename=%title: =-%
set filename=%filename%
set filepath=content\%filename%.md

REM Check if file exists
if exist "%filepath%" (
    set /p overwrite="File already exists. Overwrite? (y/n): "
    if not "%overwrite%"=="y" (
        echo Cancelled.
        exit /b 0
    )
)

REM Get current date
for /f "tokens=2-4 delims=/ " %%a in ('date /t') do (set mydate=%%c-%%a-%%b)

REM Create markdown file
(
echo # %title%
echo.
echo *Published on %mydate%*
echo.
echo Write your blog post content here...
echo.
echo ## Section 1
echo.
echo Your content...
echo.
echo ## Section 2
echo.
echo More content...
echo.
) > "%filepath%"

echo.
echo Created: %filepath%
echo.
echo Next steps:
echo 1. Edit the file: %filepath%
echo 2. Build the site: make ^&^& site_generator.exe
echo 3. Commit and push to GitHub
echo.
pause
