:: Generates the Visual Studio 2010 project from the QT .pro project file
:: !! Must set the QTDIR environment variable (e.g. QTDIR=D:\Qt\5.2.1\msvc2010_opengl) !!

@echo off

set PATH=%PATH%;%QTDIR%\bin
cd src

@echo on
qmake TFG.pro -spec win32-msvc2010 -tp vc
@echo off


set USER_FILE=TFG.vcxproj.user
if not exist "%USER_FILE%" (
  echo ^<?xml version="1.0" encoding="utf-8"?^>^<Project ToolsVersion="10.0" xmlns="http://schemas.microsoft.com/developer/msbuild/2003"^>^<PropertyGroup^>^<LocalDebuggerWorkingDirectory^>$^(ProjectDir^)../bin/^</LocalDebuggerWorkingDirectory^>^<DebuggerFlavor^>WindowsLocalDebugger^</DebuggerFlavor^>^</PropertyGroup^>^</Project^>  > %USER_FILE%
)

cd ..

echo ---------
echo Finished!
pause
