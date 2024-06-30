@echo off

rem The bat file checks the first passed from the caller scope string argument
rem If the argument length is greater than 0 the argument is used in path to the coderdbc.exe
rem Examples:

rem ./run-test-gen.bat release 
rem build\release\coderdbc.exe

rem ./run-test-gen.bat debug 
rem build\debug\coderdbc.exe

echo Argument passed: %~1

rem Check if an argument is provided
if "%~1"=="" (
    rem If no argument is provided, set the path directly to "build\coderdbc.exe"
    set "CMD_PATH=build\coderdbc.exe"
) else (
    rem If an argument is provided, use it to construct the path
    set "CMD_PATH=build\%~1\coderdbc.exe"
)

rem Output the combined path for debugging
echo Combined path: %CMD_PATH%

rem Run the program with the constructed path and other options
"%CMD_PATH%" -dbc test\testdb.dbc -out -out test\gencode -drvname testdb -nodeutils -rw

pause
