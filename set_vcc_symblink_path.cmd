::PATHS TO SOURCE BATS:
::C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\Tools\VsDevCmd.bat
::C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat
::C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat

REM C:\Users\pc\source\repos\shellsymlnk\x64\release\shellsymlnk.exe C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\IDE\CommonExtensions\Microsoft\TeamFoundation\Team Explorer
REM C:\Users\pc\source\repos\shellsymlnk\x64\release\shellsymlnk.exe C:\Program Files\Microsoft Visual Studio\2022\Community\MSBuild\Current\bin\Roslyn
REM C:\Users\pc\source\repos\shellsymlnk\x64\release\shellsymlnk.exe C:\Program Files\Microsoft Visual Studio\2022\Community\Team Tools\Performance Tools\x64
REM C:\Users\pc\source\repos\shellsymlnk\x64\release\shellsymlnk.exe C:\Program Files\Microsoft Visual Studio\2022\Community\Team Tools\Performance Tools
REM C:\Users\pc\source\repos\shellsymlnk\x64\release\shellsymlnk.exe C:\Program Files (x86)\Microsoft SDKs\Windows\v10.0A\bin\NETFX 4.8 Tools\x64
REM C:\Users\pc\source\repos\shellsymlnk\x64\release\shellsymlnk.exe C:\Program Files (x86)\Windows Kits\10\bin\10.0.19041.0\x64
REM C:\Users\pc\source\repos\shellsymlnk\x64\release\shellsymlnk.exe C:\Program Files (x86)\Windows Kits\10\bin\x64
REM C:\Users\pc\source\repos\shellsymlnk\x64\release\shellsymlnk.exe C:\Program Files\Microsoft Visual Studio\2022\Community\MSBuild\Current\Bin\amd64
REM C:\Users\pc\source\repos\shellsymlnk\x64\release\shellsymlnk.exe C:\Windows\Microsoft.NET\Framework64\v4.0.30319
REM C:\Users\pc\source\repos\shellsymlnk\x64\release\shellsymlnk.exe C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\IDE
REM C:\Users\pc\source\repos\shellsymlnk\x64\release\shellsymlnk.exe C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\Tools

REM %include% is for the preprocessor, helps it find #include <file>/"file"
setx INCLUDE "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Tools\MSVC\14.32.31326\ATLMFC\include;C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Tools\MSVC\14.32.31326\include;C:\Program Files (x86)\Windows Kits\NETFXSDK\4.8\include\um;C:\Program Files (x86)\Windows Kits\10\include\10.0.19041.0\ucrt;C:\Program Files (x86)\Windows Kits\10\include\10.0.19041.0\shared;C:\Program Files (x86)\Windows Kits\10\include\10.0.19041.0\um;C:\Program Files (x86)\Windows Kits\10\include\10.0.19041.0\winrt;C:\Program Files (x86)\Windows Kits\10\include\10.0.19041.0\cppwinrt" /m

REM %lib% is for the linker, helps it find import and static libraries.
setx LIB "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Tools\MSVC\14.32.31326\ATLMFC\lib\x64;C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Tools\MSVC\14.32.31326\lib\x64;C:\Program Files (x86)\Windows Kits\NETFXSDK\4.8\lib\um\x64;C:\Program Files (x86)\Windows Kits\10\lib\10.0.19041.0\ucrt\x64;C:\Program Files (x86)\Windows Kits\10\lib\10.0.19041.0\um\x64" /m

REM %libpath% for the compiler, helps it find metadata files -> type libraries, .NET assemblies, WinRT .winmd files.
setx LIBPATH "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Tools\MSVC\14.32.31326\ATLMFC\lib\x64;C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Tools\MSVC\14.32.31326\lib\x64;C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Tools\MSVC\14.32.31326\lib\x86\store\references;C:\Program Files (x86)\Windows Kits\10\UnionMetadata\10.0.19041.0;C:\Program Files (x86)\Windows Kits\10\References\10.0.19041.0;C:\Windows\Microsoft.NET\Framework64\v4.0.30319" /m

pause