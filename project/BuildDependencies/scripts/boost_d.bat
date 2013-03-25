@ECHO ON

SET LOC_PATH=%CD%
SET FILES=%LOC_PATH%\boost_d.txt

CALL dlextract.bat boost %FILES%  || EXIT /B 2

cd %TMP_PATH% || EXIT /B 1

xcopy boost-1_46_1-xbmc-win32\include\* "%CUR_PATH%\include\" /E /Q /I /Y || EXIT /B 5
xcopy boost-1_46_1-xbmc-win32\lib\* "%CUR_PATH%\lib\" /E /Q /I /Y || EXIT /B 5

cd %LOC_PATH% || EXIT /B 1