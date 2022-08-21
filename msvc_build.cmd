@echo off
set "name=%1"
set "ico_name=%name%"
set "subsys=WINDOWS"
set "seh=/EHsc"
set "optim=/O2 /Oi /GL /Gy"
set "linkopt=/LTCG"
set "warns=/Wall"
set "slib="
set "asm="
set "debug=/MD"
set "debug_link="
set "debug_def="
set "no_launch="
set "res_file="
set "inc_dir=/I %2\..\MicroTests\"
set "lib_dir=/LIBPATH:%2\..\MicroTests\"
cd "%2"

if not exist ico goto :noico
if not exist ico\%ico_name%.ico set ico_name=main

echo MAINICON ICON "ico\\%ico_name%.ico"> ico\res.rc
rc /r /nologo ico\res.rc
set res_file=ico\res.res

:noico

nppsave

title sd_msvc_build

if %name%==small set subsys=CONSOLE

set /p fline=< %name%.cpp

:parse_loop
for /f "tokens=1,* delims= " %%a in ("%fline%") do (
	if "%%a"=="CONSOLE" set "subsys=CONSOLE"
	if "%%a"=="DSEH" set "seh=/wd4577"
	if "%%a"=="NOPT" set "optim=/Od" & set "linkopt="
	if "%%a"=="NW" set "warns=/w"
	if "%%a"=="SLIB" set "slib=/c" & set "res_file="
	if "%%a"=="ASM" set "asm=/FAcsu"
	if "%%a"=="DBG" set "debug=/MDd /Z7" & set "debug_link=/DEBUG" & set "debug_def=/D DEBUG"
	if "%%a"=="NLAUNCH" set "no_launch=no"
	
	
	set "fline=%%b"
	goto :parse_loop
)

:recompile
cl %warns% /wd4530 /wd4710 /wd4711 /wd5045 /external:anglebrackets /external:W0 /diagnostics:caret /I %2 %optim% %debug_def% /D _%subsys% /D _UNICODE /D UNICODE /D _CRT_SECURE_NO_WARNINGS /Gm- %seh% %debug% /GS- /J /Zc:wchar_t /Zc:forScope /Zc:inline /permissive- /nologo %slib% %asm% %inc_dir% %name%.cpp %res_file% /link /SUBSYSTEM:%subsys% %linkopt% %debug_link% %lib_dir% /DYNAMICBASE:NO /MACHINE:X64 /ENTRY:wmainCRTStartup | vcstyle

::#pragma warning has priority over switches
::C4514 The optimizer removed an inline function that is not called. (DISABLE CANDIDATE)
::C4530 C++ exception handler used, but unwind semantics are not enabled. Specify /EHsc
::C4710 function not inlined (code gen.)
::C4711 function selected for automatic inline expansion (code gen.)
::C5045 Compiler will insert Spectre mitigation for memory load if /Qspectre switch specified (code gen.)

if %ERRORLEVEL% EQU 1 pause > nul & exit
if %ERRORLEVEL% EQU 2 pause > nul

if "%slib%"=="/c" ( if "%debug_link%"=="/DEBUG" (
lib /NOLOGO /VERBOSE /OUT:%name%d.lib %name%.obj ) else (
lib /NOLOGO /VERBOSE %name%.obj
set "debug=/MDd /Z7" & set "debug_link=/DEBUG" & set "debug_def=/D DEBUG"
goto :recompile )
if %ERRORLEVEL% NEQ 0 ( pause > nul & goto :cleanup ) else ( goto :cleanup ) )

::pause > nul
if "%asm%"=="/FAcsu" notepad++ %name%.cod & goto :cleanup
if "%debug_link%"=="/DEBUG" start sddbg %name%.exe & goto :cleanup

if "%no_launch%"=="" start "%name%" %name%.exe
:cleanup
del ico\res.rc ico\res.res %name%.obj
taskkill /f /im cmd.exe /fi "windowtitle eq sd_msvc_build"