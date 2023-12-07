@echo off

set "path=C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Tools\MSVC\14.34.31933\bin\HostX64\x64;C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\IDE\VC\VCPackages;C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\IDE\CommonExtensions\Microsoft\TestWindow;C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\IDE\CommonExtensions\Microsoft\TeamFoundation\Team Explorer;C:\Program Files\Microsoft Visual Studio\2022\Community\MSBuild\Current\bin\Roslyn;C:\Program Files\Microsoft Visual Studio\2022\Community\Team Tools\Performance Tools\x64;C:\Program Files\Microsoft Visual Studio\2022\Community\Team Tools\Performance Tools;C:\Program Files (x86)\Microsoft SDKs\Windows\v10.0A\bin\NETFX 4.8 Tools\x64\;C:\Program Files (x86)\HTML Help Workshop;C:\Program Files (x86)\Windows Kits\10\bin\10.0.19041.0\\x64;C:\Program Files (x86)\Windows Kits\10\bin\\x64;C:\Program Files\Microsoft Visual Studio\2022\Community\\MSBuild\Current\Bin\amd64;C:\Windows\Microsoft.NET\Framework64\v4.0.30319;C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\IDE\;C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\Tools\;%path%"

set "disabled_warns=/wd4530 /wd4710 /wd4711 /wd5045 /wd4626 /wd4625 /wd5026 /wd5027"

::NOTE: #pragma warning has priority over switches
::C4514 The optimizer removed an inline function that is not called. (DISABLE CANDIDATE)
::C4530 C++ exception handler used, but unwind semantics are not enabled. Specify /EHsc
::C4710 function not inlined (code gen.)
::C4711 function selected for automatic inline expansion (code gen.)
::C5045 compiler will insert Spectre mitigation for memory load if /Qspectre switch specified (code gen.)
::C4626 assignment operator was implicitly defined as deleted
::C4625 copy constructor was implicitly defined as deleted
::C5026 move constructor was implicitly defined as deleted
::C5027 move assignment operator was implicitly defined as deleted

set "name=%1"
set "res_name=%name%"
set "ico_name=%name%"
set "man_name=%name%"
set "rc_file_path="
set "subsys=WINDOWS"
set "seh=/EHsc"
set "optim=/O2 /Oi /GL /Gy"
set "linkopt=/LTCG"
set "warns=/Wall"
set "slib="
set "dlib="
set "dll_imp_exp="
set "asm="
set "debug=/MT"
set "debug_link="
set "debug_def="
set "no_def_lib=/NODEFAULTLIB:libcmtd.lib"
set "no_launch="
set "start_up=/ENTRY:wmainCRTStartup"
set "res_file="
set "inc_dir=/I D:\P\MT /I res\rc"
set "lib_dir=/LIBPATH:D:\P\MT\lib"
cd /d "%2"

errlogkill %2\%name%

title sd_msvc_build

if %name%==s set "subsys=CONSOLE"

set /p fline=< %name%.cpp

:parse_loop
for /f "tokens=1,* delims= " %%a in ("%fline%") do (
	if "%%a"=="CONSOLE" set "subsys=CONSOLE"
	if "%%a"=="DSEH" set "seh=/wd4577"
	if "%%a"=="NOPT" set "optim=/Od" & set "linkopt="
	if "%%a"=="NW" set "warns=/w"
	if "%%a"=="SLIB" set "slib=/c"
	if "%%a"=="DLIB" set "dlib=/DLL" & set "dll_imp_exp=/NOIMPLIB /NOEXP" & set "debug=/LD" & set "no_launch=no" & set "start_up=/ENTRY:_DllMainCRTStartup"
	if "%%a"=="DLLIE" set "dll_imp_exp="
	if "%%a"=="ASM" set "asm=/FAcsu /Facod\"
	if "%%a"=="DBG" set "debug=/MTd /Z7" & set "debug_link=/DEBUG" & set "debug_def=/D DEBUG" & set "no_def_lib=/NODEFAULTLIB:libcmt.lib"
	if "%%a"=="NLAUNCH" set "no_launch=no"
	if "%%a"=="EPMAIN" set "start_up=/ENTRY:mainCRTStartup"
	
	set "fline=%%b"
	goto :parse_loop
)

