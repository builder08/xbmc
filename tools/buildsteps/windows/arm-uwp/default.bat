@echo OFF

pushd %~dp0\..
call .helpers\default.bat
call vswhere.bat arm store
if ERRORLEVEL 1 (
	echo ERROR! Something went wrong when calling vswhere.bat
	popd
	exit /B 1
)
popd

set TARGET_CMAKE_GENERATOR=Visual Studio %vsver%
set TARGET_CMAKE_GENERATOR_PLATFORM=ARM
set TARGET_CMAKE_OPTIONS=-DCMAKE_SYSTEM_NAME=WindowsStore -DCMAKE_SYSTEM_VERSION=%UCRTVersion%
set TARGET_ARCHITECTURE=arm
set TARGET_PLATFORM=%TARGET_ARCHITECTURE%-uwp
