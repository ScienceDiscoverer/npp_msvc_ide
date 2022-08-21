# Notepad++ MSVC IDE
Turn your Notepad++ into powerful Visual Studio replacement!

Compile any `.cpp` file instantly with the press of a button:

<img src="readme/onebutton.gif">

Get pretty error/warning/note output:

<img src="readme/vcstyle.gif">

Featuring custom debugger:

<img src="readme/sddbg.gif">

How to setup:
1. Install `Customize Toolbar` plugin
2. Use `shortcuts.xml` and `.btn` file in `plugins/` to set up custom buttons
3. Install [Build Tools for Visual Studio 2022](https://visualstudio.microsoft.com/downloads/) or if you have Visual Studio installed, skip
4. Set up environment variables, launch `C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\Tools\VsDevCmd.bat`
5. `echo %include%`, `echo %lib%`, `echo %libpath%`
6. Copy to `set_vcc_symblink_path.cmd` to set them permanently
7. Alternative to steps 4-6 - use `C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat` to set up variables every time you build a file, if you don't want to mess with global environment (_the cost - slower builds_).
8. Fix up `msvc_build.cmd` and/or setup your folder structure so that it can find all needed files/programs
9. You are ready for rapid C/C++ development in Notepad++!