if "%slib%"=="/c" goto :nores
if not exist res goto :nores
if exist res\%name%.res goto :setres
if exist res\rc\%name%.rc set "rc_file_path=res\rc\%name%.rc" & goto :buildrc

if not exist res\ico ( if not exist res\man goto :nores )
if not exist res\ico\%name%.ico ( if not exist res\man\%man_name%.manifest set "res_name=main" )
if "%res_name%"=="main" ( if exist res\main.res goto :setres )

if exist res\ico call :addicon
if exist res\man call :addman

set "rc_file_path=res\%res_name%.rc"
goto :buildrc

:addicon
if not exist res\ico\%name%.ico set "ico_name=main"
echo MAINICON ICON "res\\ico\\%ico_name%.ico">> res\%res_name%.rc
exit /b
:addman
if not exist res\man\%name%.manifest set "man_name=main"
if "%dlib%"=="/DLL" ( set "man_id=2" ) else ( set "man_id=1" )
echo %man_id% 24 "res\\man\\%man_name%.manifest">> res\%res_name%.rc
exit /b
:: Process manifests, with ID CREATEPROCESS_MANIFEST_RESOURCE_ID (1), used during process creation
:: Isolation-Aware manifests, with ID ISOLATIONAWARE_MANIFEST_RESOURCE_ID (2), used during DLL loading

:buildrc
rc /r /nologo /fo res\%res_name%.res %rc_file_path%
:setres
set "res_file=res\%res_name%.res"
:nores

if "%debug%"=="/MTd /Z7" (
	if "%optim%" neq "/Od" set "optim=/O2 /Oi /Gy" & set "linkopt="
	if "%dlib%"=="/DLL" set "debug=/LDd /Z7"
)

if not exist exe mkdir exe

:recompile
cl %warns% %disabled_warns% /Feexe\ /external:anglebrackets /external:W0 /diagnostics:caret /I %2 %optim% %debug_def% /D _%subsys% /D _UNICODE /D UNICODE /D _CRT_SECURE_NO_WARNINGS /Gm- %seh% %debug% /FC /GS- /J /permissive- /nologo %slib% %asm% %inc_dir% %name%.cpp %res_file% /link %dlib% /SUBSYSTEM:%subsys% %linkopt% %debug_link% /INCREMENTAL:NO %lib_dir% %dll_imp_exp% /DYNAMICBASE:NO /MACHINE:X64 %start_up% /NODEFAULTLIB:msvcrt.lib /NODEFAULTLIB:msvcrtd.lib %no_def_lib% | vcstyle

if %ERRORLEVEL% equ 1 eline2npp & exit
if %ERRORLEVEL% equ 2 eline2npp
if %ERRORLEVEL% equ 1 goto :cleanup

::static lib assemble begin
if "%slib%"=="/c" ( if "%debug_link%"=="/DEBUG" (
lib /NOLOGO /VERBOSE /OUT:lib\%name%d.lib %name%.obj ) else (
lib /NOLOGO /VERBOSE /OUT:lib\%name%.lib %name%.obj
set "debug=/MTd /Z7" & set "debug_link=/DEBUG" & set "debug_def=/D DEBUG" & set "no_def_lib=/NODEFAULTLIB:libcmt.lib"
if "%optim%" neq "/Od" set "optim=/O2 /Oi /Gy" & set "linkopt="
cls
goto :recompile )

if %ERRORLEVEL% neq 0 ( eline2npp & goto :cleanup ) else ( goto :cleanup ) )
::static lib assemble end

if "%asm%"=="/FAcsu /Facod\" notepad++ cod\%name%.cod & goto :cleanup
if "%debug_link%"=="/DEBUG" ( if "%no_launch%"=="" start sddbg exe\%name%.exe & goto :cleanup )

if "%no_launch%"=="" start "%name%" exe\%name%.exe
:cleanup
del %name%.obj >nul 2>&1
del res\%res_name%.rc >nul 2>&1

REM pause >nul
REM for /f "tokens=2 delims=," %%a in ('tasklist /fi "imagename eq cmd.exe" /v /fo:csv /nh ^| findstr /r ".*sd_msvc_build[^,]*$"') do taskkill /pid %%a >nul 2>&1
REM TODO IMPLEMENT GENERALISED BUILD VERSION INCREMENTOR, IF EVER TRULY NEEDED