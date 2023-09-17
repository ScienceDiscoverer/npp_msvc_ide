@echo off
taskkill /im notepad++.exe
sleep 800
copy /y %~dpn1.dll "C:\Program Files\Notepad++\plugins\%~n1"
start notepad++.exe
setfoc Notepad++