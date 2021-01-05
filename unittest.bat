@ECHO OFF
SETLOCAL EnableDelayedExpansion

REM ArtNet test
pushd .
cd plugins\artnet\src
..\test\artnet_test.exe
IF NOT %ERRORLEVEL%==0 exit /B %ERRORLEVEL%
popd

REM Enttec wing test
pushd .
cd plugins\enttecwing\src
..\test\ewing_test.exe
IF NOT %ERRORLEVEL%==0 exit /B %ERRORLEVEL%
popd

REM Velleman test
pushd .
cd plugins\velleman\src
SET OLDPATH=%PATH%
PATH=%PATH%;C:\k8062d
REM Surprise, surprise, windows doesn't know how to handle mock objects *sigh*
REM ..\test\vellemanout_test.exe
IF NOT %ERRORLEVEL%==0 exit /B %ERRORLEVEL%
SET PATH=%OLDPATH%
popd

REM Engine tests
pushd .
cd engine\test
SET OLDPATH=%PATH%
PATH=%PATH%;..\..\src
FOR /D %%G IN ("*") DO (
    cd "%%G"

    REM Do something
    IF EXIST "test.bat" CALL "test.bat"

    REM Execute test
    IF EXIST "%%G_test.exe" (
        %%G_test.exe
    )

    REM Check test result and act accordingly
    IF !ERRORLEVEL! EQU 0 (
        ECHO %%G test OK
    ) else (
        exit /B %ERRORLEVEL%
    )
    cd ..
)
SET PATH=%OLDPATH%
popd

REM UI tests
pushd .
cd ui\test
SET OLDPATH=%PATH%
PATH=%PATH%;..\..\src;..\..\..\engine\src
FOR /D %%G IN ("*") DO (
    cd "%%G"

    REM Execute test
    IF EXIST "%%G_test.exe" (
        %%G_test.exe
    )

    REM Check test result and act accordingly
    IF !ERRORLEVEL! EQU 0 (
        ECHO %%G test OK
    ) else (
        exit /B %ERRORLEVEL%
    )
    cd ..
)
SET PATH=%OLDPATH%
popd

ECHO All tests passed
