@ECHO OFF
SETLOCAL ENABLEDELAYEDEXPANSION
REM setup all paths
PUSHD %~dp0\..\..\..
SET base_dir=%CD%
POPD
SET builddeps_dir=%base_dir%\project\BuildDependencies
SET bin_dir=%builddeps_dir%\bin
SET msys_dir=%builddeps_dir%\msys64
IF NOT EXIST %msys_dir% (SET msys_dir=%builddeps_dir%\msys32)
SET awk_exe=%msys_dir%\usr\bin\awk.exe
SET sed_exe=%msys_dir%\usr\bin\sed.exe

REM read the version values from version.txt
FOR /f %%i IN ('%awk_exe% "/APP_NAME/ {print $2}" %base_dir%\version.txt') DO SET APP_NAME=%%i
FOR /f %%i IN ('%awk_exe% "/COMPANY_NAME/ {print $2}" %base_dir%\version.txt') DO SET COMPANY_NAME=%%i
FOR /f %%i IN ('%awk_exe% "/WEBSITE/ {print $2}" %base_dir%\version.txt') DO SET WEBSITE=%%i
FOR /f %%i IN ('%awk_exe% "/VERSION_MAJOR/ {print $2}" %base_dir%\version.txt') DO SET MAJOR=%%i
FOR /f %%i IN ('%awk_exe% "/VERSION_MINOR/ {print $2}" %base_dir%\version.txt') DO SET MINOR=%%i
FOR /f %%i IN ('%awk_exe% "/VERSION_TAG/ {print $2}" %base_dir%\version.txt') DO SET TAG=%%i
FOR /f %%i IN ('%awk_exe% "/ADDON_API/ {print $2}" %base_dir%\version.txt') DO SET VERSION_NUMBER=%%i.0

SET APP_VERSION=%MAJOR%.%MINOR%
IF NOT [%TAG%] == [] (
  SET APP_VERSION=%APP_VERSION%-%TAG%
)

rem ----Usage----
rem BuildSetup [clean|noclean] [noprompt] [nobinaryaddons] [sh]
rem clean to force a full rebuild
rem noclean to force a build without clean
rem noprompt to avoid all prompts
rem nobinaryaddons to skip building binary addons
rem sh to use sh shell instead rxvt
CLS
COLOR 1B
TITLE %APP_NAME% for Windows Build Script
rem ----PURPOSE----
rem - Create a working application build with a single click
rem -------------------------------------------------------------
rem Config
rem If you get an error that Visual studio was not found, SET your path for VSNET main executable.
rem -------------------------------------------------------------
rem  CONFIG START
SET buildmode=ask
SET promptlevel=prompt
SET buildbinaryaddons=true
SET exitcode=0
SET useshell=rxvt
SET BRANCH=na
FOR %%b in (%1, %2, %3, %4, %5) DO (
  IF %%b==clean SET buildmode=clean
  IF %%b==noclean SET buildmode=noclean
  IF %%b==noprompt SET promptlevel=noprompt
  IF %%b==nobinaryaddons SET buildbinaryaddons=false
  IF %%b==sh SET useshell=sh
)

SET PreferredToolArchitecture=x64
SET buildconfig=Release
set WORKSPACE=%base_dir%\kodi-build


  :: sets the BRANCH env var
  call getbranch.bat

  rem  CONFIG END
  rem -------------------------------------------------------------
  goto COMPILE_CMAKE_EXE

:COMPILE_CMAKE_EXE
  ECHO Wait while preparing the build.
  ECHO ------------------------------------------------------------
  ECHO Compiling %APP_NAME% branch %BRANCH%...

  IF %buildmode%==clean (
    RMDIR /S /Q %WORKSPACE%
  )
  MKDIR %WORKSPACE%
  PUSHD %WORKSPACE%

  cmake.exe -G "%cmakeGenerator%" %cmakeProps% %base_dir%
  IF %errorlevel%==1 (
    set DIETEXT="%APP_NAME%.EXE failed to build!"
    goto DIE
  )

  cmake.exe --build . --config "%buildconfig%"
  IF %errorlevel%==1 (
    set DIETEXT="%APP_NAME%.EXE failed to build!"
    goto DIE
  )

  set EXE="%WORKSPACE%\%buildconfig%\%APP_NAME%.exe"
  set PDB="%WORKSPACE%\%buildconfig%\%APP_NAME%.pdb"
  set D3D="%WORKSPACE%\D3DCompile*.DLL"

  POPD
  ECHO Done!
  ECHO ------------------------------------------------------------
  IF "%cmakeProps%" NEQ "" GOTO MAKE_APPX
  GOTO MAKE_BUILD_EXE


