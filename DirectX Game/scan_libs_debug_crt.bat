@echo off
REM scan_libs_debug_crt.bat
REM Run this in a Developer Command Prompt (so dumpbin is available)
setlocal
set "LIBROOT=%CD%\DirectX Game\Libs"
echo Scanning libs under "%LIBROOT%" for debug CRT references...
echo.

for /f "delims=" %%F in ('dir /S /B "%LIBROOT%\*.lib" 2^>nul') do (
  echo ---- %%F
  dumpbin /directives "%%F" | findstr /I /R "MSVCRTD LIBCMTD" >nul
  if %ERRORLEVEL% EQU 0 (
    echo   DEBUG-CRT FOUND in %%F
  ) else (
    echo   OK
  )
)
echo.
echo Scan complete.
endlocal
pause