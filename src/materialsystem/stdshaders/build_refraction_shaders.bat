@echo off
setlocal

set GAMEDIR=%cd%\..\..\..\game\refraction\
set SDKBINDIR=C:\Steam\steamapps\common\Source SDK Base 2013 Singleplayer\bin\
set SOURCEDIR=..\..

call "%VS120COMNTOOLS%\vsvars32.bat"

call build_shaders.bat stdshader_dx9_20b    -game %GAMEDIR% -source %SOURCEDIR%
call build_shaders.bat stdshader_dx9_30     -game %GAMEDIR% -source %SOURCEDIR% -dx9_30 -force30 

@pause