:MAKE_BUILD_EXE
  ECHO Copying files...
  PUSHD %base_dir%\project\Win32BuildSetup
  IF EXIST BUILD_WIN32\application rmdir BUILD_WIN32\application /S /Q
  rem Add files to exclude.txt that should not be included in the installer
  
  Echo Thumbs.db>>exclude.txt
  Echo Desktop.ini>>exclude.txt
  Echo dsstdfx.bin>>exclude.txt
  Echo exclude.txt>>exclude.txt
  Echo xbmc.log>>exclude.txt
  Echo xbmc.old.log>>exclude.txt
  Echo kodi.log>>exclude.txt
  Echo kodi.old.log>>exclude.txt
  rem Exclude userdata files
  Echo userdata\advancedsettings.xml>>exclude.txt
  Echo userdata\guisettings.xml>>exclude.txt
  Echo userdata\mediasources.xml>>exclude.txt
  Echo userdata\ModeLines_template.xml>>exclude.txt
  Echo userdata\passwords.xml>>exclude.txt
  Echo userdata\profiles.xml>>exclude.txt
  Echo userdata\sources.xml>>exclude.txt
  Echo userdata\upnpserver.xml>>exclude.txt
  rem Exclude userdata folders
  Echo userdata\addon_data\>>exclude.txt
  Echo userdata\cache\>>exclude.txt
  Echo userdata\database\>>exclude.txt
  Echo userdata\playlists\>>exclude.txt
  Echo userdata\thumbnails\>>exclude.txt

  rem Exclude dlls from system to avoid duplicates
  Echo .dll>>exclude_dll.txt
  
  md BUILD_WIN32\application

  xcopy %EXE% BUILD_WIN32\application > NUL
  xcopy %D3D% BUILD_WIN32\application > NUL
  xcopy %base_dir%\userdata BUILD_WIN32\application\userdata /E /Q /I /Y /EXCLUDE:exclude.txt > NUL
  copy %base_dir%\LICENSE.GPL BUILD_WIN32\application > NUL
  copy %base_dir%\copying.txt BUILD_WIN32\application > NUL
  copy %base_dir%\privacy-policy.txt BUILD_WIN32\application > NUL
  copy %base_dir%\known_issues.txt BUILD_WIN32\application > NUL

  xcopy %WORKSPACE%\addons BUILD_WIN32\application\addons /E /Q /I /Y /EXCLUDE:exclude.txt > NUL
  xcopy %WORKSPACE%\*.dll BUILD_WIN32\application /Q /I /Y > NUL
  xcopy %WORKSPACE%\libbluray-*.jar BUILD_WIN32\application /Q /I /Y > NUL
  xcopy %WORKSPACE%\system BUILD_WIN32\application\system /E /Q /I /Y /EXCLUDE:exclude.txt+exclude_dll.txt  > NUL
  xcopy %WORKSPACE%\media BUILD_WIN32\application\media /E /Q /I /Y /EXCLUDE:exclude.txt  > NUL

  REM create AppxManifest.xml
  "%sed_exe%" -e s/@APP_NAME@/%APP_NAME%/g -e s/@COMPANY_NAME@/%COMPANY_NAME%/g -e s/@TARGET_ARCHITECTURE@/%TARGET_ARCHITECTURE%/g -e s/@APP_VERSION@/%APP_VERSION%/g -e s/@VERSION_NUMBER@/%VERSION_NUMBER%/g "AppxManifest.xml.in" > "BUILD_WIN32\application\AppxManifest.xml"

  SET build_path=%CD%
  IF %buildbinaryaddons%==true (
    ECHO ------------------------------------------------------------
    ECHO Building addons...
    cd %base_dir%\tools\buildsteps\windows
    IF %buildmode%==clean (
      call make-addons.bat clean
    )
    call make-addons.bat
    IF %errorlevel%==1 (
      set DIETEXT="failed to build addons"
      cd %build_path%
      goto DIE
    )

    cd %build_path%
    IF EXIST error.log del error.log > NUL
  )

  rem restore color and title, some scripts mess these up
  COLOR 1B
  TITLE %APP_NAME% for Windows Build Script

  IF EXIST exclude.txt del exclude.txt  > NUL
  IF EXIST exclude_dll.txt del exclude_dll.txt  > NUL
  del /s /q /f BUILD_WIN32\application\*.so  > NUL
  del /s /q /f BUILD_WIN32\application\*.h  > NUL
  del /s /q /f BUILD_WIN32\application\*.cpp  > NUL
  del /s /q /f BUILD_WIN32\application\*.exp  > NUL
  del /s /q /f BUILD_WIN32\application\*.lib  > NUL
  POPD
  
  ECHO ------------------------------------------------------------
  ECHO Build Succeeded!
  GOTO NSIS_EXE

