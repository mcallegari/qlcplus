@echo off

setlocal enabledelayedexpansion

REM Define the source and destination directories
SET SOURCE_DIR=.
SET DEST_DIR=build
if [%2] neq [] (
  SET DEST_DIR=%2
)

echo Using the destination directory %DEST_DIR%

if not exist %DEST_DIR%\resources\  (
  mkdir %DEST_DIR%\resources\
)

Copy resources directories necessary for unittest
xcopy /y /i /q /s %SOURCE_DIR%\resources\colorfilters %DEST_DIR%\resources\colorfilters\
xcopy /y /i /q /s %SOURCE_DIR%\resources\fixtures %DEST_DIR%\resources\fixtures\
xcopy /y /i /q /s %SOURCE_DIR%\resources\gobos %DEST_DIR%\resources\gobos\
xcopy /y /i /q /s %SOURCE_DIR%\resources\icons %DEST_DIR%\resources\icons\
xcopy /y /i /q /s %SOURCE_DIR%\resources\inputprofiles %DEST_DIR%\resources\inputprofiles\
xcopy /y /i /q /s %SOURCE_DIR%\resources\rgbscripts %DEST_DIR%\resources\rgbscripts\
xcopy /y /i /q /s %SOURCE_DIR%\resources\schemas %DEST_DIR%\resources\schemas\

REM Find all files necessary for tests recursively in the source directory and copy to destination directory
:: Copy all files whose name is test.bat to the destination directory
for /r "%SOURCE_DIR%" %%f in (*.bat) do (
  if exist "%%f" if /I NOT "%%~dpf"=="%DEST_DIR%" (
  	echo "%%~dpf" | find /I "%DEST_DIR%" > nul
  	if not errorlevel 1 (
  		REM ignore the directory including DEST_DIR in the path
  	) else (
      set B=%%f
      set REL_PATH=!B:%CD%\=!
      REM https://stackoverflow.com/questions/4283312/why-does-the-command-xcopy-in-batch-file-ask-for-file-or-folder
      xcopy "%%f" "%DEST_DIR%\!REL_PATH!*" /C /Q /Y
  	)
  )
)

:: Copy all files that match the "*.xml*" pattern to the destination directory
for /r "%SOURCE_DIR%" %%f in (*.xml*) do (
  if exist "%%f" if /I NOT "%%~dpf"=="%DEST_DIR%" (
  	echo "%%~dpf" | find /I "%DEST_DIR%" > nul
  	if not errorlevel 1 (
  		REM ignore the directory including DEST_DIR in the path
  	) else (
      set B=%%f
      set REL_PATH=!B:%CD%\=!
      xcopy "%%f" "%DEST_DIR%\!REL_PATH!*" /C /Q /Y
  	)
  )
)
echo File copy completed successfully!

copy %SOURCE_DIR%\unittest.bat %DEST_DIR%\

cd /d "%DEST_DIR%\"
unittest.bat %1
