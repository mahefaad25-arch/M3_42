@echo off
setlocal EnableExtensions EnableDelayedExpansion
title Windows Recovery Repair Toolkit
color 0A
mode con: cols=78 lines=35

:: ================================================================
:: WINDOWS RECOVERY REPAIR TOOLKIT
:: Version 2.0 - robust / safer batch implementation
:: ================================================================

:: ----- Require Administrator privileges -----
net session >nul 2>&1
if not "%errorlevel%"=="0" (
    echo.
    echo ================================================================
    echo   Administrator privileges are required.
    echo   Requesting elevation...
    echo ================================================================
    echo.
    powershell -NoProfile -ExecutionPolicy Bypass -Command "Start-Process -FilePath '%~f0' -Verb RunAs"
    exit /b
)

:: ----- Common paths -----
set "REPORTDIR=%USERPROFILE%\Desktop\WindowsRepairReports"
if not exist "%REPORTDIR%" mkdir "%REPORTDIR%" >nul 2>&1

:MENU
cls
echo ==============================================================================
echo                    WINDOWS RECOVERY REPAIR TOOLKIT
echo ==============================================================================
echo.
echo   [ 1] System Information       [13] WinRE Information
echo   [ 2] SFC Scan                 [14] System Restore
echo   [ 3] SFC Verify               [15] Memory Diagnostic
echo   [ 4] DISM ScanHealth          [16] Advanced Startup
echo   [ 5] DISM RestoreHealth       [17] Windows Update
echo   [ 6] Component Cleanup        [18] Full System Report
echo   [ 7] Drive Health             [19] Software Updates
echo   [ 8] Flush DNS                [20] Temporary Files Cleanup
echo   [ 9] Reset Winsock             [21] Service Configuration
echo   [10] Reset TCP/IP              [22] Reset Microsoft Store
echo   [11] Battery Report            [23] Advanced Drive Health
echo   [12] Performance Report
echo.
echo ==============================================================================
echo   [Q] Exit
echo ==============================================================================
echo.
set "choice="
set /p "choice=Select [1-23/Q]: "

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
if /i "%choice%"=="19" goto LOGICIELUPDATE
if /i "%choice%"=="20" goto TEMPCLEAN
if /i "%choice%"=="21" goto SERVICES
if /i "%choice%"=="22" goto RESETSTORE
if /i "%choice%"=="23" goto ADVDRIVEHEALTH
if /i "%choice%"=="Q"  goto EXIT

echo.
echo [X] Invalid selection.
timeout /t 2 >nul
goto MENU


:SYSTEMINFO
cls
call :HEADER "SYSTEM INFORMATION"
systeminfo
call :DONE
goto MENU


:SFCSCAN
cls
call :HEADER "SFC - SYSTEM FILE CHECK"
echo This may take some time.
echo.
sfc /scannow
echo.
call :SHOWERROR "SFC scan"
call :DONE
goto MENU


:SFCVERIFY
cls
call :HEADER "SFC - VERIFY ONLY"
sfc /verifyonly
echo.
call :SHOWERROR "SFC verification"
call :DONE
goto MENU


:DISMSCAN
cls
call :HEADER "DISM - SCAN HEALTH"
DISM /Online /Cleanup-Image /ScanHealth
echo.
call :SHOWERROR "DISM ScanHealth"
call :DONE
goto MENU


:DISMREPAIR
cls
call :HEADER "DISM - RESTORE HEALTH"
echo Internet access may be required if Windows needs repair files.
echo.
DISM /Online /Cleanup-Image /RestoreHealth
echo.
call :SHOWERROR "DISM RestoreHealth"
call :DONE
goto MENU


:CLEANUP
cls
call :HEADER "DISM - COMPONENT CLEANUP"
DISM /Online /Cleanup-Image /StartComponentCleanup
echo.
call :SHOWERROR "Component cleanup"
call :DONE
goto MENU


:DRIVEHEALTH
cls
call :HEADER "DRIVE HEALTH"

echo [PHYSICAL DISKS]
powershell -NoProfile -Command "Get-CimInstance Win32_DiskDrive | Select-Object Model,Status,@{N='SizeGB';E={[math]::Round($_.Size/1GB,1)}} | Format-Table -AutoSize"

echo.
echo [SYSTEM DRIVE SCAN]
chkdsk C: /scan

echo.
call :SHOWERROR "Drive health check"
call :DONE
goto MENU


:FLUSHDNS
cls
call :HEADER "FLUSH DNS"
echo.
ipconfig /flushdns
echo.
call :SHOWERROR "DNS cache flush"
call :DONE
goto MENU


:WINSOCK
cls
call :HEADER "RESET WINSOCK"
echo This operation normally requires a restart to take effect.
echo.
netsh winsock reset
echo.
call :SHOWERROR "Winsock reset"
echo Restart Windows after this operation if requested.
call :DONE
goto MENU


:TCPIP
cls
call :HEADER "RESET TCP/IP"
echo WARNING: Network connections may be interrupted.
echo.
choice /c YN /n /m "Continue? [Y/N]: "
if errorlevel 2 goto MENU

echo.
netsh int ip reset
ipconfig /flushdns
ipconfig /release
ipconfig /renew
echo.
call :SHOWERROR "TCP/IP reset"
call :DONE
goto MENU


:BATTERY
cls
call :HEADER "BATTERY REPORT"
set "REPORT=%REPORTDIR%\battery-report.html"

powercfg /batteryreport /output "%REPORT%"
echo.
if exist "%REPORT%" (
    echo [OK] Report created:
    echo %REPORT%
) else (
    echo [X] Battery report could not be created.
)
call :DONE
goto MENU


