@ECHO OFF

call %~dp0\default.bat
PUSHD %~dp0\..
CALL BuildSetup.bat %*
POPD