:NSIS_EXE
  ECHO ------------------------------------------------------------
  ECHO Generating installer includes...
  PUSHD %base_dir%\project\Win32BuildSetup
  call genNsisIncludes.bat
  ECHO ------------------------------------------------------------
  call getdeploydependencies.bat
  CALL extract_git_rev.bat > NUL
  SET APP_SETUPFILE=%APP_NAME%Setup-%GIT_REV%-%BRANCH%-%TARGET_ARCHITECTURE%.exe
  SET APP_PDBFILE=%APP_NAME%Setup-%GIT_REV%-%BRANCH%-%TARGET_ARCHITECTURE%.pdb
  ECHO Creating installer %APP_SETUPFILE%...
  IF EXIST %APP_SETUPFILE% del %APP_SETUPFILE% > NUL
  rem get path to makensis.exe from registry, first try tab delim
  FOR /F "tokens=2* delims=  " %%A IN ('REG QUERY "HKLM\Software\NSIS" /ve') DO SET NSISExePath=%%B

  IF NOT EXIST "%NSISExePath%" (
    rem try with space delim instead of tab
    FOR /F "tokens=2* delims= " %%A IN ('REG QUERY "HKLM\Software\NSIS" /ve') DO SET NSISExePath=%%B
  )
      
  IF NOT EXIST "%NSISExePath%" (
    rem fails on localized windows (Default) becomes (Par D�faut)
    FOR /F "tokens=3* delims=  " %%A IN ('REG QUERY "HKLM\Software\NSIS" /ve') DO SET NSISExePath=%%B
  )

  IF NOT EXIST "%NSISExePath%" (
    FOR /F "tokens=3* delims= " %%A IN ('REG QUERY "HKLM\Software\NSIS" /ve') DO SET NSISExePath=%%B
  )
  
  rem proper x64 registry checks
  IF NOT EXIST "%NSISExePath%" (
    ECHO using x64 registry entries
    FOR /F "tokens=2* delims=  " %%A IN ('REG QUERY "HKLM\Software\Wow6432Node\NSIS" /ve') DO SET NSISExePath=%%B
  )
  IF NOT EXIST "%NSISExePath%" (
    rem try with space delim instead of tab
    FOR /F "tokens=2* delims= " %%A IN ('REG QUERY "HKLM\Software\Wow6432Node\NSIS" /ve') DO SET NSISExePath=%%B
  )
  IF NOT EXIST "%NSISExePath%" (
    rem on win 7 x64, the previous fails
    FOR /F "tokens=3* delims=  " %%A IN ('REG QUERY "HKLM\Software\Wow6432Node\NSIS" /ve') DO SET NSISExePath=%%B
  )
  IF NOT EXIST "%NSISExePath%" (
    rem try with space delim instead of tab
    FOR /F "tokens=3* delims= " %%A IN ('REG QUERY "HKLM\Software\Wow6432Node\NSIS" /ve') DO SET NSISExePath=%%B
  )

  SET NSISExe=%NSISExePath%\makensis.exe
  "%NSISExe%" /V1 /X"SetCompressor /FINAL lzma" /Dapp_root="%CD%\BUILD_WIN32" /DAPP_NAME="%APP_NAME%" /DTARGET_ARCHITECTURE="%TARGET_ARCHITECTURE%" /DVERSION_NUMBER="%VERSION_NUMBER%" /DCOMPANY_NAME="%COMPANY_NAME%" /DWEBSITE="%WEBSITE%" /Dapp_revision="%GIT_REV%" /Dapp_branch="%BRANCH%" /D%TARGET_ARCHITECTURE% "genNsisInstaller.nsi"
  IF NOT EXIST "%APP_SETUPFILE%" (
    POPD
    set DIETEXT=Failed to create %APP_SETUPFILE%. NSIS installed?
    goto DIE
  )
  copy %PDB% %APP_PDBFILE% > nul
  POPD
  ECHO ------------------------------------------------------------
  ECHO Done!
  ECHO Setup is located at %CD%\%APP_SETUPFILE%
  ECHO ------------------------------------------------------------
  GOTO END
  
:DIE
  ECHO ------------------------------------------------------------
  ECHO !-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-
  ECHO    ERROR ERROR ERROR ERROR ERROR ERROR ERROR ERROR ERROR
  ECHO !-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-
  set DIETEXT=ERROR: %DIETEXT%
  echo %DIETEXT%
  SET exitcode=1
  ECHO ------------------------------------------------------------
  GOTO END

:MAKE_APPX
:: todo copy appx to BUILD_WIN32
:END
  IF %promptlevel% NEQ noprompt (
    ECHO Press any key to exit...
    pause > NUL
  )
  EXIT /B %exitcode%
