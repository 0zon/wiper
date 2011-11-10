@echo off
rem # create file for wiping
wipe.exe >readme.txt
rem # wipe
wipe readme.txt
echo %errorlevel%
pause
