@echo off
title Windows Recovery Repair Toolkit
color 0A
mode con: cols=65 lines=25

:MENU
cls

echo ================================================================
echo              WINDOWS RECOVERY REPAIR TOOLKIT
echo ================================================================
echo.
echo   [1] System Info             [10] Reset TCP/IP
echo   [2] SFC Scan                [11] Battery Report
echo   [3] SFC Verify              [12] Performance Report
echo   [4] DISM Scan              [13] WinRE Info
echo   [5] DISM Repair             [14] System Restore
echo   [6] Component Cleanup       [15] Memory Diagnostic
echo   [7] Drive Health            [16] Advanced Startup
echo   [8] Flush DNS               [17] Windows Update
echo   [9] Reset Winsock           [18] Full Report
echo   [Q] Exit
echo.
echo ================================================================
echo.
set /p "choice=Select [1-18/Q]: "

if /i "%choice%"=="1"  goto SYSTEMINFO
if /i "%choice%"=="2"  goto SFCSCAN
if /i "%choice%"=="3"  goto SFCVERIFY
if /i "%choice%"=="4"  goto DISMSCAN
if /i "%choice%"=="5"  goto DISMREPAIR
if /i "%choice%"=="6"  goto CLEANUP
if /i "%choice%"=="7"  goto DRIVEHEALTH
if /i "%choice%"=="8"  goto FLUSHDNS
if /i "%choice%"=="9"  goto WINSOCK
if /i "%choice%"=="10" goto TCPIP
if /i "%choice%"=="11" goto BATTERY
if /i "%choice%"=="12" goto PERFORMANCE
if /i "%choice%"=="13" goto WINRE
if /i "%choice%"=="14" goto RESTORE
if /i "%choice%"=="15" goto MEMORY
if /i "%choice%"=="16" goto ADVANCED
if /i "%choice%"=="17" goto UPDATE
if /i "%choice%"=="18" goto FULLREPORT
if /i "%choice%"=="Q"  goto EXIT

echo.
echo Invalid selection.
timeout /t 2 >nul
goto MENU


:SYSTEMINFO
cls
echo === SYSTEM INFORMATION ===
systeminfo
pause
goto MENU


:SFCSCAN
cls
echo === SFC SCAN ===
echo.
sfc /scannow
pause
goto MENU


:SFCVERIFY
cls
echo === SFC VERIFY ===
echo.
sfc /verifyonly
pause
goto MENU


:DISMSCAN
cls
echo === DISM SCAN ===
echo.
DISM /Online /Cleanup-Image /ScanHealth
pause
goto MENU


:DISMREPAIR
cls
echo === DISM REPAIR ===
echo.
DISM /Online /Cleanup-Image /RestoreHealth
pause
goto MENU


:CLEANUP
cls
echo === COMPONENT CLEANUP ===
echo.
DISM /Online /Cleanup-Image /StartComponentCleanup
pause
goto MENU


:DRIVEHEALTH
cls
echo === DRIVE HEALTH ===
echo.
wmic diskdrive get model,status
echo.
echo Checking system drive...
chkdsk C: /scan
pause
goto MENU


:FLUSHDNS
cls
echo === FLUSH DNS ===
echo.
ipconfig /flushdns
pause
goto MENU


:WINSOCK
cls
echo === RESET WINSOCK ===
echo.
netsh winsock reset
echo.
echo Restart Windows after this operation.
pause
goto MENU


:TCPIP
cls
echo === RESET TCP/IP ===
echo.
netsh int ip reset
echo.
ipconfig /release
ipconfig /renew
ipconfig /flushdns
pause
goto MENU


:BATTERY
cls
echo === BATTERY REPORT ===
echo.

set "REPORT=%USERPROFILE%\Desktop\battery-report.html"

powercfg /batteryreport /output "%REPORT%"

echo.
echo Battery report created:
echo %REPORT%
pause
goto MENU


:PERFORMANCE
cls
echo === PERFORMANCE REPORT ===
echo.
echo Generating performance report...
echo.

perfmon /report

pause
goto MENU


:WINRE
cls
echo === WINDOWS RECOVERY ENVIRONMENT ===
echo.

reagentc /info

pause
goto MENU


:RESTORE
cls
echo === SYSTEM RESTORE ===
echo.

rstrui.exe

goto MENU


:MEMORY
cls
echo === MEMORY DIAGNOSTIC ===
echo.
echo Starting Windows Memory Diagnostic...
echo.

mdsched.exe

goto MENU


:ADVANCED
cls
echo === ADVANCED STARTUP ===
echo.
echo Opening Windows Recovery options...
echo.

shutdown /r /o /t 0

goto EXIT


:UPDATE
cls
echo === WINDOWS UPDATE ===
echo.
echo Opening Windows Update...
echo.

start ms-settings:windowsupdate

pause
goto MENU


:FULLREPORT
cls
echo ================================================================
echo                     FULL SYSTEM REPORT
echo ================================================================
echo.

echo [SYSTEM INFORMATION]
echo ================================================================
systeminfo

echo.
echo [NETWORK CONFIGURATION]
echo ================================================================
ipconfig /all

echo.
echo [DISK INFORMATION]
echo ================================================================
wmic diskdrive get model,status,size

echo.
echo [WINDOWS RECOVERY ENVIRONMENT]
echo ================================================================
reagentc /info

echo.
echo [SFC VERIFY]
echo ================================================================
sfc /verifyonly

echo.
echo Report completed.
pause
goto MENU


:EXIT
cls
echo.
echo ================================================================
echo          WINDOWS RECOVERY REPAIR TOOLKIT
echo.
echo                    Exiting...
echo ================================================================
echo.
timeout /t 2 >nul
exit /b