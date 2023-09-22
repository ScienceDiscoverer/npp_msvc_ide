@echo off

set "libs=txt wtxt utf utfcase txtp wtxtp stdp stdr sddb darr time wndint ftools txttst wtxttst ptst"

:parse_loop
for /f "tokens=1,* delims= " %%a in ("%libs%") do (
	cmd /c %~dp0msvc_build %%a %~dp0..\
	cls
	set "libs=%%b"
	goto :parse_loop
)

::pause >nul