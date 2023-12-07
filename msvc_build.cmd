@echo off
REM ADD LOOP HERE TO TEST IF 2022 NOT EXIIST TEST 2021 2020 2019 ETC TAKE CURYEAR AND DECREMENT!

set "bat_dir=C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"
if not exist "%bat_dir%" echo Failed to find VCVARS64.BAT at "%bat_dir%". Edit 'msvc_build.cmd' with the correct path to your Visual Studio! & pause & exit

call "%bat_dir%"
set "path=%bin_dir%;%path%"

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

if exist lib\ext\curl (
	if not exist lib\ext\curl\libcurl_a.lib (
		cd lib\ext\curl\src\winbuild
		nmake /f Makefile.vc mode=static RTLIBCFG=static
		move ..\builds\libcurl-vc-x64-release-static-ipv6-sspi-schannel\lib\libcurl_a.lib ..\..
	)
	if not exist lib\ext\curl\libcurl_a_debug.lib (
		cd lib\ext\curl\src\winbuild
		nmake /f Makefile.vc mode=static RTLIBCFG=static DEBUG=yes
		move ..\builds\libcurl-vc-x64-debug-static-ipv6-sspi-schannel\lib\libcurl_a_debug.lib ..\..
	)
	
	if exist ..\builds rmdir /s /q ..\builds
	cd %~dp0
)

set "libs=txt wtxt utf utfcase txtp wtxtp stdp stdr sddb darr time wndint ftools"
set "obj_out_dir=/Folib\"

:build_all_libs_loop
for /f "tokens=1,* delims= " %%a in ("%libs%") do (
	set "libs=%%b"
	
	if not exist lib\%%a.cpp goto :continue
	if exist lib\%%a.lib ( if exist lib\%%ad.lib goto :continue )
	
	set "name=lib\%%a"
	goto :build_func
)

set "lib_build_done=1"
set "obj_out_dir="

for %%f in (*.cpp) do (
	set "name=%%~nf"
	call :build_func
)

exit

:build_func
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
set "start_up=/ENTRY:wmainCRTStartup"
set "res_file="
set "inc_dir=/I %~dp0\lib /I res\rc"
set "lib_dir=/LIBPATH:%~dp0\lib\"

title sd_msvc_build

set /p fline=< %name%.cpp

:parse_loop
for /f "tokens=1,* delims= " %%a in ("%fline%") do (
	if "%%a"=="CONSOLE" set "subsys=CONSOLE"
	if "%%a"=="DSEH" set "seh=/wd4577"
	if "%%a"=="NOPT" set "optim=/Od" & set "linkopt="
	if "%%a"=="NW" set "warns=/w"
	if "%%a"=="SLIB" set "slib=/c"
	if "%%a"=="DLIB" set "dlib=/DLL" & set "dll_imp_exp=/NOIMPLIB /NOEXP" & set "debug=/LD" & set "start_up=/ENTRY:_DllMainCRTStartup"
	if "%%a"=="DLLIE" set "dll_imp_exp="
	if "%%a"=="DBG" set "debug=/MTd /Z7" & set "debug_link=/DEBUG" & set "debug_def=/D DEBUG" & set "no_def_lib=/NODEFAULTLIB:libcmt.lib"
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

:recompile
cl %warns% %disabled_warns% /external:anglebrackets /external:W0 /diagnostics:caret /I %~dp0 %optim% %debug_def% /D _%subsys% /D _UNICODE /D UNICODE /D _CRT_SECURE_NO_WARNINGS /Gm- %seh% %debug% /FC /GS- /J /permissive- /nologo %slib% %asm% %inc_dir% %name%.cpp %res_file% %obj_out_dir% /link %dlib% /SUBSYSTEM:%subsys% %linkopt% %debug_link% /INCREMENTAL:NO %lib_dir% %dll_imp_exp% /DYNAMICBASE:NO /MACHINE:X64 %start_up% /NODEFAULTLIB:msvcrt.lib /NODEFAULTLIB:msvcrtd.lib %no_def_lib%

if %ERRORLEVEL% equ 1 pause >nul & exit /b
if %ERRORLEVEL% equ 2 pause >nul & goto :cleanup

::static lib assemble begin
if "%slib%"=="/c" ( if "%debug_link%"=="/DEBUG" (
lib /NOLOGO /VERBOSE /OUT:%name%d.lib %name%.obj ) else (
lib /NOLOGO /VERBOSE %name%.obj
set "debug=/MTd /Z7" & set "debug_link=/DEBUG" & set "debug_def=/D DEBUG" & set "no_def_lib=/NODEFAULTLIB:libcmt.lib"
if "%optim%" neq "/Od" set "optim=/O2 /Oi /Gy" & set "linkopt="
cls
goto :recompile )

if %ERRORLEVEL% neq 0 ( pause >nul & goto :cleanup ) else ( goto :cleanup ) )
::static lib assemble end

:cleanup
del %name%.obj >nul 2>&1
del res\%res_name%.rc >nul 2>&1

:continue
cls
if "%lib_build_done%"=="1" ( exit /b ) else ( goto :build_all_libs_loop )