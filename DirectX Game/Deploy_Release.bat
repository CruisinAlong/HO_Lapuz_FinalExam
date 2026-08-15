@echo off
REM deploy_release.bat [path-to-built-exe] [target-release-folder]
REM If no exe path provided the script will try to auto-find a Release exe.
setlocal

REM Project folder (relative to repo root where this script lives)
set "PROJ=%CD%\DirectX Game"

REM Default destination release folder (repo root \Release)
if "%~2"=="" (
  set "DEST=%CD%\Release"
) else (
  set "DEST=%~2"
)

REM Ensure destination exists
if not exist "%DEST%" (
  mkdir "%DEST%"
)

REM Determine EXE
if "%~1"=="" (
  echo No exe path provided. Attempting auto-detect...
  set "EXE="
  REM try commonly used VS output folders in order
  for %%F in ("%PROJ%\x64\Release\*.exe") do set "EXE=%%~fF"
  if not defined EXE for %%F in ("%PROJ%\Release\*.exe") do set "EXE=%%~fF"
  if not defined EXE for %%F in ("%PROJ%\x86\Release\*.exe") do set "EXE=%%~fF"
  if not defined EXE (
    echo ERROR: Could not find a built exe automatically.
    echo Provide the path to the built exe as the first argument.
    exit /b 1
  )
) else (
  set "EXE=%~1"
  if not exist "%EXE%" (
    echo ERROR: Specified exe "%EXE%" not found.
    exit /b 1
  )
)

echo Copying exe "%EXE%" => "%DEST%\"
copy /Y "%EXE%" "%DEST%\" >nul

echo Copying HLSL shaders...
xcopy /Y /I "%PROJ%\*.hlsl" "%DEST%\" >nul

echo Copying Images (if present)...
if exist "%PROJ%\Images\" (
  xcopy /E /Y /I "%PROJ%\Images\*" "%DEST%\Images\" >nul
)

echo Copying Meshes (if present)...
if exist "%PROJ%\Meshes\" (
  xcopy /E /Y /I "%PROJ%\Meshes\*" "%DEST%\Meshes\" >nul
)

echo Copying ImGui (if present)...
if exist "%PROJ%\ImGui\" (
  xcopy /E /Y /I "%PROJ%\ImGui\*" "%DEST%\ImGui\" >nul
)

echo Copying Libs (optional)...
if exist "%PROJ%\Libs\" (
  xcopy /E /Y /I "%PROJ%\Libs\*" "%DEST%\Libs\" >nul
)

echo Deployment complete. Files are in "%DEST%".
endlocal