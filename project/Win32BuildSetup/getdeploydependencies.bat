@echo off

REM If KODI_MIRROR is not set externally to this script, set it to the default mirror URL
IF "%KODI_MIRROR%" == "" SET KODI_MIRROR=http://mirrors.kodi.tv
echo Downloading from mirror %KODI_MIRROR%


:: Following commands expect this script's parent directory to be the current directory, so make sure that's so
PUSHD %~dp0

if not exist dependencies\vcredist\2015 mkdir dependencies\vcredist\2015

if not exist dependencies\vcredist\2015\vcredist_%TARGET_ARCHITECTURE%.exe (
  echo Downloading vc140 redist...
  ..\BuildDependencies\bin\wget --tries=5 --retry-connrefused --waitretry=2 -nv -O dependencies\vcredist\2015\vcredist_%TARGET_ARCHITECTURE%.exe %KODI_MIRROR%/build-deps/win32/vcredist/2015/vcredist_%TARGET_ARCHITECTURE%.exe
)
:: Restore the previous current directory
POPD
