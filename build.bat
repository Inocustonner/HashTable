@echo OFF
setlocal enabledelayedexpansion
set /A do_main_only=0
REM set srcPath=src\
set objPath=obj\
set outputPath=bin\
set CompilerFlags=/c /O2 /Oi /Gd /GF /nologo /W4 /EHsc /wd4101 /wd4201 /wd4477
set CompilerFlagsD=/c /Oi /Gd /GF /nologo /W3 /EHsc /wd4101 /wd4201 /wd4477 /Zi
set LinkerFlagsD=/SUBSYSTEM:CONSOLE /WX /incremental:no /NOLOGO /OUT:bin\bin.exe /DEBUG 
REM set CompilerFlags=/Fo\..\obj

REM Compile each .cpp file to .obj
if not exist %objPath% mkdir %objPath%
if not exist %outputPath% mkdir %outputPath%

rem %cd% %~dp0
if %do_main_only%==1 (
	set tu=%srcPath%main.cpp
	set obj=/Fo%~dp0\%objPath%main
	call :compile
	) else (
	cd %srcPath%
	REM	tus = translation units
	set tus=
	for /f "delims=." %%f in ('dir *.cpp /b /S') do (
		REM set tus=!tus! %srcPath%%%f.cpp
		set tus=!tus! %%f.cpp))
		
	REM		objp = object file path
	set objp=/Fo%~dp0\%objPath%
	set tus=%tus:~1%

	call :compile
	goto :link_all_objs
	exit /B 0

:compile
REM echo|set /p="Compiling: "
echo Compiling:
cl %objp% %tus% %CompilerFlagsD%
exit /B 0

:link_all_objs

set objs=
for /f %%f in ('dir /b /S %~dp0%objPath%') do ( 
	REM %objPath%
	set objs=!objs! %%f
	)
set objs=%objs:~1%

echo Linking: %objs%
:link

link %objs% %LinkerFlagsD%
goto :end

:end
exit /B 0