:PERFORMANCE
cls
call :HEADER "PERFORMANCE REPORT"
echo Windows Performance Monitor will generate a diagnostic report.
echo This can take approximately 60 seconds.
echo.
perfmon /report
call :DONE
goto MENU


:WINRE
cls
call :HEADER "WINDOWS RECOVERY ENVIRONMENT"
reagentc /info
call :DONE
goto MENU


:RESTORE
cls
call :HEADER "SYSTEM RESTORE"
echo System Restore will open in a separate window.
echo.
start "" rstrui.exe
call :DONE
goto MENU


:MEMORY
cls
call :HEADER "WINDOWS MEMORY DIAGNOSTIC"
echo Windows Memory Diagnostic will open.
echo Choose whether to restart now or check on the next restart.
echo.
start "" mdsched.exe
call :DONE
goto MENU


:ADVANCED
cls
call :HEADER "ADVANCED STARTUP"
echo Windows will restart into the Advanced Startup environment.
echo.
choice /c YN /n /m "Restart now? [Y/N]: "
if errorlevel 2 goto MENU
shutdown /r /o /t 0
exit /b


:UPDATE
cls
call :HEADER "WINDOWS UPDATE"
echo Opening Windows Update settings...
start "" ms-settings:windowsupdate
call :DONE
goto MENU


:FULLREPORT
cls
call :HEADER "FULL SYSTEM REPORT"

set "FULLREPORT=%REPORTDIR%\full-system-report.txt"

echo Generating report...
echo Please wait.
echo.

(
    echo ================================================================
    echo WINDOWS RECOVERY REPAIR TOOLKIT - FULL SYSTEM REPORT
    echo Generated: %date% %time%
    echo ================================================================
    echo.
    echo [SYSTEM INFORMATION]
    systeminfo
    echo.
    echo [NETWORK CONFIGURATION]
    ipconfig /all
    echo.
    echo [PHYSICAL DISKS]
    powershell -NoProfile -Command "Get-CimInstance Win32_DiskDrive | Select-Object Model,Status,@{N='SizeGB';E={[math]::Round($_.Size/1GB,1)}} | Format-Table -AutoSize"
    echo.
    echo [WINDOWS RECOVERY ENVIRONMENT]
    reagentc /info
    echo.
    echo [SFC VERIFY]
    sfc /verifyonly
) > "%FULLREPORT%" 2>&1

if exist "%FULLREPORT%" (
    echo [OK] Full report created:
    echo %FULLREPORT%
) else (
    echo [X] Report creation failed.
)
call :DONE
goto MENU


:LOGICIELUPDATE
cls
call :HEADER "SOFTWARE UPDATES"
where winget >nul 2>&1
if errorlevel 1 (
    echo [X] winget is not available on this system.
    call :DONE
    goto MENU
)

echo Checking installed software for available updates...
echo.
winget upgrade --all
echo.
call :DONE
goto MENU


:TEMPCLEAN
cls
call :HEADER "TEMPORARY FILE CLEANUP"
echo This removes temporary files only.
echo The Windows Prefetch folder is intentionally NOT deleted.
echo.

choice /c YN /n /m "Continue? [Y/N]: "
if errorlevel 2 goto MENU

echo.
echo Cleaning user temporary files...
del /q /f /s "%TEMP%\*" >nul 2>&1

echo Cleaning Windows temporary files...
del /q /f /s "%SystemRoot%\Temp\*" >nul 2>&1

echo.
echo [OK] Temporary-file cleanup completed.
call :DONE
goto MENU


:SERVICES
cls
call :HEADER "SERVICE CONFIGURATION"
echo This option changes Windows service startup settings.
echo It disables DiagTrack and dmwappushservice.
echo.
echo Current status:
sc query DiagTrack
echo.
sc query dmwappushservice
echo.

choice /c YN /n /m "Disable these services? [Y/N]: "
if errorlevel 2 goto MENU

echo.
sc config DiagTrack start= disabled
sc stop DiagTrack >nul 2>&1
sc config dmwappushservice start= disabled
sc stop dmwappushservice >nul 2>&1

echo.
echo Service configuration completed.
call :DONE
goto MENU


:RESETSTORE
cls
call :HEADER "RESET MICROSOFT STORE"
echo Resetting Microsoft Store cache...
echo.
start "" wsreset.exe
call :DONE
goto MENU


:ADVDRIVEHEALTH
cls
call :HEADER "ADVANCED DRIVE HEALTH"
echo.
powershell -NoProfile -Command "Get-PhysicalDisk | Select-Object DeviceId,FriendlyName,OperationalStatus,HealthStatus,MediaType,Size | Format-Table -AutoSize"
echo.
call :DONE
goto MENU


:CHECKNET
ping -n 1 -w 2000 1.1.1.1 >nul 2>&1
if errorlevel 1 exit /b 1
exit /b 0


:HEADER
echo ==============================================================================
echo   %~1
echo ==============================================================================
echo.
exit /b 0


:SHOWERROR
if errorlevel 1 (
    echo [!] %~1 returned an error code.
) else (
    echo [OK] %~1 completed successfully.
)
exit /b 0


:DONE
echo.
echo ==============================================================================
echo   Operation finished.
echo ==============================================================================
pause
exit /b 0


:EXIT
cls
echo.
echo ==============================================================================
echo                 WINDOWS RECOVERY REPAIR TOOLKIT
echo.
echo                            Exiting...
echo ==============================================================================
echo.
timeout /t 2 >nul
exit /b 0